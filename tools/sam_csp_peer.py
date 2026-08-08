#!/usr/bin/env python3
"""Explicit-device CSP v1/KISS peer for the SAMV71 RS485 link."""

from __future__ import annotations

import argparse
from dataclasses import asdict, dataclass
import json
from pathlib import Path
import struct
import sys
import time
from typing import Sequence, TextIO


PROTOCOL_VERSION = 1
LOCAL_ADDRESS = 1
PEER_ADDRESS = 2
CSP_PRIORITY_NORMAL = 2
CSP_SOURCE_PORT = 13
COMMAND_PORT = 10
TELEMETRY_PORT = 11
DIAGNOSTIC_PORT = 12

OPCODE_SET_OUTPUTS = 1
OPCODE_SET_MODE = 2
OPCODE_GET_SNAPSHOT = 1
OPCODE_GET_HEALTH = 1

REQUEST_HEADER_LENGTH = 4
RESPONSE_HEADER_LENGTH = 6
SET_OUTPUTS_REQUEST_LENGTH = 10
SET_MODE_REQUEST_LENGTH = 5
SNAPSHOT_RESPONSE_LENGTH = 66
HEALTH_RESPONSE_LENGTH = 58
SNAPSHOT_PT_COUNT = 9
SNAPSHOT_TC_COUNT = 4
HEALTH_COUNTER_COUNT = 11

KISS_FEND = 0xC0
KISS_FESC = 0xDB
KISS_TFEND = 0xDC
KISS_TFESC = 0xDD
KISS_DATA_COMMAND = 0x00

DEFAULT_BAUD = 921600
DEFAULT_TIMEOUT_SECONDS = 1.0
MAX_APPLICATION_PAYLOAD = 296
MAX_RAW_FRAME_SIZE = 304
MAX_KISS_FRAME_SIZE = 611
RECOVERY_ACCEPTANCE_MS = 250.0

STATUS_NAMES = (
    "OK",
    "BAD_VERSION",
    "BAD_LENGTH",
    "BAD_OPCODE",
    "INVALID_ARGUMENT",
    "INVALID_STATE",
    "APPLY_FAILED",
    "INTERNAL_ERROR",
    "BUSY",
)

HEALTH_COUNTER_NAMES = (
    "uart_errors",
    "dma_errors",
    "tx_timeouts",
    "tx_failures",
    "protocol_errors",
    "stream_dropped_bytes",
    "stream_high_watermark",
    "stream_discontinuities",
    "recovery_attempts",
    "recovery_successes",
    "recovery_failures",
)


class PeerError(Exception):
    """Base class for peer and wire-contract failures."""


class ProtocolError(PeerError):
    """A frame or application response violates the binary contract."""


class CrcError(ProtocolError):
    """The application-payload CRC32C did not match."""


class PeerTimeoutError(PeerError):
    """No complete KISS response arrived before the explicit deadline."""


class RemoteStatusError(PeerError):
    """The SAM service returned a non-OK status response."""

    def __init__(self, status: int, detail: int):
        name = STATUS_NAMES[status] if 0 <= status < len(STATUS_NAMES) else f"UNKNOWN_{status}"
        super().__init__(f"remote status {name} ({status}), detail {detail}")
        self.status = status
        self.detail = detail


@dataclass(frozen=True)
class CspHeader:
    priority: int
    source: int
    destination: int
    destination_port: int
    source_port: int
    flags: int


@dataclass(frozen=True)
class StatusResponse:
    version: int
    opcode: int
    transaction_id: int
    status: int
    detail: int


@dataclass(frozen=True)
class SnapshotResponse:
    version: int
    opcode: int
    transaction_id: int
    status: int
    detail: int
    sample_time_ms: int
    current_mode: int
    requested_mode: int
    validity_mask: int
    pt_millivolt: tuple[int, ...]
    tc_microvolt: tuple[int, ...]


@dataclass(frozen=True)
class HealthResponse:
    version: int
    opcode: int
    transaction_id: int
    status: int
    detail: int
    uptime_ms: int
    link_state: int
    last_error: int
    counters: tuple[int, ...]

    def named_counters(self) -> dict[str, int]:
        return dict(zip(HEALTH_COUNTER_NAMES, self.counters, strict=True))


@dataclass(frozen=True)
class TransactionResult:
    header: CspHeader
    application_payload: bytes
    request_frame: bytes
    response_frame: bytes
    latency_seconds: float


def _require_integer(name: str, value: int, minimum: int, maximum: int) -> int:
    if not isinstance(value, int) or isinstance(value, bool) or not minimum <= value <= maximum:
        raise ValueError(f"{name} must be an integer in {minimum}..{maximum}")
    return value


