import builtins
import copy
from contextlib import redirect_stderr, redirect_stdout
import io
import json
from pathlib import Path
import re
import struct
import tempfile
from types import SimpleNamespace
import unittest
from unittest import mock
import weakref

from tools import sam_csp_peer as peer


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
VECTOR_PATH = REPOSITORY_ROOT / "tests" / "rs485_peer" / "golden_vectors.json"


def load_vectors():
    with VECTOR_PATH.open("r", encoding="utf-8") as stream:
        return json.load(stream)


def vector_by_name(document, section, name):
    return next(vector for vector in document[section] if vector["name"] == name)


def c_define(path, name):
    source = path.read_text(encoding="utf-8")
    match = re.search(
        rf"^[ \t]*#define\s+{re.escape(name)}\s+([0-9]+)U?(?:\s|$)",
        source,
        re.MULTILINE,
    )
    if match is None:
        raise AssertionError(f"missing integer define {name} in {path}")
    return int(match.group(1))


def c_integer_assignment(path, name):
    source = path.read_text(encoding="utf-8")
    match = re.search(rf"\b{re.escape(name)}\s*=\s*([0-9]+)U?\b", source)
    if match is None:
        raise AssertionError(f"missing integer assignment {name} in {path}")
    return int(match.group(1))


class FakeSerial:
    def __init__(self, received=b"", events=None):
        self.received = bytearray(received)
        self.writes = []
        self.flush_calls = 0
        self.baudrate = peer.DEFAULT_BAUD
        self.events = events

    def __enter__(self):
        return self

    def __exit__(self, exception_type, exception, traceback):
        return False

    def write(self, data):
        copied = bytes(data)
        self.writes.append(copied)
        if self.events is not None:
            self.events.append("write")
        return len(copied)

    def flush(self):
        self.flush_calls += 1
        if self.events is not None:
            self.events.append("flush")

    def read(self, size=1):
        if not self.received:
            return b""
        result = bytes(self.received[:size])
        del self.received[:size]
        return result


class GoldenVectorTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.document = load_vectors()

    def test_vectors_cross_check_committed_c_wire_constants(self):
        constants = self.document["constants"]
        protocol = REPOSITORY_ROOT / "sam_ctl.X" / "iGRVT50" / "header" / "csp" / "sam_csp_protocol.h"
        config = REPOSITORY_ROOT / "sam_ctl.X" / "iGRVT50" / "header" / "csp" / "sam_csp_config.h"
        csp_types = REPOSITORY_ROOT / "third_party" / "libcsp" / "include" / "csp" / "csp_types.h"
        pairs = {
            "protocol_version": (protocol, "SAM_CSP_PROTOCOL_VERSION"),
            "local_address": (config, "SAM_CSP_LOCAL_ADDRESS"),
            "peer_address": (config, "SAM_CSP_PEER_ADDRESS"),
            "command_port": (protocol, "SAM_CSP_COMMAND_PORT"),
            "telemetry_port": (protocol, "SAM_CSP_TELEMETRY_PORT"),
            "diagnostic_port": (protocol, "SAM_CSP_DIAGNOSTIC_PORT"),
            "request_header_length": (protocol, "SAM_CSP_REQUEST_HEADER_LENGTH"),
            "response_header_length": (protocol, "SAM_CSP_RESPONSE_HEADER_LENGTH"),
            "set_outputs_request_length": (protocol, "SAM_CSP_SET_OUTPUTS_REQUEST_LENGTH"),
            "set_mode_request_length": (protocol, "SAM_CSP_SET_MODE_REQUEST_LENGTH"),
            "snapshot_response_length": (protocol, "SAM_CSP_SNAPSHOT_RESPONSE_LENGTH"),
            "health_response_length": (protocol, "SAM_CSP_HEALTH_RESPONSE_LENGTH"),
            "snapshot_pt_count": (protocol, "SAM_CSP_SNAPSHOT_PT_COUNT"),
            "snapshot_tc_count": (protocol, "SAM_CSP_SNAPSHOT_TC_COUNT"),
            "health_counter_count": (protocol, "SAM_CSP_HEALTH_COUNTER_COUNT"),
            "csp_priority_bits": (csp_types, "CSP_ID_PRIO_SIZE"),
            "csp_host_bits": (csp_types, "CSP_ID_HOST_SIZE"),
            "csp_port_bits": (csp_types, "CSP_ID_PORT_SIZE"),
            "csp_flag_bits": (csp_types, "CSP_ID_FLAGS_SIZE"),
        }
        for key, (path, define_name) in pairs.items():
            with self.subTest(key=key):
                self.assertEqual(c_define(path, define_name), constants[key])
        self.assertEqual(constants["priority"], c_integer_assignment(csp_types, "CSP_PRIO_NORM"))
        self.assertEqual(constants["source_port"], c_define(config, "SAM_CSP_MAX_BIND_PORT") + 1)

    def test_validation_status_and_detail_values_cross_check_c_contract(self):
        protocol = REPOSITORY_ROOT / "sam_ctl.X" / "iGRVT50" / "header" / "csp" / "sam_csp_protocol.h"
        codec = REPOSITORY_ROOT / "sam_ctl.X" / "iGRVT50" / "source" / "csp" / "sam_csp_codec.c"
        vectors = {vector["name"]: vector for vector in self.document["validation"]}
        self.assertEqual(
            vectors["bad_version"]["expected_status"],
            c_integer_assignment(protocol, "SAM_CSP_STATUS_BAD_VERSION"),
        )
        self.assertEqual(vectors["bad_version"]["expected_detail"], 0)
        self.assertEqual(
            vectors["bad_length"]["expected_status"],
            c_integer_assignment(protocol, "SAM_CSP_STATUS_BAD_LENGTH"),
        )
        self.assertEqual(
            vectors["bad_length"]["expected_detail"],
            c_define(protocol, "SAM_CSP_SET_OUTPUTS_REQUEST_LENGTH"),
        )
        self.assertEqual(
            vectors["invalid_mask"]["expected_status"],
            c_integer_assignment(protocol, "SAM_CSP_STATUS_INVALID_ARGUMENT"),
        )
        self.assertEqual(
            vectors["invalid_mask"]["expected_detail"],
            c_integer_assignment(codec, "SAM_CSP_SET_OUTPUTS_LPV_OFFSET"),
        )

    def test_crc32c_matches_standard_check_value(self):
        self.assertEqual(peer.crc32c(b"123456789"), 0xE3069283)

    def test_normal_and_validation_vectors_match_every_transport_layer(self):
        for section in ("normal", "validation"):
            for vector in self.document[section]:
                with self.subTest(vector=vector["name"]):
                    application = bytes.fromhex(vector["application_payload_hex"])
                    header = peer.encode_csp_header(
                        vector["priority"],
                        vector["source"],
                        vector["destination"],
                        vector["destination_port"],
                        vector["source_port"],
                        vector["flags"],
                    )
                    self.assertEqual(header.hex(), vector["csp_header_hex"])
                    self.assertEqual(struct.pack(">I", peer.crc32c(application)).hex(), vector["crc32c_hex"])
                    raw = peer.build_raw_frame(header, application)
                    self.assertEqual(raw.hex(), vector["raw_frame_hex"])
                    self.assertEqual(peer.kiss_encode(raw).hex(), vector["kiss_frame_hex"])
                    decoded_header, decoded_application = peer.decode_raw_frame(raw)
                    self.assertEqual(decoded_header.flags, 0)
                    self.assertEqual(decoded_application, application)
                    self.assertEqual(peer.kiss_decode(bytes.fromhex(vector["kiss_frame_hex"])), raw)

    def test_escape_vector_contains_c0_and_db_and_round_trips(self):
        vector = vector_by_name(self.document, "normal", "escape_c0_db_set_outputs_request")
        application = bytes.fromhex(vector["application_payload_hex"])
        kiss_frame = bytes.fromhex(vector["kiss_frame_hex"])
        self.assertIn(0xC0, application)
        self.assertIn(0xDB, application)
        self.assertIn(bytes.fromhex("dbdcdbdd"), kiss_frame)
        self.assertEqual(peer.kiss_decode(kiss_frame).hex(), vector["raw_frame_hex"])

    def test_crc_corruption_is_rejected(self):
        vector = vector_by_name(self.document, "errors", "crc_corruption")
        application = bytes.fromhex(vector["application_payload_hex"])
        header = bytes.fromhex(vector["csp_header_hex"])
        expected_crc = struct.pack(">I", peer.crc32c(application))
        corrupt_crc = bytes.fromhex(vector["crc32c_hex"])
        raw = bytes.fromhex(vector["raw_frame_hex"])
        kiss = bytes.fromhex(vector["kiss_frame_hex"])
        self.assertEqual(vector["expected_error"], "crc32c")
        self.assertEqual(expected_crc.hex(), vector["expected_crc32c_hex"])
        self.assertNotEqual(corrupt_crc, expected_crc)
        self.assertEqual(raw, header + application + corrupt_crc)
        self.assertEqual(kiss, peer.kiss_encode(raw))
        with self.assertRaises(peer.CrcError):
            peer.decode_raw_frame(peer.kiss_decode(kiss))

    def test_selftest_rejects_any_inconsistent_crc_error_metadata(self):
        mutations = {
            "based_on": "get_snapshot_request",
            "application_payload_hex": "00014567",
            "csp_header_hex": "84130c00",
            "expected_crc32c_hex": "00000000",
            "crc32c_hex": "00000000",
            "raw_frame_hex": "84130c00010145672750bb26",
            "kiss_frame_hex": "c00084130c00010145672750bb26c0",
            "expected_error": "not-crc32c",
        }
        for field, changed_value in mutations.items():
            with self.subTest(field=field), tempfile.TemporaryDirectory() as directory:
                document = copy.deepcopy(self.document)
                document["errors"][0][field] = changed_value
                path = Path(directory) / "vectors.json"
                path.write_text(json.dumps(document), encoding="utf-8")
                with self.assertRaises(peer.ProtocolError):
                    peer.run_selftest(path, output=io.StringIO())

    def test_selftest_rejects_coherent_non_four_byte_corrupt_crc_metadata(self):
        document = copy.deepcopy(self.document)
        vector = document["errors"][0]
        vector["crc32c_hex"] = "002750bb26"
        vector["raw_frame_hex"] = "84130d0001014567002750bb26"
        vector["kiss_frame_hex"] = "c00084130d0001014567002750bb26c0"
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "vectors.json"
            path.write_text(json.dumps(document), encoding="utf-8")
            with self.assertRaises(peer.ProtocolError):
                peer.run_selftest(path, output=io.StringIO())