def encode_csp_header(
    priority: int,
    source: int,
    destination: int,
    destination_port: int,
    source_port: int,
    flags: int = 0,
) -> bytes:
    """Encode the libcsp v1 32-bit identifier bytewise in network order."""
    priority = _require_integer("priority", priority, 0, 3)
    source = _require_integer("source", source, 0, 31)
    destination = _require_integer("destination", destination, 0, 31)
    destination_port = _require_integer("destination_port", destination_port, 0, 63)
    source_port = _require_integer("source_port", source_port, 0, 63)
    flags = _require_integer("flags", flags, 0, 255)
    if flags != 0:
        raise ValueError("SAM CSP Binary v1 requires CSP header flags to be zero")

    identifier = (
        (priority << 30)
        | (source << 25)
        | (destination << 20)
        | (destination_port << 14)
        | (source_port << 8)
        | flags
    )
    return identifier.to_bytes(4, "big")


def decode_csp_header(encoded: bytes) -> CspHeader:
    if len(encoded) != 4:
        raise ProtocolError("CSP v1 header must be exactly four bytes")
    identifier = int.from_bytes(encoded, "big")
    header = CspHeader(
        priority=(identifier >> 30) & 0x03,
        source=(identifier >> 25) & 0x1F,
        destination=(identifier >> 20) & 0x1F,
        destination_port=(identifier >> 14) & 0x3F,
        source_port=(identifier >> 8) & 0x3F,
        flags=identifier & 0xFF,
    )
    if header.flags != 0:
        raise ProtocolError("SAM CSP Binary v1 received nonzero CSP header flags")
    return header


def crc32c(data: bytes) -> int:
    """Return libcsp's reflected Castagnoli CRC with init/final XOR."""
    crc = 0xFFFFFFFF
    for byte in data:
        crc ^= byte
        for _ in range(8):
            crc = (crc >> 1) ^ 0x82F63B78 if crc & 1 else crc >> 1
    return crc ^ 0xFFFFFFFF


def build_raw_frame(csp_header: bytes, application_payload: bytes) -> bytes:
    if len(csp_header) != 4:
        raise ValueError("CSP header must be exactly four bytes")
    if len(application_payload) > MAX_APPLICATION_PAYLOAD:
        raise ValueError(f"application payload exceeds {MAX_APPLICATION_PAYLOAD} bytes")
    checksum = struct.pack(">I", crc32c(application_payload))
    return bytes(csp_header) + bytes(application_payload) + checksum


def decode_raw_frame(raw_frame: bytes) -> tuple[CspHeader, bytes]:
    if len(raw_frame) > MAX_RAW_FRAME_SIZE:
        raise ProtocolError(f"raw CSP frame exceeds maximum {MAX_RAW_FRAME_SIZE} bytes")
    if len(raw_frame) < 8:
        raise ProtocolError("raw CSP frame is shorter than header plus CRC32C")
    header = decode_csp_header(raw_frame[:4])
    application_payload = raw_frame[4:-4]
    received_crc = int.from_bytes(raw_frame[-4:], "big")
    expected_crc = crc32c(application_payload)
    if received_crc != expected_crc:
        raise CrcError(
            f"CRC32C mismatch: received {received_crc:08x}, expected {expected_crc:08x}"
        )
    return header, application_payload


def kiss_encode(raw_frame: bytes) -> bytes:
    if len(raw_frame) > MAX_RAW_FRAME_SIZE:
        raise ValueError(f"raw CSP frame exceeds maximum {MAX_RAW_FRAME_SIZE} bytes")
    encoded = bytearray((KISS_FEND, KISS_DATA_COMMAND))
    for byte in raw_frame:
        if byte == KISS_FEND:
            encoded.extend((KISS_FESC, KISS_TFEND))
        elif byte == KISS_FESC:
            encoded.extend((KISS_FESC, KISS_TFESC))
        else:
            encoded.append(byte)
    encoded.append(KISS_FEND)
    return bytes(encoded)


def kiss_decode(kiss_frame: bytes) -> bytes:
    if len(kiss_frame) > MAX_KISS_FRAME_SIZE:
        raise ProtocolError(f"KISS frame exceeds maximum {MAX_KISS_FRAME_SIZE} bytes")
    if len(kiss_frame) < 3 or kiss_frame[0] != KISS_FEND or kiss_frame[-1] != KISS_FEND:
        raise ProtocolError("KISS frame requires FEND, data command, and trailing FEND")

    decoded = bytearray()
    escaped = False
    for byte in kiss_frame[1:-1]:
        if escaped:
            if byte == KISS_TFEND:
                decoded.append(KISS_FEND)
            elif byte == KISS_TFESC:
                decoded.append(KISS_FESC)
            else:
                raise ProtocolError(f"invalid KISS escape byte 0x{byte:02x}")
            escaped = False
        elif byte == KISS_FESC:
            escaped = True
        elif byte == KISS_FEND:
            raise ProtocolError("unescaped FEND inside KISS frame")
        else:
            decoded.append(byte)
    if escaped:
        raise ProtocolError("truncated KISS escape sequence")
    if not decoded or decoded[0] != KISS_DATA_COMMAND:
        raise ProtocolError("only KISS data command 0 is supported")
    return bytes(decoded[1:])


def encode_set_outputs(
    transaction_id: int,
    lpv_mask: int,
    hpv_mask: int,
    heater_mask: int,
    spark: int,
) -> bytes:
    transaction_id = _require_integer("transaction_id", transaction_id, 0, 0xFFFF)
    lpv_mask = _require_integer("lpv_mask", lpv_mask, 0, 0x0FFF)
    hpv_mask = _require_integer("hpv_mask", hpv_mask, 0, 0xFF)
    heater_mask = _require_integer("heater_mask", heater_mask, 0, 0x0F)
    spark = _require_integer("spark", spark, 0, 1)
    return struct.pack(
        ">BBHHBBBB",
        PROTOCOL_VERSION,
        OPCODE_SET_OUTPUTS,
        transaction_id,
        lpv_mask,
        hpv_mask,
        heater_mask,
        spark,
        0,
    )


def encode_set_mode(transaction_id: int, mode: int) -> bytes:
    transaction_id = _require_integer("transaction_id", transaction_id, 0, 0xFFFF)
    mode = _require_integer("mode", mode, 0, 3)
    return struct.pack(">BBHB", PROTOCOL_VERSION, OPCODE_SET_MODE, transaction_id, mode)


def _encode_header_only_request(transaction_id: int, opcode: int) -> bytes:
    transaction_id = _require_integer("transaction_id", transaction_id, 0, 0xFFFF)
    return struct.pack(">BBH", PROTOCOL_VERSION, opcode, transaction_id)


def encode_get_snapshot(transaction_id: int) -> bytes:
    return _encode_header_only_request(transaction_id, OPCODE_GET_SNAPSHOT)


def encode_get_health(transaction_id: int) -> bytes:
    return _encode_header_only_request(transaction_id, OPCODE_GET_HEALTH)


def _decode_response_header(
    payload: bytes,
    expected_opcode: int,
    expected_transaction_id: int,
) -> StatusResponse:
    if len(payload) < RESPONSE_HEADER_LENGTH:
        raise ProtocolError("response is shorter than the six-byte common header")
    version, opcode, transaction_id, status, detail = struct.unpack(">BBHBB", payload[:6])
    if version != PROTOCOL_VERSION:
        raise ProtocolError(f"response version {version} is not {PROTOCOL_VERSION}")
    if opcode != expected_opcode:
        raise ProtocolError(f"response opcode {opcode} does not match {expected_opcode}")
    if transaction_id != expected_transaction_id:
        raise ProtocolError(
            f"response transaction 0x{transaction_id:04x} does not match 0x{expected_transaction_id:04x}"
        )
    if status >= len(STATUS_NAMES):
        raise ProtocolError(f"unknown response status {status}")
    if status != 0 and len(payload) != RESPONSE_HEADER_LENGTH:
        raise ProtocolError("error response must be exactly six bytes")
    if status == 0 and detail != 0:
        raise ProtocolError("OK response detail must be zero")
    return StatusResponse(version, opcode, transaction_id, status, detail)


def decode_status_response(
    payload: bytes,
    expected_opcode: int,
    expected_transaction_id: int,
) -> StatusResponse:
    if len(payload) != RESPONSE_HEADER_LENGTH:
        raise ProtocolError("status response must be exactly six bytes")
    return _decode_response_header(payload, expected_opcode, expected_transaction_id)


def _require_ok(response: StatusResponse) -> None:
    if response.status != 0:
        raise RemoteStatusError(response.status, response.detail)


def decode_snapshot_response(payload: bytes, expected_transaction_id: int) -> SnapshotResponse:
    response = _decode_response_header(payload, OPCODE_GET_SNAPSHOT, expected_transaction_id)
    _require_ok(response)
    if len(payload) != SNAPSHOT_RESPONSE_LENGTH:
        raise ProtocolError(f"snapshot response must be exactly {SNAPSHOT_RESPONSE_LENGTH} bytes")
    sample_time_ms, current_mode, requested_mode, validity_mask = struct.unpack(">IBBH", payload[6:14])
    if current_mode > 3 or requested_mode > 3:
        raise ProtocolError("snapshot mode is outside 0..3")
    if validity_mask & 0xE000:
        raise ProtocolError("snapshot validity mask has reserved bits set")
    pt_values = struct.unpack(">9i", payload[14:50])
    tc_values = struct.unpack(">4i", payload[50:66])
    return SnapshotResponse(
        response.version,
        response.opcode,
        response.transaction_id,
        response.status,
        response.detail,
        sample_time_ms,
        current_mode,
        requested_mode,
        validity_mask,
        pt_values,
        tc_values,
    )