class FrameBoundTests(unittest.TestCase):
    def test_frame_limits_match_committed_rs485_profile(self):
        profile = REPOSITORY_ROOT / "third_party" / "csp-rs485" / "include" / "csp_rs485_profile.h"
        buffer_data_size = c_define(profile, "CSP_RS485_CSP_BUFFER_DATA_SIZE")
        crc_size = c_define(profile, "CSP_RS485_CSP_CRC_SIZE")
        header_size = c_define(profile, "CSP_RS485_CSP_HEADER_SIZE")
        self.assertEqual(peer.MAX_APPLICATION_PAYLOAD, buffer_data_size - crc_size)
        self.assertEqual(peer.MAX_RAW_FRAME_SIZE, header_size + buffer_data_size)
        self.assertEqual(peer.MAX_KISS_FRAME_SIZE, 3 + (2 * peer.MAX_RAW_FRAME_SIZE))

    def test_encode_accepts_exact_maximum_and_rejects_overlength(self):
        header = peer.encode_csp_header(2, 2, 1, 10, 13, 0)
        self.assertEqual(
            len(peer.build_raw_frame(header, bytes(peer.MAX_APPLICATION_PAYLOAD))),
            peer.MAX_RAW_FRAME_SIZE,
        )
        maximum_kiss = peer.kiss_encode(bytes((peer.KISS_FEND,)) * peer.MAX_RAW_FRAME_SIZE)
        self.assertEqual(len(maximum_kiss), peer.MAX_KISS_FRAME_SIZE)
        with self.assertRaises(ValueError):
            peer.build_raw_frame(header, bytes(peer.MAX_APPLICATION_PAYLOAD + 1))
        with self.assertRaises(ValueError):
            peer.kiss_encode(bytes(peer.MAX_RAW_FRAME_SIZE + 1))

    def test_decode_rejects_overlength_raw_and_kiss_frames(self):
        header = peer.encode_csp_header(2, 2, 1, 10, 13, 0)
        overlength_raw = header + bytes(peer.MAX_APPLICATION_PAYLOAD + 1 + 4)
        with self.assertRaisesRegex(peer.ProtocolError, "maximum"):
            peer.decode_raw_frame(overlength_raw)
        overlength_kiss = bytes((peer.KISS_FEND, peer.KISS_DATA_COMMAND)) + bytes(609) + bytes((peer.KISS_FEND,))
        self.assertEqual(len(overlength_kiss), peer.MAX_KISS_FRAME_SIZE + 1)
        with self.assertRaisesRegex(peer.ProtocolError, "maximum"):
            peer.kiss_decode(overlength_kiss)

    def test_receive_rejects_overlength_before_timeout(self):
        overlength_unterminated = bytes((peer.KISS_FEND, peer.KISS_DATA_COMMAND)) + bytes(610)
        with self.assertRaisesRegex(peer.ProtocolError, "maximum"):
            peer.read_kiss_frame(FakeSerial(overlength_unterminated), timeout_seconds=0.01)


class ApplicationCodecTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.document = load_vectors()

    def test_request_encoders_match_c_codec_payloads(self):
        cases = [
            (peer.encode_set_outputs(0x1234, 0x0A55, 0xA5, 0x09, 1), "set_outputs_request"),
            (peer.encode_set_mode(0x2345, 3), "set_mode_request"),
            (peer.encode_get_snapshot(0x3456), "get_snapshot_request"),
            (peer.encode_get_health(0x4567), "get_health_request"),
        ]
        for actual, name in cases:
            with self.subTest(name=name):
                expected = vector_by_name(self.document, "normal", name)["application_payload_hex"]
                self.assertEqual(actual.hex(), expected)

    def test_request_encoders_reject_values_the_c_service_rejects(self):
        invalid = [
            lambda: peer.encode_set_outputs(1, 0x1000, 0, 0, 0),
            lambda: peer.encode_set_outputs(1, 0, 0x100, 0, 0),
            lambda: peer.encode_set_outputs(1, 0, 0, 0x10, 0),
            lambda: peer.encode_set_outputs(1, 0, 0, 0, 2),
            lambda: peer.encode_set_mode(1, 4),
        ]
        for operation in invalid:
            with self.subTest(operation=operation):
                with self.assertRaises(ValueError):
                    operation()

    def test_snapshot_decoder_preserves_signed_big_endian_values(self):
        vector = vector_by_name(self.document, "normal", "snapshot_signed_response")
        snapshot = peer.decode_snapshot_response(bytes.fromhex(vector["application_payload_hex"]), 0xBEEF)
        self.assertEqual(snapshot.sample_time_ms, 0x12345678)
        self.assertEqual(snapshot.current_mode, 2)
        self.assertEqual(snapshot.requested_mode, 3)
        self.assertEqual(snapshot.validity_mask, 0x1FFF)
        self.assertEqual(
            snapshot.pt_millivolt,
            (-1, 0, 1, 0x10203040, -0x01020304, -0x80000000, 0x7FFFFFFF, -123456789, 123456789),
        )
        self.assertEqual(snapshot.tc_microvolt, (-2, 2, -2000000000, 2000000000))

    def test_health_decoder_preserves_counter_order_and_width(self):
        vector = vector_by_name(self.document, "normal", "health_response")
        health = peer.decode_health_response(bytes.fromhex(vector["application_payload_hex"]), 0xCAFE)
        self.assertEqual(health.uptime_ms, 0x11223344)
        self.assertEqual(health.link_state, 2)
        self.assertEqual(health.last_error, 4)
        self.assertEqual(
            health.counters,
            (
                0,
                1,
                0x10203040,
                0xFFFFFFFF,
                0x80000000,
                0x7FFFFFFF,
                0xDEADBEEF,
                0x01020304,
                0xA5A5A5A5,
                0xFFFF,
                0xCAFEBABE,
            ),
        )

    def test_response_decoders_reject_wrong_transaction_and_exact_length(self):
        status = bytes.fromhex("010112340000")
        with self.assertRaises(peer.ProtocolError):
            peer.decode_status_response(status, 1, 0x4321)
        with self.assertRaises(peer.ProtocolError):
            peer.decode_status_response(status + b"\x00", 1, 0x1234)
        with self.assertRaises(peer.ProtocolError):
            peer.decode_snapshot_response(status, 0x1234)


class TransactionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.document = load_vectors()

    def test_transaction_performs_one_write_one_flush_and_checks_response(self):
        request = vector_by_name(self.document, "normal", "set_outputs_request")
        response = vector_by_name(self.document, "normal", "set_outputs_response")
        serial_port = FakeSerial(bytes.fromhex(response["kiss_frame_hex"]))
        result = peer.transact(
            serial_port,
            destination_port=10,
            application_payload=bytes.fromhex(request["application_payload_hex"]),
            timeout_seconds=0.1,
        )
        self.assertEqual(serial_port.writes, [bytes.fromhex(request["kiss_frame_hex"])])
        self.assertEqual(serial_port.flush_calls, 1)
        self.assertEqual(result.application_payload.hex(), response["application_payload_hex"])
        self.assertEqual(result.header.source, 1)
        self.assertEqual(result.header.destination, 2)

    def test_transaction_rejects_wrong_transaction_id(self):
        response = vector_by_name(self.document, "normal", "set_outputs_response")
        serial_port = FakeSerial(bytes.fromhex(response["kiss_frame_hex"]))
        with self.assertRaises(peer.ProtocolError):
            peer.transact(
                serial_port,
                destination_port=10,
                application_payload=peer.encode_set_outputs(0x4321, 0, 0, 0, 0),
                timeout_seconds=0.1,
            )
        self.assertEqual(len(serial_port.writes), 1)
        self.assertEqual(serial_port.flush_calls, 1)

    def test_transaction_timeout_does_not_retry(self):
        serial_port = FakeSerial()
        with self.assertRaises(peer.PeerTimeoutError):
            peer.transact(
                serial_port,
                destination_port=12,
                application_payload=peer.encode_get_health(1),
                timeout_seconds=0.001,
            )
        self.assertEqual(len(serial_port.writes), 1)
        self.assertEqual(serial_port.flush_calls, 1)


class _FakeIds:
    def __init__(self):
        self.value = 0

    def take(self):
        result = self.value
        self.value = (self.value + 1) & 0xFFFF
        return result


class _RetainedRecord(dict):
    __slots__ = ("__weakref__",)


class _FakeClock:
    def __init__(self, events):
        self.now = 0.0
        self.events = events

    def monotonic(self):
        self.events.append("monotonic")
        return self.now

    def sleep(self, seconds):
        self.events.append("sleep")
        self.now += seconds

    def advance(self, seconds):
        self.now += seconds


class CommandBodyTests(unittest.TestCase):
    def test_large_traffic_run_retains_only_first_and_last_records(self):
        live_refs = []
        maximum_live = 0
        sequence = 0

        def synthetic_snapshot(serial_port, transaction_id, timeout_seconds):
            nonlocal maximum_live, sequence
            del serial_port, transaction_id, timeout_seconds
            live_refs[:] = [reference for reference in live_refs if reference() is not None]
            maximum_live = max(maximum_live, len(live_refs))
            if len(live_refs) > 2:
                raise AssertionError("traffic command retained more than first/last records")
            sequence += 1
            record = _RetainedRecord(latency_ms=0.25, sequence=sequence)
            live_refs.append(weakref.ref(record))
            return record, None

        args = SimpleNamespace(
            device="COM_TEST",
            baud=peer.DEFAULT_BAUD,
            timeout_seconds=0.1,
            interval_ms=0.0,
        )
        output = io.StringIO()
        with (
            mock.patch.object(peer, "_open_serial", return_value=FakeSerial()),
            mock.patch.object(peer, "_snapshot_transaction", side_effect=synthetic_snapshot),
            redirect_stdout(output),
        ):
            result = peer._run_traffic(args, _FakeIds(), count=5000)

        summary = json.loads(output.getvalue())
        self.assertEqual(result, 0)
        self.assertLessEqual(maximum_live, 2)
        self.assertEqual(summary["attempted"], 5000)
        self.assertEqual(summary["first_transaction"]["sequence"], 1)
        self.assertEqual(summary["last_transaction"]["sequence"], 5000)

    @staticmethod
    def _health_response(transaction_id, counters):
        return peer.HealthResponse(
            version=1,
            opcode=1,
            transaction_id=transaction_id,
            status=0,
            detail=0,
            uptime_ms=0,
            link_state=1,
            last_error=0,
            counters=tuple(counters),
        )

    def _run_synthetic_recovery(self, settle_seconds, post_reply_seconds):
        events = []
        clock = _FakeClock(events)
        serial_port = FakeSerial(events=events)
        calls = 0

        def synthetic_health(port, transaction_id, timeout_seconds):
            nonlocal calls
            del port, timeout_seconds
            calls += 1
            counters = [0] * 11
            if calls == 2:
                counters[0] = 1
                counters[8] = 1
                counters[9] = 1
                clock.advance(post_reply_seconds)
            response = self._health_response(transaction_id, counters)
            return {"latency_ms": post_reply_seconds * 1000.0}, response

        args = SimpleNamespace(
            device="COM_TEST",
            baud=peer.DEFAULT_BAUD,
            timeout_seconds=1.0,
            iterations=1,
            fault_baud=115200,
            fault_bytes=64,
            settle_seconds=settle_seconds,
        )
        output = io.StringIO()
        with (
            mock.patch.object(peer, "_open_serial", return_value=serial_port),
            mock.patch.object(peer, "_health_transaction", side_effect=synthetic_health),
            mock.patch.object(peer.time, "monotonic", side_effect=clock.monotonic),
            mock.patch.object(peer.time, "sleep", side_effect=clock.sleep),
            redirect_stdout(output),
        ):
            result = peer._run_recovery(args, _FakeIds())
        return result, json.loads(output.getvalue()), events

    def test_recovery_250ms_gate_includes_configured_settle_interval(self):
        result, summary, events = self._run_synthetic_recovery(0.250, 0.001)
        trial = summary["trials"][0]
        self.assertEqual(events[:3], ["write", "flush", "monotonic"])
        self.assertEqual(result, 1)
        self.assertEqual(trial["outcome"], "fail")
        self.assertEqual(trial["post_fault_elapsed_ms"], 251.0)
        self.assertEqual(trial["post_request_latency_ms"], 1.0)

    def test_recovery_passes_when_settle_plus_first_reply_is_within_250ms(self):
        result, summary, _ = self._run_synthetic_recovery(0.200, 0.040)
        trial = summary["trials"][0]
        self.assertEqual(result, 0)
        self.assertEqual(trial["outcome"], "pass")
        self.assertEqual(trial["post_fault_elapsed_ms"], 240.0)
        self.assertEqual(trial["post_request_latency_ms"], 40.0)

    def test_recovery_uses_unrounded_elapsed_for_250ms_boundary(self):
        result, summary, _ = self._run_synthetic_recovery(0.249, 0.0010004)
        trial = summary["trials"][0]
        self.assertEqual(result, 1)
        self.assertEqual(trial["outcome"], "fail")
        self.assertEqual(trial["post_fault_elapsed_ms"], 250.0)
        self.assertAlmostEqual(trial["post_request_latency_ms"], 1.0004)