def decode_health_response(payload: bytes, expected_transaction_id: int) -> HealthResponse:
    response = _decode_response_header(payload, OPCODE_GET_HEALTH, expected_transaction_id)
    _require_ok(response)
    if len(payload) != HEALTH_RESPONSE_LENGTH:
        raise ProtocolError(f"health response must be exactly {HEALTH_RESPONSE_LENGTH} bytes")
    uptime_ms, link_state, last_error, reserved = struct.unpack(">IBBH", payload[6:14])
    if link_state > 2:
        raise ProtocolError("health link state is outside 0..2")
    if last_error > 4:
        raise ProtocolError("health last error is outside 0..4")
    if reserved != 0:
        raise ProtocolError("health reserved field must be zero")
    counters = struct.unpack(">11I", payload[14:58])
    return HealthResponse(
        response.version,
        response.opcode,
        response.transaction_id,
        response.status,
        response.detail,
        uptime_ms,
        link_state,
        last_error,
        counters,
    )


def _request_identity(application_payload: bytes) -> tuple[int, int]:
    if len(application_payload) < REQUEST_HEADER_LENGTH:
        raise ValueError("application request is shorter than four bytes")
    version, opcode, transaction_id = struct.unpack(">BBH", application_payload[:4])
    if version != PROTOCOL_VERSION:
        raise ValueError(f"request version must be {PROTOCOL_VERSION}")
    return opcode, transaction_id


def read_kiss_frame(serial_port, timeout_seconds: float) -> bytes:
    if timeout_seconds <= 0:
        raise ValueError("timeout_seconds must be positive")
    deadline = time.monotonic() + timeout_seconds
    frame = bytearray()
    started = False
    while time.monotonic() < deadline:
        chunk = serial_port.read(1)
        if not chunk:
            continue
        for byte in chunk:
            if not started:
                if byte == KISS_FEND:
                    frame[:] = bytes((KISS_FEND,))
                    started = True
                continue
            if byte == KISS_FEND:
                if len(frame) == 1:
                    continue
                if len(frame) >= MAX_KISS_FRAME_SIZE:
                    raise ProtocolError(f"KISS frame exceeds maximum {MAX_KISS_FRAME_SIZE} bytes")
                frame.append(byte)
                return bytes(frame)
            if len(frame) >= MAX_KISS_FRAME_SIZE:
                raise ProtocolError(f"KISS frame exceeds maximum {MAX_KISS_FRAME_SIZE} bytes")
            frame.append(byte)
    raise PeerTimeoutError(f"no complete KISS frame within {timeout_seconds:.3f} seconds")


def transact(
    serial_port,
    destination_port: int,
    application_payload: bytes,
    timeout_seconds: float,
) -> TransactionResult:
    """Perform exactly one synchronous request/write/flush and receive one reply."""
    destination_port = _require_integer("destination_port", destination_port, 0, 63)
    opcode, transaction_id = _request_identity(application_payload)
    request_header = encode_csp_header(
        CSP_PRIORITY_NORMAL,
        PEER_ADDRESS,
        LOCAL_ADDRESS,
        destination_port,
        CSP_SOURCE_PORT,
        0,
    )
    request_frame = kiss_encode(build_raw_frame(request_header, application_payload))

    start = time.monotonic()
    written = serial_port.write(request_frame)
    if written != len(request_frame):
        raise PeerError(f"serial write accepted {written} of {len(request_frame)} bytes")
    serial_port.flush()
    response_frame = read_kiss_frame(serial_port, timeout_seconds)
    response_raw = kiss_decode(response_frame)
    header, response_payload = decode_raw_frame(response_raw)
    latency = time.monotonic() - start

    expected_header = CspHeader(
        CSP_PRIORITY_NORMAL,
        LOCAL_ADDRESS,
        PEER_ADDRESS,
        CSP_SOURCE_PORT,
        destination_port,
        0,
    )
    if header != expected_header:
        raise ProtocolError(f"unexpected response CSP header {header!r}")
    _decode_response_header(response_payload, opcode, transaction_id)
    return TransactionResult(header, response_payload, request_frame, response_frame, latency)


def _parse_integer(value: str) -> int:
    try:
        return int(value, 0)
    except ValueError as error:
        raise argparse.ArgumentTypeError(str(error)) from error


def _positive_float(value: str) -> float:
    try:
        parsed = float(value)
    except ValueError as error:
        raise argparse.ArgumentTypeError(str(error)) from error
    if parsed <= 0:
        raise argparse.ArgumentTypeError("value must be positive")
    return parsed


def _nonnegative_float(value: str) -> float:
    try:
        parsed = float(value)
    except ValueError as error:
        raise argparse.ArgumentTypeError(str(error)) from error
    if parsed < 0:
        raise argparse.ArgumentTypeError("value must be nonnegative")
    return parsed


def _positive_integer(value: str) -> int:
    parsed = _parse_integer(value)
    if parsed <= 0:
        raise argparse.ArgumentTypeError("value must be positive")
    return parsed


def _add_online_options(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--device", required=True, help="explicit serial device, for example COM7 or /dev/ttyUSB0")
    parser.add_argument("--baud", type=_positive_integer, default=DEFAULT_BAUD)
    parser.add_argument("--timeout-seconds", type=_positive_float, default=DEFAULT_TIMEOUT_SECONDS)


def build_argument_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)

    selftest = commands.add_parser("selftest", help="verify built-in codecs against golden vectors without serial")
    selftest.add_argument("--vectors", type=Path, default=_default_vector_path())

    set_outputs = commands.add_parser("set-outputs", help="set every actuator output as an absolute mask")
    _add_online_options(set_outputs)
    set_outputs.add_argument("--lpv-mask", type=_parse_integer, required=True)
    set_outputs.add_argument("--hpv-mask", type=_parse_integer, required=True)
    set_outputs.add_argument("--heater-mask", type=_parse_integer, required=True)
    set_outputs.add_argument("--spark", type=_parse_integer, required=True)

    set_mode = commands.add_parser("set-mode", help="request mode 0..3")
    _add_online_options(set_mode)
    set_mode.add_argument("--mode", type=_parse_integer, required=True)

    for name, help_text in (
        ("snapshot", "read one coherent sensor snapshot"),
        ("health", "read link health and counters"),
        ("smoke", "perform one request for every binary service"),
    ):
        command = commands.add_parser(name, help=help_text)
        _add_online_options(command)

    sequence = commands.add_parser("sequence", help="alternate snapshot and health traffic for a duration")
    _add_online_options(sequence)
    sequence.add_argument("--duration-seconds", type=_positive_float, required=True)
    sequence.add_argument("--interval-ms", type=_nonnegative_float, default=100.0)

    rx_burst = commands.add_parser("rx-burst", help="send a finite back-to-back snapshot burst")
    _add_online_options(rx_burst)
    rx_burst.add_argument("--count", type=_positive_integer, default=100)
    rx_burst.add_argument("--interval-ms", type=_nonnegative_float, default=0.0)

    recovery = commands.add_parser("recovery", help="inject wrong-baud bytes and observe health recovery")
    _add_online_options(recovery)
    recovery.add_argument("--iterations", type=_positive_integer, default=100)
    recovery.add_argument("--fault-baud", type=_positive_integer, default=115200)
    recovery.add_argument("--fault-bytes", type=_positive_integer, default=64)
    recovery.add_argument("--settle-seconds", type=_nonnegative_float, default=0.20)
    return parser


def _default_vector_path() -> Path:
    return Path(__file__).resolve().parents[1] / "tests" / "rs485_peer" / "golden_vectors.json"


def _load_vector_document(path: Path) -> dict:
    with path.open("r", encoding="utf-8") as stream:
        return json.load(stream)