class CliContractTests(unittest.TestCase):
    def test_all_approved_commands_exist_and_online_commands_require_device(self):
        parser = peer.build_argument_parser()
        subparsers = next(action for action in parser._actions if action.choices)
        self.assertEqual(
            set(subparsers.choices),
            {"selftest", "set-outputs", "set-mode", "snapshot", "health", "smoke", "sequence", "rx-burst", "recovery"},
        )
        self.assertEqual(parser.parse_args(["selftest"]).command, "selftest")
        for command in set(subparsers.choices) - {"selftest"}:
            with self.subTest(command=command):
                with redirect_stderr(io.StringIO()), self.assertRaises(SystemExit):
                    parser.parse_args([command])

    def test_selftest_does_not_import_serial_and_prints_exact_pass_marker(self):
        original_import = builtins.__import__

        def reject_serial(name, *args, **kwargs):
            if name == "serial" or name.startswith("serial."):
                raise AssertionError("offline selftest imported pyserial")
            return original_import(name, *args, **kwargs)

        output = io.StringIO()
        builtins.__import__ = reject_serial
        try:
            peer.run_selftest(VECTOR_PATH, output=output)
        finally:
            builtins.__import__ = original_import
        self.assertEqual(output.getvalue().strip(), "PASS golden vectors")

    def test_only_pyserial_35_is_pinned(self):
        requirements = (REPOSITORY_ROOT / "tools" / "requirements-csp-peer.txt").read_text(encoding="utf-8")
        self.assertEqual(requirements.splitlines(), ["pyserial==3.5"])

    def test_recovery_default_and_task12_command_leave_reply_headroom(self):
        parser = peer.build_argument_parser()
        parsed = parser.parse_args(["recovery", "--device", "COM_TEST"])
        self.assertEqual(parsed.settle_seconds, 0.20)
        self.assertLess(parsed.settle_seconds * 1000.0, peer.RECOVERY_ACCEPTANCE_MS)

        plan = (
            REPOSITORY_ROOT
            / "docs"
            / "superpowers"
            / "plans"
            / "2026-08-06-samv71-libcsp-rs485-porting.md"
        ).read_text(encoding="utf-8")
        configured = re.findall(
            r"sam_csp_peer\.py recovery [^\r\n]*--settle-seconds ([0-9.]+)",
            plan,
        )
        self.assertEqual(configured, ["0.20"])
        self.assertLess(float(configured[0]) * 1000.0, peer.RECOVERY_ACCEPTANCE_MS)


if __name__ == "__main__":
    unittest.main()