def _validate_crc_error_vector(vector: dict, normal_vectors: dict[str, dict]) -> None:
    name = vector.get("name", "unnamed-error-vector")
    try:
        based_on = vector["based_on"]
        base_vector = normal_vectors[based_on]
        application = bytes.fromhex(vector["application_payload_hex"])
        header = bytes.fromhex(vector["csp_header_hex"])
        expected_crc = bytes.fromhex(vector["expected_crc32c_hex"])
        corrupt_crc = bytes.fromhex(vector["crc32c_hex"])
        raw = bytes.fromhex(vector["raw_frame_hex"])
        kiss = bytes.fromhex(vector["kiss_frame_hex"])
        expected_error = vector["expected_error"]
        base_application = bytes.fromhex(base_vector["application_payload_hex"])
        base_header = bytes.fromhex(base_vector["csp_header_hex"])
        base_crc = bytes.fromhex(base_vector["crc32c_hex"])
    except (KeyError, TypeError, ValueError) as error:
        raise ProtocolError(f"{name}: malformed CRC error metadata") from error

    calculated_crc = struct.pack(">I", crc32c(application))
    if application != base_application or header != base_header or expected_crc != base_crc:
        raise ProtocolError(f"{name}: based_on metadata mismatch")
    if expected_error != "crc32c":
        raise ProtocolError(f"{name}: expected_error must be crc32c")
    if len(header) != 4 or len(expected_crc) != 4 or len(corrupt_crc) != 4:
        raise ProtocolError(f"{name}: CSP header and CRC fields must be four bytes")
    if expected_crc != calculated_crc:
        raise ProtocolError(f"{name}: expected CRC32C metadata mismatch")
    if corrupt_crc == expected_crc:
        raise ProtocolError(f"{name}: corrupt CRC32C equals the expected value")
    if raw != header + application + corrupt_crc:
        raise ProtocolError(f"{name}: corrupt raw frame metadata mismatch")
    if kiss != kiss_encode(raw):
        raise ProtocolError(f"{name}: corrupt KISS frame metadata mismatch")
    decoded_raw = kiss_decode(kiss)
    if decoded_raw != raw:
        raise ProtocolError(f"{name}: corrupt KISS frame did not round-trip")
    try:
        decode_raw_frame(decoded_raw)
    except CrcError:
        return
    raise ProtocolError(f"{name}: corrupt CRC was accepted")


def run_selftest(path: Path | None = None, output: TextIO = sys.stdout) -> None:
    document = _load_vector_document(path or _default_vector_path())
    expected_constants = {
        "protocol_version": PROTOCOL_VERSION,
        "local_address": LOCAL_ADDRESS,
        "peer_address": PEER_ADDRESS,
        "priority": CSP_PRIORITY_NORMAL,
        "source_port": CSP_SOURCE_PORT,
        "command_port": COMMAND_PORT,
        "telemetry_port": TELEMETRY_PORT,
        "diagnostic_port": DIAGNOSTIC_PORT,
        "request_header_length": REQUEST_HEADER_LENGTH,
        "response_header_length": RESPONSE_HEADER_LENGTH,
        "set_outputs_request_length": SET_OUTPUTS_REQUEST_LENGTH,
        "set_mode_request_length": SET_MODE_REQUEST_LENGTH,
        "snapshot_response_length": SNAPSHOT_RESPONSE_LENGTH,
        "health_response_length": HEALTH_RESPONSE_LENGTH,
        "snapshot_pt_count": SNAPSHOT_PT_COUNT,
        "snapshot_tc_count": SNAPSHOT_TC_COUNT,
        "health_counter_count": HEALTH_COUNTER_COUNT,
        "csp_priority_bits": 2,
        "csp_host_bits": 5,
        "csp_port_bits": 6,
        "csp_flag_bits": 8,
    }
    if document.get("schema") != "sam-csp-binary-v1" or document.get("constants") != expected_constants:
        raise ProtocolError("golden-vector schema or constants do not match the peer")

    for section in ("normal", "validation"):
        for vector in document[section]:
            header = encode_csp_header(
                vector["priority"],
                vector["source"],
                vector["destination"],
                vector["destination_port"],
                vector["source_port"],
                vector["flags"],
            )
            application = bytes.fromhex(vector["application_payload_hex"])
            raw = build_raw_frame(header, application)
            if header.hex() != vector["csp_header_hex"]:
                raise ProtocolError(f"{vector['name']}: CSP header mismatch")
            if struct.pack(">I", crc32c(application)).hex() != vector["crc32c_hex"]:
                raise ProtocolError(f"{vector['name']}: CRC32C mismatch")
            if raw.hex() != vector["raw_frame_hex"] or kiss_encode(raw).hex() != vector["kiss_frame_hex"]:
                raise ProtocolError(f"{vector['name']}: frame mismatch")
            decoded_header, decoded_application = decode_raw_frame(kiss_decode(bytes.fromhex(vector["kiss_frame_hex"])))
            if decoded_header.flags != 0 or decoded_application != application:
                raise ProtocolError(f"{vector['name']}: round-trip mismatch")

    normal_vectors = {vector["name"]: vector for vector in document["normal"]}
    for vector in document["errors"]:
        _validate_crc_error_vector(vector, normal_vectors)

    snapshot_vector = next(vector for vector in document["normal"] if vector["name"] == "snapshot_signed_response")
    decode_snapshot_response(bytes.fromhex(snapshot_vector["application_payload_hex"]), 0xBEEF)
    health_vector = next(vector for vector in document["normal"] if vector["name"] == "health_response")
    decode_health_response(bytes.fromhex(health_vector["application_payload_hex"]), 0xCAFE)
    print("PASS golden vectors", file=output)


def _open_serial(device: str, baud: int, timeout_seconds: float):
    import serial  # pyserial is intentionally lazy: offline tests/selftest never import it.

    return serial.Serial(
        port=device,
        baudrate=baud,
        bytesize=serial.EIGHTBITS,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        timeout=min(timeout_seconds, 0.05),
        write_timeout=timeout_seconds,
    )


class _TransactionIds:
    def __init__(self):
        self._next = (time.monotonic_ns() >> 8) & 0xFFFF

    def take(self) -> int:
        value = self._next
        self._next = (self._next + 1) & 0xFFFF
        return value


def _record(result: TransactionResult, decoded) -> dict:
    value = asdict(decoded)
    if isinstance(decoded, HealthResponse):
        value["named_counters"] = decoded.named_counters()
    return {
        "request_hex": result.request_frame.hex(),
        "response_hex": result.response_frame.hex(),
        "transaction_id": int.from_bytes(result.application_payload[2:4], "big"),
        "latency_ms": round(result.latency_seconds * 1000.0, 3),
        "response": value,
    }


def _status_transaction(serial_port, destination_port: int, payload: bytes, timeout: float) -> dict:
    opcode, transaction_id = _request_identity(payload)
    result = transact(serial_port, destination_port, payload, timeout)
    response = decode_status_response(result.application_payload, opcode, transaction_id)
    _require_ok(response)
    return _record(result, response)


def _snapshot_transaction(serial_port, transaction_id: int, timeout: float) -> tuple[dict, SnapshotResponse]:
    result = transact(serial_port, TELEMETRY_PORT, encode_get_snapshot(transaction_id), timeout)
    response = decode_snapshot_response(result.application_payload, transaction_id)
    return _record(result, response), response


def _health_transaction(serial_port, transaction_id: int, timeout: float) -> tuple[dict, HealthResponse]:
    result = transact(serial_port, DIAGNOSTIC_PORT, encode_get_health(transaction_id), timeout)
    response = decode_health_response(result.application_payload, transaction_id)
    return _record(result, response), response


def _print_json(value) -> None:
    print(json.dumps(value, indent=2, sort_keys=True))


def _run_single(args, ids: _TransactionIds) -> int:
    with _open_serial(args.device, args.baud, args.timeout_seconds) as serial_port:
        transaction_id = ids.take()
        if args.command == "set-outputs":
            payload = encode_set_outputs(transaction_id, args.lpv_mask, args.hpv_mask, args.heater_mask, args.spark)
            _print_json(_status_transaction(serial_port, COMMAND_PORT, payload, args.timeout_seconds))
        elif args.command == "set-mode":
            payload = encode_set_mode(transaction_id, args.mode)
            _print_json(_status_transaction(serial_port, COMMAND_PORT, payload, args.timeout_seconds))
        elif args.command == "snapshot":
            record, _ = _snapshot_transaction(serial_port, transaction_id, args.timeout_seconds)
            _print_json(record)
        elif args.command == "health":
            record, _ = _health_transaction(serial_port, transaction_id, args.timeout_seconds)
            _print_json(record)
        else:
            raise AssertionError(f"unsupported single command {args.command}")
    return 0


def _run_smoke(args, ids: _TransactionIds) -> int:
    with _open_serial(args.device, args.baud, args.timeout_seconds) as serial_port:
        records = [
            _status_transaction(
                serial_port,
                COMMAND_PORT,
                encode_set_outputs(ids.take(), 0, 0, 0, 0),
                args.timeout_seconds,
            ),
            _status_transaction(serial_port, COMMAND_PORT, encode_set_mode(ids.take(), 1), args.timeout_seconds),
            _snapshot_transaction(serial_port, ids.take(), args.timeout_seconds)[0],
            _health_transaction(serial_port, ids.take(), args.timeout_seconds)[0],
        ]
    _print_json({"command": "smoke", "transactions": records})
    return 0


def _classify_error(error: Exception) -> str:
    if isinstance(error, PeerTimeoutError):
        return "timeouts"
    if isinstance(error, CrcError):
        return "crc_errors"
    if isinstance(error, RemoteStatusError):
        return "remote_status_errors"
    return "malformed_responses"


def _run_traffic(args, ids: _TransactionIds, count: int | None) -> int:
    summary = {
        "attempted": 0,
        "completed": 0,
        "timeouts": 0,
        "crc_errors": 0,
        "remote_status_errors": 0,
        "malformed_responses": 0,
        "maximum_latency_ms": 0.0,
    }
    first_transaction = None
    last_transaction = None
    start = time.monotonic()
    with _open_serial(args.device, args.baud, args.timeout_seconds) as serial_port:
        while count is None or summary["attempted"] < count:
            if count is None and time.monotonic() - start >= args.duration_seconds:
                break
            summary["attempted"] += 1
            try:
                if count is not None or summary["attempted"] % 2:
                    record, _ = _snapshot_transaction(serial_port, ids.take(), args.timeout_seconds)
                else:
                    record, _ = _health_transaction(serial_port, ids.take(), args.timeout_seconds)
                summary["completed"] += 1
                summary["maximum_latency_ms"] = max(summary["maximum_latency_ms"], record["latency_ms"])
                if first_transaction is None:
                    first_transaction = record
                last_transaction = record
            except (PeerError, ValueError) as error:
                summary[_classify_error(error)] += 1
            if args.interval_ms > 0:
                time.sleep(args.interval_ms / 1000.0)
    summary["elapsed_seconds"] = round(time.monotonic() - start, 3)
    summary["first_transaction"] = first_transaction
    summary["last_transaction"] = last_transaction
    _print_json(summary)
    errors = summary["attempted"] - summary["completed"]
    return 0 if errors == 0 else 1


def _counter_delta(before: HealthResponse, after: HealthResponse) -> dict[str, int]:
    return {
        name: (new - old) & 0xFFFFFFFF
        for name, old, new in zip(HEALTH_COUNTER_NAMES, before.counters, after.counters, strict=True)
    }


def _run_recovery(args, ids: _TransactionIds) -> int:
    trials = []
    failures = 0
    inconclusive = 0
    with _open_serial(args.device, args.baud, args.timeout_seconds) as serial_port:
        for iteration in range(args.iterations):
            before_record, before = _health_transaction(serial_port, ids.take(), args.timeout_seconds)
            serial_port.baudrate = args.fault_baud
            fault_data = bytes((0x55,)) * args.fault_bytes
            written = serial_port.write(fault_data)
            if written != len(fault_data):
                raise PeerError(f"fault write accepted {written} of {len(fault_data)} bytes")
            serial_port.flush()
            recovery_epoch = time.monotonic()
            time.sleep(args.settle_seconds)
            serial_port.baudrate = args.baud

            try:
                after_record, after = _health_transaction(serial_port, ids.take(), args.timeout_seconds)
                actual_post_fault_elapsed_ms = (time.monotonic() - recovery_epoch) * 1000.0
                reported_post_fault_elapsed_ms = round(actual_post_fault_elapsed_ms, 3)
                delta = _counter_delta(before, after)
                conclusive = delta["uart_errors"] > 0
                passed = (
                    conclusive
                    and delta["recovery_attempts"] > 0
                    and delta["recovery_successes"] > 0
                    and after.link_state == 1
                    and actual_post_fault_elapsed_ms <= RECOVERY_ACCEPTANCE_MS
                )
                if not conclusive:
                    outcome = "inconclusive"
                    inconclusive += 1
                elif passed:
                    outcome = "pass"
                else:
                    outcome = "fail"
                    failures += 1
                trials.append(
                    {
                        "iteration": iteration + 1,
                        "outcome": outcome,
                        "counter_delta": delta,
                        "post_fault_elapsed_ms": reported_post_fault_elapsed_ms,
                        "post_request_latency_ms": after_record["latency_ms"],
                        "before_transaction": before_record,
                        "after_transaction": after_record,
                        "output_observation_required": True,
                    }
                )
            except PeerError as error:
                failures += 1
                trials.append(
                    {
                        "iteration": iteration + 1,
                        "outcome": "fail",
                        "post_fault_elapsed_ms": round((time.monotonic() - recovery_epoch) * 1000.0, 3),
                        "error": str(error),
                    }
                )
    _print_json(
        {
            "iterations": args.iterations,
            "passed": args.iterations - failures - inconclusive,
            "failed": failures,
            "inconclusive": inconclusive,
            "trials": trials,
        }
    )
    return 0 if failures == 0 and inconclusive == 0 else 1


def main(argv: Sequence[str] | None = None) -> int:
    args = build_argument_parser().parse_args(argv)
    if args.command == "selftest":
        run_selftest(args.vectors)
        return 0

    ids = _TransactionIds()
    try:
        if args.command in {"set-outputs", "set-mode", "snapshot", "health"}:
            return _run_single(args, ids)
        if args.command == "smoke":
            return _run_smoke(args, ids)
        if args.command == "sequence":
            return _run_traffic(args, ids, count=None)
        if args.command == "rx-burst":
            return _run_traffic(args, ids, count=args.count)
        if args.command == "recovery":
            return _run_recovery(args, ids)
        raise AssertionError(f"unhandled command {args.command}")
    except (PeerError, ValueError) as error:
        print(f"ERROR {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
