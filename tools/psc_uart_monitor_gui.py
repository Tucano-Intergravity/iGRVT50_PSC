#!/usr/bin/env python3
"""
PSC CSP/RS485 master GUI.

USART1 is no longer an ASCII $iGRVT50 line protocol.  This tool acts as the
OBC-side CSP master and sends one request at a time.  PSC only transmits as a
response to these requests.
"""

from __future__ import annotations

import queue
import math
import struct
import threading
import time
import tkinter as tk
from dataclasses import dataclass
from datetime import datetime
from tkinter import messagebox, ttk

try:
    import serial
    from serial.tools import list_ports
except ImportError:
    serial = None
    list_ports = None


DEFAULT_BAUDRATE = 921600
SERIAL_READ_TIMEOUT_SEC = 0.02
SERIAL_PORT_SETTLE_SEC = 0.2
COMMAND_RESPONSE_TIMEOUT_SEC = 0.5
COMMAND_MAX_RETRIES = 5
AUTO_HEALTH_LABEL = "GET_HEALTH_AUTO"
AUTO_HEALTH_POLL_MS = 1000

PSC_CSP_ADDRESS = 0x10
OBC_CSP_ADDRESS = 0x0A
CSP_PRIO_NORM = 2
CSP_SOURCE_PORT_FIRST = 14
CSP_SOURCE_PORT_LAST = 63

CSP_PORT_PING = 1
CSP_PORT_REBOOT = 4
CSP_PORT_COMMAND = 10
CSP_PORT_TELEMETRY = 11
CSP_PORT_DIAGNOSTICS = 12

CSP_PROTOCOL_VERSION = 1
CSP_REBOOT_MAGIC = 0x80078007
CSP_PING_PAYLOAD = b"PSC-PING"

OP_SET_OUTPUTS = 0x01
OP_SET_MODE = 0x02
OP_THRUSTER_START = 0x03
OP_PAR_START = 0x04
OP_PAR_STOP = 0x05
OP_SET_LPV_OUTPUTS = 0x06
OP_SET_SIM_SENSOR_VALUES = 0x07
OP_SIM_START = 0x08
OP_SIM_STOP = 0x09
OP_GET_SENSOR_SNAPSHOT = 0x01
OP_GET_SOLVALVE_STATE = 0x02
OP_GET_HEALTH = 0x01

STATUS_NAMES = {
    0: "OK",
    1: "BAD_VERSION",
    2: "BAD_LENGTH",
    3: "BAD_OPCODE",
    4: "INVALID_ARGUMENT",
    5: "INVALID_STATE",
    6: "APPLY_FAILED",
    7: "INTERNAL_ERROR",
    8: "BUSY",
    255: "DROP",
}

MODE_OPTIONS = ("init_mode", "normal_mode", "run_mode", "diagnostic_mode")
MODE_TO_VALUE = {name: index for index, name in enumerate(MODE_OPTIONS)}
MODE_NAMES = {value: name for name, value in MODE_TO_VALUE.items()}

PT_COUNT = 9
TC_COUNT = 5
LPV_COUNT = 12
HPV_COUNT = 8
HTR_COUNT = 4
PAR_OXIDIZER_PT_OPTIONS = ("PT-O3", "PT-O4")
PAR_FUEL_PT_OPTIONS = ("PT-F3", "PT-F4")

PT_LABELS = (
    "PT-O1\nPT1",
    "PT-O2\nPT2",
    "PT-O3\nPT3",
    "PT-O4\nPT4",
    "PT-F1\nPT5",
    "PT-F2\nPT6",
    "PT-F3\nPT7",
    "PT-F4\nPT8",
    "PT-C1\nPT9",
)
LPV_LABELS = tuple(f"SV-R{idx + 1:02d}\nLPV{idx + 1}" for idx in range(LPV_COUNT))
HPV_LABELS = (
    "SV-O1\nHPV1",
    "SV-O2\nHPV2",
    "SV-O3\nHPV3",
    "SPARE\nHPV4",
    "SV-F1\nHPV5",
    "SV-F2\nHPV6",
    "SV-F3\nHPV7",
    "SPARE\nHPV8",
)
TC_LABELS = (
    "TC1\nAIN0/1",
    "TC2\nAIN2/3",
    "TC3\nAIN4/5",
    "TC4\nAIN6/7",
    "CJC1 10k NTC\nAIN8/9",
)

SENSOR_RESPONSE_LENGTH = 126
SOLVALVE_RESPONSE_LENGTH = 16
HEALTH_DEBUG_MAX_MESSAGES = 4
HEALTH_RESPONSE_LENGTH = 106

KISS_FEND = 0xC0
KISS_FESC = 0xDB
KISS_TFEND = 0xDC
KISS_TFESC = 0xDD
KISS_TNC_DATA = 0x00

CRC32C_POLY_REVERSED = 0x82F63B78


@dataclass
class CspId:
    priority: int
    source: int
    destination: int
    destination_port: int
    source_port: int
    flags: int


@dataclass
class CspPacket:
    csp_id: CspId
    payload: bytes
    raw_frame_hex: str


@dataclass
class SensorPacket:
    tick_ms: int
    current_mode: int
    requested_mode: int
    validity_mask: int
    pt_values: list[int]
    pt_pressure_values: list[int]
    tc_values: list[int]
    tc_temperature_values: list[int]


@dataclass
class SolvalvePacket:
    tick_ms: int
    current_mode: int
    lpv_mask: int
    hpv_mask: int
    heater_mask: int
    spark_on: int


@dataclass
class DebugMessage:
    debug_sequence: int
    debug_elapsed_ms: int
    debug_source: int
    debug_event: int
    debug_mode: int


@dataclass
class HealthPacket:
    uptime_ms: int
    current_mode: int
    link_state: int
    last_error: int
    debug_messages: list[DebugMessage]
    counters: list[int]


@dataclass
class TxRequest:
    label: str
    frame: bytes


@dataclass
class PendingCommand:
    label: str
    port: int
    opcode: int
    transaction_id: int
    source_port: int
    frame: bytes
    expected_kind: str
    expected_payload: bytes = b""
    retries_done: int = 0
    deadline: float = 0.0
    waiting_for_tx: bool = False


def crc32c(data: bytes) -> int:
    crc = 0xFFFFFFFF
    for value in data:
        crc ^= value
        for _ in range(8):
            if crc & 1:
                crc = (crc >> 1) ^ CRC32C_POLY_REVERSED
            else:
                crc >>= 1
            crc &= 0xFFFFFFFF
    return crc ^ 0xFFFFFFFF


def pack_csp_id(csp_id: CspId) -> int:
    return (
        ((csp_id.priority & 0x03) << 30)
        | ((csp_id.source & 0x1F) << 25)
        | ((csp_id.destination & 0x1F) << 20)
        | ((csp_id.destination_port & 0x3F) << 14)
        | ((csp_id.source_port & 0x3F) << 8)
        | (csp_id.flags & 0xFF)
    )


def unpack_csp_id(value: int) -> CspId:
    return CspId(
        priority=(value >> 30) & 0x03,
        source=(value >> 25) & 0x1F,
        destination=(value >> 20) & 0x1F,
        destination_port=(value >> 14) & 0x3F,
        source_port=(value >> 8) & 0x3F,
        flags=value & 0xFF,
    )


def escape_kiss(raw: bytes) -> bytes:
    escaped = bytearray()
    for value in raw:
        if value == KISS_FEND:
            escaped.extend((KISS_FESC, KISS_TFEND))
        elif value == KISS_FESC:
            escaped.extend((KISS_FESC, KISS_TFESC))
        else:
            escaped.append(value)
    return bytes(escaped)


def build_csp_frame(destination_port: int, source_port: int, payload: bytes) -> bytes:
    csp_id = CspId(
        priority=CSP_PRIO_NORM,
        source=OBC_CSP_ADDRESS,
        destination=PSC_CSP_ADDRESS,
        destination_port=destination_port,
        source_port=source_port,
        flags=0,
    )
    header = pack_csp_id(csp_id).to_bytes(4, "big")
    checksum = crc32c(payload).to_bytes(4, "big")
    return bytes((KISS_FEND, KISS_TNC_DATA)) + escape_kiss(header + payload + checksum) + bytes((KISS_FEND,))


def decode_csp_raw_frame(raw: bytes) -> CspPacket:
    if len(raw) < 8:
        raise ValueError(f"short frame length {len(raw)}")

    csp_id = unpack_csp_id(int.from_bytes(raw[:4], "big"))
    payload_with_crc = raw[4:]
    if len(payload_with_crc) < 4:
        raise ValueError("missing CRC")

    payload = payload_with_crc[:-4]
    received_crc = int.from_bytes(payload_with_crc[-4:], "big")
    expected_crc = crc32c(payload)
    if received_crc != expected_crc:
        raise ValueError(f"CRC mismatch rx=0x{received_crc:08X} expected=0x{expected_crc:08X}")

    return CspPacket(
        csp_id=csp_id,
        payload=payload,
        raw_frame_hex=raw.hex(" "),
    )


def status_text(status: int, detail: int) -> str:
    return f"{STATUS_NAMES.get(status, f'UNKNOWN_{status}')} detail={detail}"


def mode_text(mode: int) -> str:
    return MODE_NAMES.get(mode, "unknown")


def debug_source_text(source: int) -> str:
    return {
        1: "THRUSTER",
        2: "THRUSTER_CHECK",
        3: "PAR",
    }.get(source, f"SRC_{source}")


def debug_event_text(source: int, event: int) -> str:
    if source == 1:
        return {
            0: "SV-O3_ON",
            1: "SV-F3_ON",
            2: "SP_ON",
            3: "SV-F3_OFF",
            4: "SV-O3_OFF",
            5: "SP_OFF",
        }.get(event, f"EVENT_{event}")
    if source == 2:
        return {
            0: "PRE_RUN_CHECK",
            1: "RUN_MONITOR_1HZ",
        }.get(event, f"EVENT_{event}")
    if source == 3:
        return {
            0: "PAR_ROUTINE_1HZ",
            1: "PAR_START",
            2: "PAR_STOP",
        }.get(event, f"EVENT_{event}")
    return f"EVENT_{event}"


def mask_bit(mask: int, bit_index: int) -> int:
    return 1 if (mask & (1 << bit_index)) else 0


def parse_common_status(payload: bytes, expected_opcode: int, transaction_id: int) -> tuple[int, int]:
    if len(payload) < 6:
        raise ValueError(f"short response length {len(payload)}")
    version, opcode, response_tid, status, detail = struct.unpack_from(">BBHBB", payload, 0)
    if version != CSP_PROTOCOL_VERSION:
        raise ValueError(f"version {version} != {CSP_PROTOCOL_VERSION}")
    if opcode != expected_opcode:
        raise ValueError(f"opcode 0x{opcode:02X} != 0x{expected_opcode:02X}")
    if response_tid != transaction_id:
        raise ValueError(f"transaction_id {response_tid} != {transaction_id}")
    return status, detail


def parse_sensor_response(
    payload: bytes,
    transaction_id: int,
    expected_opcode: int = OP_GET_SENSOR_SNAPSHOT,
) -> tuple[int, int, SensorPacket | None]:
    status, detail = parse_common_status(payload, expected_opcode, transaction_id)
    if status != 0:
        return status, detail, None
    if len(payload) != SENSOR_RESPONSE_LENGTH:
        raise ValueError(f"sensor response length {len(payload)} != {SENSOR_RESPONSE_LENGTH}")

    offset = 6
    tick_ms = struct.unpack_from(">I", payload, offset)[0]
    offset += 4
    current_mode = payload[offset]
    offset += 1
    requested_mode = payload[offset]
    offset += 1
    validity_mask = struct.unpack_from(">H", payload, offset)[0]
    offset += 2
    pt_values = list(struct.unpack_from(">9i", payload, offset))
    offset += 4 * PT_COUNT
    pt_pressure_values = list(struct.unpack_from(">9i", payload, offset))
    offset += 4 * PT_COUNT
    tc_values = list(struct.unpack_from(f">{TC_COUNT}i", payload, offset))
    offset += 4 * TC_COUNT
    tc_temperature_values = list(struct.unpack_from(f">{TC_COUNT}i", payload, offset))
    return status, detail, SensorPacket(
        tick_ms=tick_ms,
        current_mode=current_mode,
        requested_mode=requested_mode,
        validity_mask=validity_mask,
        pt_values=pt_values,
        pt_pressure_values=pt_pressure_values,
        tc_values=tc_values,
        tc_temperature_values=tc_temperature_values,
    )


def parse_solvalve_response(payload: bytes, transaction_id: int) -> tuple[int, int, SolvalvePacket | None]:
    status, detail = parse_common_status(payload, OP_GET_SOLVALVE_STATE, transaction_id)
    if status != 0:
        return status, detail, None
    if len(payload) != SOLVALVE_RESPONSE_LENGTH:
        raise ValueError(f"solvalve response length {len(payload)} != {SOLVALVE_RESPONSE_LENGTH}")

    offset = 6
    tick_ms = struct.unpack_from(">I", payload, offset)[0]
    offset += 4
    current_mode = payload[offset]
    offset += 1
    lpv_mask = struct.unpack_from(">H", payload, offset)[0]
    offset += 2
    hpv_mask = payload[offset]
    offset += 1
    heater_mask = payload[offset]
    offset += 1
    spark_on = payload[offset]
    return status, detail, SolvalvePacket(
        tick_ms=tick_ms,
        current_mode=current_mode,
        lpv_mask=lpv_mask,
        hpv_mask=hpv_mask,
        heater_mask=heater_mask,
        spark_on=spark_on,
    )


def parse_health_response(payload: bytes, transaction_id: int) -> tuple[int, int, HealthPacket | None]:
    status, detail = parse_common_status(payload, OP_GET_HEALTH, transaction_id)
    if status != 0:
        return status, detail, None
    if len(payload) != HEALTH_RESPONSE_LENGTH:
        raise ValueError(f"health response length {len(payload)} != {HEALTH_RESPONSE_LENGTH}")

    offset = 6
    uptime_ms = struct.unpack_from(">I", payload, offset)[0]
    offset += 4
    current_mode = payload[offset]
    offset += 1
    link_state = payload[offset]
    offset += 1
    last_error = payload[offset]
    offset += 1
    debug_count = payload[offset]
    offset += 1
    debug_messages: list[DebugMessage] = []
    active_debug_count = min(debug_count, HEALTH_DEBUG_MAX_MESSAGES)
    for slot in range(HEALTH_DEBUG_MAX_MESSAGES):
        debug_sequence = struct.unpack_from(">I", payload, offset)[0]
        offset += 4
        debug_elapsed_ms = struct.unpack_from(">I", payload, offset)[0]
        offset += 4
        debug_source = payload[offset]
        offset += 1
        debug_event = payload[offset]
        offset += 1
        debug_mode = payload[offset]
        offset += 1
        offset += 1
        if slot < active_debug_count:
            debug_messages.append(
                DebugMessage(
                    debug_sequence=debug_sequence,
                    debug_elapsed_ms=debug_elapsed_ms,
                    debug_source=debug_source,
                    debug_event=debug_event,
                    debug_mode=debug_mode,
                )
            )
    counters = list(struct.unpack_from(">11I", payload, offset))
    return status, detail, HealthPacket(
        uptime_ms=uptime_ms,
        current_mode=current_mode,
        link_state=link_state,
        last_error=last_error,
        debug_messages=debug_messages,
        counters=counters,
    )


class KissFrameDecoder:
    def __init__(self) -> None:
        self._started = False
        self._escaped = False
        self._first = False
        self._buffer = bytearray()

    def feed(self, chunk: bytes) -> list[CspPacket]:
        packets: list[CspPacket] = []
        for value in chunk:
            if not self._started:
                if value == KISS_FEND:
                    self._started = True
                    self._escaped = False
                    self._first = True
                    self._buffer.clear()
                continue

            if self._escaped:
                if value == KISS_TFEND:
                    self._buffer.append(KISS_FEND)
                elif value == KISS_TFESC:
                    self._buffer.append(KISS_FESC)
                else:
                    self._started = False
                    self._escaped = False
                    self._first = False
                    self._buffer.clear()
                self._escaped = False
                continue

            if value == KISS_FESC:
                self._escaped = True
                continue

            if value == KISS_FEND:
                if self._buffer:
                    packets.append(decode_csp_raw_frame(bytes(self._buffer)))
                self._started = True
                self._escaped = False
                self._first = True
                self._buffer.clear()
                continue

            if self._first:
                self._first = False
                if value != KISS_TNC_DATA:
                    self._started = False
                    self._buffer.clear()
                continue

            self._buffer.append(value)

        return packets


class SerialReader(threading.Thread):
    def __init__(self, port: str, baudrate: int, rx_queue: queue.Queue[tuple[str, object]]) -> None:
        super().__init__(daemon=True)
        self._port = port
        self._baudrate = baudrate
        self._rx_queue = rx_queue
        self._tx_queue: queue.Queue[TxRequest] = queue.Queue()
        self._stop_event = threading.Event()
        self._serial_port = None

    def send_frame(self, label: str, frame: bytes) -> None:
        self._tx_queue.put(TxRequest(label=label, frame=frame))

    def stop(self) -> None:
        self._stop_event.set()
        if self._serial_port is not None:
            try:
                self._serial_port.close()
            except serial.SerialException:
                pass

    def run(self) -> None:
        decoder = KissFrameDecoder()

        try:
            self._serial_port = serial.Serial(
                port=self._port,
                baudrate=self._baudrate,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                timeout=SERIAL_READ_TIMEOUT_SEC,
                write_timeout=1.0,
            )
            self._serial_port.reset_input_buffer()
            self._serial_port.reset_output_buffer()
            time.sleep(SERIAL_PORT_SETTLE_SEC)
            if self._stop_event.is_set():
                return
            self._rx_queue.put(("opened", f"Opened {self._port} at {self._baudrate} bps, 8N1 CSP/RS485"))

            while not self._stop_event.is_set():
                while True:
                    try:
                        request = self._tx_queue.get_nowait()
                    except queue.Empty:
                        break
                    self._serial_port.write(request.frame)
                    self._serial_port.flush()
                    self._rx_queue.put(("tx", request))

                chunk = self._serial_port.read(self._serial_port.in_waiting or 1)
                if not chunk:
                    continue

                try:
                    packets = decoder.feed(chunk)
                except ValueError as exc:
                    self._rx_queue.put(("parse_error", str(exc)))
                    decoder = KissFrameDecoder()
                    continue

                for packet in packets:
                    self._rx_queue.put(("packet", packet))

        except serial.SerialException as exc:
            self._rx_queue.put(("serial_error", str(exc)))
        finally:
            if self._serial_port is not None and self._serial_port.is_open:
                self._serial_port.close()
            self._rx_queue.put(("closed", "Serial port closed"))


class PscCspMonitorApp(tk.Tk):
    def __init__(self) -> None:
        super().__init__()
        self.title("PSC CSP/RS485 Monitor")
        self.geometry("1180x760")
        self.minsize(980, 560)

        self.rx_queue: queue.Queue[tuple[str, object]] = queue.Queue()
        self.reader: SerialReader | None = None
        self.packet_count = 0
        self.ignored_count = 0
        self.serial_error_count = 0
        self.tc_timeout_count = 0
        self.last_packet_time = 0.0
        self.pending_command: PendingCommand | None = None
        self.serial_ready = False
        self.next_transaction_id = 1
        self.next_source_port = CSP_SOURCE_PORT_FIRST

        self.port_var = tk.StringVar()
        self.baud_var = tk.StringVar(value=str(DEFAULT_BAUDRATE))
        self.status_var = tk.StringVar(value="Disconnected")
        self.mode_var = tk.StringVar(value="normal_mode")
        self.burn_time_var = tk.StringVar(value="1000")
        self.sv_o3_open_delay_var = tk.StringVar(value="1000")
        self.sv_f3_open_delay_var = tk.StringVar(value="2000")
        self.spark_start_delay_var = tk.StringVar(value="500")
        self.spark_duration_var = tk.StringVar(value="3000")
        self.sv_o3_close_delay_var = tk.StringVar(value="500")
        self.sv_f3_close_delay_var = tk.StringVar(value="2000")
        self.par_oxidizer_pt_var = tk.StringVar(value="PT-O3")
        self.par_fuel_pt_var = tk.StringVar(value="PT-F3")
        self.tm_mode_var = tk.StringVar(value="-")
        self.tick_var = tk.StringVar(value="-")
        self.packet_count_var = tk.StringVar(value="0")
        self.ignored_count_var = tk.StringVar(value="0")
        self.serial_error_count_var = tk.StringVar(value="0")
        self.tc_timeout_count_var = tk.StringVar(value="0")
        self.last_update_var = tk.StringVar(value="-")
        self.age_var = tk.StringVar(value="-")
        self.health_var = tk.StringVar(value="-")
        self.sensor_source_var = tk.StringVar(value="REAL")
        self.auto_health_poll_var = tk.IntVar(value=1)

        self.current_mode_value: int | None = None
        self.sensor_override_active = False
        self.pt_vars = [tk.StringVar(value="-") for _ in range(PT_COUNT)]
        self.pt_pressure_vars = [tk.StringVar(value="-") for _ in range(PT_COUNT)]
        self.pt_display_vars = [tk.StringVar(value="- mV\n- bar") for _ in range(PT_COUNT)]
        self.tc_vars = [tk.StringVar(value="-") for _ in range(TC_COUNT)]
        self.tc_temp_vars = [tk.StringVar(value="-") for _ in range(TC_COUNT)]
        self.tc_display_vars = [tk.StringVar(value="- uV\n- K (- C)") for _ in range(TC_COUNT)]
        self.lpv_tm_vars = [tk.StringVar(value="-") for _ in range(LPV_COUNT)]
        self.hpv_tm_vars = [tk.StringVar(value="-") for _ in range(HPV_COUNT)]
        self.htr_tm_vars = [tk.StringVar(value="-") for _ in range(HTR_COUNT)]
        self.sp_tm_var = tk.StringVar(value="-")
        self.lpv_cmd_vars = [tk.IntVar(value=0) for _ in range(LPV_COUNT)]
        self.hpv_cmd_vars = [tk.IntVar(value=0) for _ in range(HPV_COUNT)]
        self.htr_cmd_vars = [tk.IntVar(value=0) for _ in range(HTR_COUNT)]
        self.sp_cmd_var = tk.IntVar(value=0)
        self.thruster_param_entries: list[ttk.Entry] = []
        self.par_sensor_combos: list[ttk.Combobox] = []
        self.sim_pt_bar_vars = [tk.StringVar(value="0.000") for _ in range(PT_COUNT)]
        self.sim_tc_kelvin_vars = [tk.StringVar(value="293.150") for _ in range(TC_COUNT)]
        self.sim_sensor_entries: list[ttk.Entry] = []

        self._build_ui()
        self.refresh_ports()
        self.after(20, self._poll_queue)
        self.after(500, self._update_packet_age)
        self.after(AUTO_HEALTH_POLL_MS, self._poll_auto_health)
        self.protocol("WM_DELETE_WINDOW", self._on_close)

    def _build_ui(self) -> None:
        self.columnconfigure(0, weight=1)
        self.rowconfigure(0, weight=1)

        self._scroll_canvas = tk.Canvas(self, highlightthickness=0, borderwidth=0)
        scroll_y = ttk.Scrollbar(self, orient="vertical", command=self._scroll_canvas.yview)
        self._scroll_canvas.configure(yscrollcommand=scroll_y.set)
        self._scroll_canvas.grid(row=0, column=0, sticky="nsew")
        scroll_y.grid(row=0, column=1, sticky="ns")

        content = ttk.Frame(self._scroll_canvas)
        content.columnconfigure(0, weight=1)
        content.rowconfigure(7, weight=1)
        self._scroll_window = self._scroll_canvas.create_window((0, 0), window=content, anchor="nw")
        content.bind("<Configure>", self._on_scroll_content_configure)
        self._scroll_canvas.bind("<Configure>", self._on_scroll_canvas_configure)
        self.bind_all("<MouseWheel>", self._on_mousewheel)

        connection = ttk.LabelFrame(content, text="Connection")
        connection.grid(row=0, column=0, padx=12, pady=(12, 6), sticky="ew")
        connection.columnconfigure(1, weight=1)

        ttk.Label(connection, text="Port").grid(row=0, column=0, padx=(10, 6), pady=10, sticky="w")
        self.port_combo = ttk.Combobox(connection, textvariable=self.port_var, width=20, state="readonly")
        self.port_combo.grid(row=0, column=1, padx=6, pady=10, sticky="w")
        ttk.Button(connection, text="Refresh", command=self.refresh_ports).grid(row=0, column=2, padx=6, pady=10)

        ttk.Label(connection, text="Baud").grid(row=0, column=3, padx=(18, 6), pady=10, sticky="w")
        self.baud_combo = ttk.Combobox(
            connection,
            textvariable=self.baud_var,
            width=12,
            values=("9600", "19200", "38400", "57600", "115200", "230400", "460800", "921600"),
        )
        self.baud_combo.grid(row=0, column=4, padx=6, pady=10, sticky="w")

        self.connect_button = ttk.Button(connection, text="Connect", command=self.connect_serial)
        self.connect_button.grid(row=0, column=5, padx=(18, 6), pady=10)
        self.disconnect_button = ttk.Button(connection, text="Disconnect", command=self.disconnect_serial, state="disabled")
        self.disconnect_button.grid(row=0, column=6, padx=(6, 10), pady=10)

        status = ttk.LabelFrame(content, text="Status")
        status.grid(row=1, column=0, padx=12, pady=6, sticky="ew")
        for col in range(11):
            status.columnconfigure(col, weight=1)
        self._add_status_item(status, 0, "Status", self.status_var)
        self._add_status_item(status, 1, "Tick", self.tick_var)
        self._add_status_item(status, 2, "Mode", self.tm_mode_var)
        self._add_status_item(status, 3, "Packets", self.packet_count_var)
        self._add_status_item(status, 4, "Ignored", self.ignored_count_var)
        self._add_status_item(status, 5, "Serial Err", self.serial_error_count_var)
        self._add_status_item(status, 6, "Timeouts", self.tc_timeout_count_var)
        self._add_status_item(status, 7, "Health", self.health_var)
        self._add_status_item(status, 8, "Last Update", self.last_update_var)
        self._add_status_item(status, 9, "Age", self.age_var)
        self._add_status_item(status, 10, "Sensor Src", self.sensor_source_var)

        status_request_frame = ttk.Frame(status)
        status_request_frame.grid(row=1, column=0, columnspan=11, padx=6, pady=(2, 6), sticky="ew")
        status_request_frame.columnconfigure(0, weight=1)
        self.request_health_button = ttk.Button(status_request_frame, text="Health", command=self.send_health_req, state="disabled")
        self.request_health_button.grid(row=0, column=0, padx=(0, 6), sticky="ew")
        self.auto_health_check = ttk.Checkbutton(status_request_frame, text="Auto 1Hz", variable=self.auto_health_poll_var)
        self.auto_health_check.grid(row=0, column=1, padx=(6, 0), sticky="w")

        values = ttk.Frame(content)
        values.grid(row=2, column=0, padx=12, pady=6, sticky="ew")
        values.columnconfigure(0, weight=1)

        pt_frame = ttk.LabelFrame(values, text="Pressure Transducers")
        pt_frame.grid(row=0, column=0, pady=(0, 6), sticky="ew")
        for col in range(PT_COUNT):
            pt_frame.columnconfigure(col, weight=1)
        for idx, value_var in enumerate(self.pt_display_vars):
            self._add_combined_value_cell(pt_frame, 0, idx, PT_LABELS[idx], value_var)

        tc_frame = ttk.LabelFrame(values, text="TC / CJC")
        tc_frame.grid(row=1, column=0, sticky="ew")
        for col in range(TC_COUNT):
            tc_frame.columnconfigure(col, weight=1)
        for idx, value_var in enumerate(self.tc_display_vars):
            self._add_combined_value_cell(tc_frame, 0, idx, TC_LABELS[idx], value_var)

        tm_request_frame = ttk.LabelFrame(values, text="Telemetry Requests")
        tm_request_frame.grid(row=2, column=0, pady=(6, 0), sticky="ew")
        tm_request_frame.columnconfigure(0, weight=1)
        self.request_sensor_tm_button = ttk.Button(
            tm_request_frame,
            text="Sensor Raw / Converted",
            command=self.send_sensor_tmreq,
            state="disabled",
        )
        self.request_sensor_tm_button.grid(row=0, column=0, padx=6, pady=6, sticky="ew")

        telemetry_actuator_frame = ttk.LabelFrame(content, text="Telemetry - Actuator States")
        telemetry_actuator_frame.grid(row=3, column=0, padx=12, pady=6, sticky="ew")
        telemetry_actuator_frame.columnconfigure(0, weight=3)
        telemetry_actuator_frame.columnconfigure(1, weight=2)
        telemetry_actuator_frame.columnconfigure(2, weight=2)

        lpv_tm_frame = ttk.LabelFrame(telemetry_actuator_frame, text="LPV")
        lpv_tm_frame.grid(row=0, column=0, padx=(10, 8), pady=8, sticky="ew")
        for col in range(6):
            lpv_tm_frame.columnconfigure(col, weight=1)
        for idx, variable in enumerate(self.lpv_tm_vars):
            self._add_state_cell(lpv_tm_frame, idx // 6, idx % 6, LPV_LABELS[idx], variable)

        hpv_tm_frame = ttk.LabelFrame(telemetry_actuator_frame, text="HPV")
        hpv_tm_frame.grid(row=0, column=1, padx=(8, 10), pady=8, sticky="ew")
        for col in range(4):
            hpv_tm_frame.columnconfigure(col, weight=1)
        for idx, variable in enumerate(self.hpv_tm_vars):
            self._add_state_cell(hpv_tm_frame, idx // 4, idx % 4, HPV_LABELS[idx], variable)

        htr_tm_frame = ttk.LabelFrame(telemetry_actuator_frame, text="Heater / SP")
        htr_tm_frame.grid(row=0, column=2, padx=(8, 10), pady=8, sticky="ew")
        for col in range(5):
            htr_tm_frame.columnconfigure(col, weight=1)
        for idx, variable in enumerate(self.htr_tm_vars):
            self._add_state_cell(htr_tm_frame, 0, idx, f"HTR{idx + 1}", variable)
        self._add_state_cell(htr_tm_frame, 0, HTR_COUNT, "SP", self.sp_tm_var)

        command_frame = ttk.LabelFrame(content, text="Telecommand - Set Outputs")
        command_frame.grid(row=4, column=0, padx=12, pady=6, sticky="ew")
        command_frame.columnconfigure(0, weight=3)
        command_frame.columnconfigure(1, weight=2)
        command_frame.columnconfigure(2, weight=2)

        lpv_frame = ttk.LabelFrame(command_frame, text="LPV")
        lpv_frame.grid(row=0, column=0, padx=(10, 8), pady=8, sticky="ew")
        for col in range(6):
            lpv_frame.columnconfigure(col, weight=1)
        for idx, variable in enumerate(self.lpv_cmd_vars):
            self._add_valve_check(lpv_frame, idx // 6, idx % 6, LPV_LABELS[idx], variable)

        hpv_frame = ttk.LabelFrame(command_frame, text="HPV (DIAG only)")
        hpv_frame.grid(row=0, column=1, padx=(8, 10), pady=8, sticky="ew")
        for col in range(4):
            hpv_frame.columnconfigure(col, weight=1)
        for idx, variable in enumerate(self.hpv_cmd_vars):
            self._add_valve_check(hpv_frame, idx // 4, idx % 4, HPV_LABELS[idx], variable)

        htr_frame = ttk.LabelFrame(command_frame, text="Heater / SP")
        htr_frame.grid(row=0, column=2, padx=(8, 10), pady=8, sticky="ew")
        for col in range(5):
            htr_frame.columnconfigure(col, weight=1)
        for idx, variable in enumerate(self.htr_cmd_vars):
            self._add_valve_check(htr_frame, 0, idx, f"HTR{idx + 1}", variable)
        self._add_valve_check(htr_frame, 0, HTR_COUNT, "SP", self.sp_cmd_var)

        output_frame = ttk.Frame(command_frame)
        output_frame.grid(row=1, column=0, columnspan=3, padx=10, pady=(0, 8), sticky="ew")
        output_frame.columnconfigure(0, weight=1)
        output_frame.columnconfigure(1, weight=1)
        output_frame.columnconfigure(2, weight=1)
        self.request_sv_tm_button = ttk.Button(output_frame, text="SV State", command=self.send_sv_tmreq, state="disabled")
        self.request_sv_tm_button.grid(row=0, column=0, padx=(0, 4), sticky="ew")
        self.send_command_button = ttk.Button(output_frame, text="Set Outputs", command=self.send_set_outputs, state="disabled")
        self.send_command_button.grid(row=0, column=1, padx=4, sticky="ew")
        self.all_off_button = ttk.Button(output_frame, text="All Off", command=self.send_all_off, state="disabled")
        self.all_off_button.grid(row=0, column=2, padx=(4, 0), sticky="ew")

        control_frame = ttk.LabelFrame(content, text="CSP Requests")
        control_frame.grid(row=5, column=0, padx=12, pady=6, sticky="ew")
        for col in range(2):
            control_frame.columnconfigure(col, weight=1)

        mode_frame = ttk.LabelFrame(control_frame, text="Mode")
        mode_frame.grid(row=0, column=0, padx=(10, 6), pady=10, sticky="ew")
        ttk.Label(mode_frame, text="Mode").grid(row=0, column=0, padx=(0, 6))
        self.mode_combo = ttk.Combobox(
            mode_frame,
            textvariable=self.mode_var,
            values=MODE_OPTIONS,
            width=18,
            state="readonly",
        )
        self.mode_combo.grid(row=0, column=1, padx=(0, 6))
        self.send_mode_button = ttk.Button(mode_frame, text="Set Mode", command=self.send_mode, state="disabled")
        self.send_mode_button.grid(row=0, column=2)

        service_frame = ttk.LabelFrame(control_frame, text="CSP Services")
        service_frame.grid(row=0, column=1, padx=(6, 10), pady=10, sticky="ew")
        self.ping_button = ttk.Button(service_frame, text="Ping", command=self.send_csp_ping, state="disabled")
        self.ping_button.grid(row=0, column=0, padx=(6, 3), pady=6, sticky="ew")
        self.reboot_button = ttk.Button(service_frame, text="Reset", command=self.send_csp_reboot, state="disabled")
        self.reboot_button.grid(row=0, column=1, padx=(3, 6), pady=6, sticky="ew")

        sequence_frame = ttk.LabelFrame(control_frame, text="Sequences")
        sequence_frame.grid(row=1, column=0, columnspan=2, padx=10, pady=(0, 10), sticky="ew")
        for col in (1, 3, 5, 7):
            sequence_frame.columnconfigure(col, weight=1)
        self._add_thruster_param_entry(sequence_frame, 0, 0, "Burn ms", self.burn_time_var)
        self._add_thruster_param_entry(sequence_frame, 0, 2, "SV-O3 open ms", self.sv_o3_open_delay_var)
        self._add_thruster_param_entry(sequence_frame, 0, 4, "SV-F3 open ms", self.sv_f3_open_delay_var)
        self._add_thruster_param_entry(sequence_frame, 0, 6, "SP start ms", self.spark_start_delay_var)
        self._add_thruster_param_entry(sequence_frame, 1, 0, "SP duration ms", self.spark_duration_var)
        self._add_thruster_param_entry(sequence_frame, 1, 2, "SV-O3 close ms", self.sv_o3_close_delay_var)
        self._add_thruster_param_entry(sequence_frame, 1, 4, "SV-F3 close ms", self.sv_f3_close_delay_var)
        self.thruster_start_button = ttk.Button(sequence_frame, text="Thruster Start", command=self.send_thruster_start, state="disabled")
        self.thruster_start_button.grid(row=2, column=0, columnspan=2, padx=(0, 6), pady=(6, 0), sticky="ew")
        self.par_start_button = ttk.Button(sequence_frame, text="PAR Start", command=self.send_par_start, state="disabled")
        self.par_start_button.grid(row=2, column=2, columnspan=2, padx=(0, 6), pady=(6, 0), sticky="ew")
        self.par_stop_button = ttk.Button(sequence_frame, text="PAR Stop", command=self.send_par_stop, state="disabled")
        self.par_stop_button.grid(row=2, column=4, columnspan=2, padx=(0, 6), pady=(6, 0), sticky="ew")
        self._add_par_sensor_combo(
            sequence_frame,
            3,
            0,
            "Ox PT",
            self.par_oxidizer_pt_var,
            PAR_OXIDIZER_PT_OPTIONS,
        )
        self._add_par_sensor_combo(
            sequence_frame,
            3,
            4,
            "Fuel PT",
            self.par_fuel_pt_var,
            PAR_FUEL_PT_OPTIONS,
        )

        sim_frame = ttk.LabelFrame(control_frame, text="Simulation Sensors")
        sim_frame.grid(row=2, column=0, columnspan=2, padx=10, pady=(0, 10), sticky="ew")
        sim_frame.columnconfigure(0, weight=3)
        sim_frame.columnconfigure(1, weight=2)

        sim_pt_frame = ttk.LabelFrame(sim_frame, text="Pressure Input (bar)")
        sim_pt_frame.grid(row=0, column=0, padx=(6, 4), pady=6, sticky="ew")
        for col in range(6):
            sim_pt_frame.columnconfigure(col, weight=1)
        for idx, variable in enumerate(self.sim_pt_bar_vars):
            self._add_sim_param_entry(sim_pt_frame, idx // 3, (idx % 3) * 2, PT_LABELS[idx], variable, 8)

        sim_tc_frame = ttk.LabelFrame(sim_frame, text="Temperature Input (K)")
        sim_tc_frame.grid(row=0, column=1, padx=(4, 6), pady=6, sticky="ew")
        for col in range(4):
            sim_tc_frame.columnconfigure(col, weight=1)
        for idx, variable in enumerate(self.sim_tc_kelvin_vars):
            self._add_sim_param_entry(sim_tc_frame, idx // 2, (idx % 2) * 2, TC_LABELS[idx], variable, 8)

        sim_control_frame = ttk.Frame(sim_frame)
        sim_control_frame.grid(row=1, column=0, columnspan=2, padx=6, pady=(0, 6), sticky="ew")
        for col in range(3):
            sim_control_frame.columnconfigure(col, weight=1)
        self.sim_start_button = ttk.Button(
            sim_control_frame,
            text="SIM Start",
            command=self.send_sim_start,
            state="disabled",
        )
        self.sim_start_button.grid(row=0, column=0, padx=(0, 4), sticky="ew")
        self.send_sim_sensor_button = ttk.Button(
            sim_control_frame,
            text="Set SIM Sensors",
            command=self.send_sim_sensor_values,
            state="disabled",
        )
        self.send_sim_sensor_button.grid(row=0, column=1, padx=4, sticky="ew")
        self.sim_stop_button = ttk.Button(
            sim_control_frame,
            text="SIM Stop",
            command=self.send_sim_stop,
            state="disabled",
        )
        self.sim_stop_button.grid(row=0, column=2, padx=(4, 0), sticky="ew")

        log_frame = ttk.LabelFrame(content, text="Log")
        log_frame.grid(row=7, column=0, padx=12, pady=(6, 12), sticky="nsew")
        log_frame.columnconfigure(0, weight=1)
        log_frame.rowconfigure(0, weight=1)

        self.log_text = tk.Text(log_frame, height=8, wrap="none", state="disabled")
        self.log_text.grid(row=0, column=0, sticky="nsew")

        log_scroll_y = ttk.Scrollbar(log_frame, orient="vertical", command=self.log_text.yview)
        log_scroll_y.grid(row=0, column=1, sticky="ns")
        self.log_text.configure(yscrollcommand=log_scroll_y.set)

        log_buttons = ttk.Frame(log_frame)
        log_buttons.grid(row=1, column=0, columnspan=2, sticky="e", pady=(6, 0))
        ttk.Button(log_buttons, text="Clear Log", command=self.clear_log).grid(row=0, column=0, padx=(0, 6))

    def _on_scroll_content_configure(self, _event: tk.Event) -> None:
        self._scroll_canvas.configure(scrollregion=self._scroll_canvas.bbox("all"))

    def _on_scroll_canvas_configure(self, event: tk.Event) -> None:
        self._scroll_canvas.itemconfigure(self._scroll_window, width=event.width)

    def _on_mousewheel(self, event: tk.Event) -> None:
        if event.delta:
            self._scroll_canvas.yview_scroll(int(-1 * (event.delta / 120)), "units")

    def _add_status_item(self, parent: ttk.Frame, column: int, label: str, variable: tk.StringVar) -> None:
        frame = ttk.Frame(parent)
        frame.grid(row=0, column=column, padx=4, sticky="ew")
        ttk.Label(frame, text=label).grid(row=0, column=0, sticky="w")
        ttk.Label(frame, textvariable=variable, font=("Segoe UI", 11, "bold")).grid(row=1, column=0, sticky="w")

    def _add_value_cell(
        self,
        parent: ttk.LabelFrame,
        row: int,
        column: int,
        label: str,
        variable: tk.StringVar,
        unit: str,
    ) -> None:
        frame = ttk.Frame(parent, padding=(6, 6))
        frame.grid(row=row, column=column, padx=6, pady=6, sticky="ew")
        frame.columnconfigure(0, weight=1)
        ttk.Label(frame, text=label).grid(row=0, column=0, sticky="w")
        value_row = ttk.Frame(frame)
        value_row.grid(row=1, column=0, sticky="ew")
        ttk.Label(value_row, textvariable=variable, font=("Consolas", 14, "bold")).grid(row=0, column=0, sticky="w")
        ttk.Label(value_row, text=unit).grid(row=0, column=1, padx=(4, 0), sticky="s")

    def _add_combined_value_cell(
        self,
        parent: ttk.LabelFrame,
        row: int,
        column: int,
        label: str,
        variable: tk.StringVar,
    ) -> None:
        frame = ttk.Frame(parent, padding=(6, 5))
        frame.grid(row=row, column=column, padx=6, pady=5, sticky="ew")
        frame.columnconfigure(0, weight=1)
        ttk.Label(frame, text=label).grid(row=0, column=0, sticky="w")
        ttk.Label(
            frame,
            textvariable=variable,
            font=("Consolas", 12, "bold"),
            justify="left",
        ).grid(row=1, column=0, sticky="w")

    def _format_measurement_value(self, value: str, unit: str) -> str:
        return f"- {unit}" if value == "-" else f"{value} {unit}"

    def _format_temperature_value(self, value: str) -> str:
        if value == "-":
            return "- K (- C)"
        try:
            kelvin = float(value)
        except ValueError:
            return f"{value} K"
        return f"{kelvin:.3f} K ({kelvin - 273.15:.1f}C)"

    def _refresh_pt_display(self, idx: int) -> None:
        if idx < 0 or idx >= PT_COUNT:
            return
        self.pt_display_vars[idx].set(
            f"{self._format_measurement_value(self.pt_vars[idx].get(), 'mV')}\n"
            f"{self._format_measurement_value(self.pt_pressure_vars[idx].get(), 'bar')}"
        )

    def _refresh_tc_display(self, idx: int) -> None:
        if idx < 0 or idx >= TC_COUNT:
            return
        self.tc_display_vars[idx].set(
            f"{self._format_measurement_value(self.tc_vars[idx].get(), 'uV')}\n"
            f"{self._format_temperature_value(self.tc_temp_vars[idx].get())}"
        )

    def _add_state_cell(self, parent: ttk.Frame, row: int, column: int, label: str, variable: tk.StringVar) -> None:
        frame = ttk.Frame(parent, padding=(4, 3))
        frame.grid(row=row, column=column, padx=4, pady=3, sticky="ew")
        frame.columnconfigure(0, weight=1)
        ttk.Label(frame, text=label).grid(row=0, column=0, sticky="w")
        ttk.Label(frame, textvariable=variable, font=("Consolas", 12, "bold")).grid(row=1, column=0, sticky="w")

    def _add_valve_check(self, parent: ttk.Frame, row: int, column: int, label: str, variable: tk.IntVar) -> None:
        check = ttk.Checkbutton(parent, text=label, variable=variable)
        check.grid(row=row, column=column, padx=4, pady=3, sticky="w")

    def _add_thruster_param_entry(
        self,
        parent: ttk.Frame,
        row: int,
        column: int,
        label: str,
        variable: tk.StringVar,
    ) -> None:
        ttk.Label(parent, text=label).grid(row=row, column=column, padx=(0, 4), pady=3, sticky="w")
        entry = ttk.Entry(parent, textvariable=variable, width=8)
        entry.grid(row=row, column=column + 1, padx=(0, 10), pady=3, sticky="ew")
        self.thruster_param_entries.append(entry)

    def _add_par_sensor_combo(
        self,
        parent: ttk.Frame,
        row: int,
        column: int,
        label: str,
        variable: tk.StringVar,
        values: tuple[str, ...],
    ) -> None:
        ttk.Label(parent, text=label).grid(row=row, column=column, padx=(0, 4), pady=3, sticky="w")
        combo = ttk.Combobox(parent, textvariable=variable, values=values, width=8, state="readonly")
        combo.grid(row=row, column=column + 1, padx=(0, 10), pady=3, sticky="ew")
        self.par_sensor_combos.append(combo)

    def _add_sim_param_entry(
        self,
        parent: ttk.Frame,
        row: int,
        column: int,
        label: str,
        variable: tk.StringVar,
        width: int,
    ) -> None:
        ttk.Label(parent, text=label).grid(row=row, column=column, padx=(0, 4), pady=3, sticky="w")
        entry = ttk.Entry(parent, textvariable=variable, width=width)
        entry.grid(row=row, column=column + 1, padx=(0, 8), pady=3, sticky="ew")
        self.sim_sensor_entries.append(entry)

    def refresh_ports(self) -> None:
        if list_ports is None:
            self._set_status("pyserial is not installed")
            return

        ports = list(list_ports.comports())
        values = [port.device for port in ports]
        self.port_combo.configure(values=values)
        if values and not self.port_var.get():
            self.port_var.set(values[0])
        elif self.port_var.get() not in values:
            self.port_var.set(values[0] if values else "")
        self._log(f"Ports: {', '.join(values) if values else 'none'}")

    def connect_serial(self) -> None:
        if serial is None:
            messagebox.showerror("Missing dependency", "pyserial is required. Run: python -m pip install pyserial")
            return

        port = self.port_var.get().strip()
        if not port:
            messagebox.showwarning("No port", "Select a serial port first.")
            return

        try:
            baudrate = int(self.baud_var.get().strip())
        except ValueError:
            messagebox.showwarning("Invalid baud", "Baud rate must be a number.")
            return

        self.disconnect_serial(wait=False)
        self.reader = SerialReader(port, baudrate, self.rx_queue)
        self.reader.start()
        self.serial_ready = False
        self.packet_count = 0
        self.ignored_count = 0
        self.serial_error_count = 0
        self.tc_timeout_count = 0
        self.last_packet_time = 0.0
        self.pending_command = None
        self.current_mode_value = None
        self.sensor_override_active = False
        self.next_transaction_id = 1
        self.next_source_port = CSP_SOURCE_PORT_FIRST
        self.packet_count_var.set("0")
        self.ignored_count_var.set("0")
        self.serial_error_count_var.set("0")
        self.tc_timeout_count_var.set("0")
        self.tick_var.set("-")
        self.tm_mode_var.set("-")
        self.health_var.set("-")
        self.sensor_source_var.set("REAL")
        self.last_update_var.set("-")
        self.age_var.set("-")
        for variable in (
            self.pt_vars + self.pt_pressure_vars + self.tc_vars + self.tc_temp_vars +
            self.lpv_tm_vars + self.hpv_tm_vars + self.htr_tm_vars
        ):
            variable.set("-")
        for idx in range(PT_COUNT):
            self._refresh_pt_display(idx)
        for idx in range(TC_COUNT):
            self._refresh_tc_display(idx)
        self.sp_tm_var.set("-")
        self._set_connected_ui(False)
        self.connect_button.configure(state="disabled")
        self.disconnect_button.configure(state="normal")
        self.port_combo.configure(state="disabled")
        self.baud_combo.configure(state="disabled")
        self._set_status("Connecting")

    def disconnect_serial(self, wait: bool = True) -> None:
        self.pending_command = None
        self.serial_ready = False
        if self.reader is not None:
            self.reader.stop()
            if wait:
                self.reader.join(timeout=1.0)
            self.reader = None

        self._set_connected_ui(False)
        if self.status_var.get() != "Disconnected":
            self._set_status("Disconnected")

    def clear_log(self) -> None:
        self.log_text.configure(state="normal")
        self.log_text.delete("1.0", "end")
        self.log_text.configure(state="disabled")

    def _allocate_transaction_id(self) -> int:
        transaction_id = self.next_transaction_id
        self.next_transaction_id = (self.next_transaction_id + 1) & 0xFFFF
        if self.next_transaction_id == 0:
            self.next_transaction_id = 1
        return transaction_id

    def _allocate_source_port(self) -> int:
        source_port = self.next_source_port
        self.next_source_port += 1
        if self.next_source_port > CSP_SOURCE_PORT_LAST:
            self.next_source_port = CSP_SOURCE_PORT_FIRST
        return source_port

    def _has_pending_command(self) -> bool:
        return self.pending_command is not None

    def _start_request(
        self,
        label: str,
        port: int,
        opcode: int,
        extra_payload: bytes,
        expected_kind: str,
        quiet: bool = False,
    ) -> bool:
        if self.reader is None or not self.serial_ready:
            if not quiet:
                messagebox.showwarning("Not connected", "Connect to a serial port first.")
            return False
        if self._has_pending_command():
            if not quiet:
                messagebox.showwarning("Command pending", "Wait for the current response or timeout.")
            return False

        transaction_id = self._allocate_transaction_id()
        source_port = self._allocate_source_port()
        payload = struct.pack(">BBH", CSP_PROTOCOL_VERSION, opcode, transaction_id) + extra_payload
        frame = build_csp_frame(port, source_port, payload)
        self.pending_command = PendingCommand(
            label=label,
            port=port,
            opcode=opcode,
            transaction_id=transaction_id,
            source_port=source_port,
            frame=frame,
            expected_kind=expected_kind,
            expected_payload=payload,
        )
        self._send_pending_command_attempt()
        return True

    def _start_standard_request(
        self,
        label: str,
        port: int,
        payload: bytes,
        expected_kind: str,
    ) -> bool:
        if self.reader is None or not self.serial_ready:
            messagebox.showwarning("Not connected", "Connect to a serial port first.")
            return False
        if self._has_pending_command():
            messagebox.showwarning("Command pending", "Wait for the current response or timeout.")
            return False

        source_port = self._allocate_source_port()
        frame = build_csp_frame(port, source_port, payload)
        self.pending_command = PendingCommand(
            label=label,
            port=port,
            opcode=0,
            transaction_id=0,
            source_port=source_port,
            frame=frame,
            expected_kind=expected_kind,
            expected_payload=payload,
        )
        self._send_pending_command_attempt()
        return True

    def _send_standard_no_response(self, label: str, port: int, payload: bytes) -> bool:
        if self.reader is None or not self.serial_ready:
            messagebox.showwarning("Not connected", "Connect to a serial port first.")
            return False
        if self._has_pending_command():
            messagebox.showwarning("Command pending", "Wait for the current response or timeout.")
            return False

        source_port = self._allocate_source_port()
        frame = build_csp_frame(port, source_port, payload)
        self.reader.send_frame(label, frame)
        self._set_status(f"{label} sent")
        return True

    def _send_pending_command_attempt(self) -> None:
        if self.reader is None or self.pending_command is None:
            return

        total_attempts = COMMAND_MAX_RETRIES + 1
        attempt = self.pending_command.retries_done + 1
        self.pending_command.deadline = 0.0
        self.pending_command.waiting_for_tx = True
        self._set_connected_ui(True)
        if self.pending_command.label != AUTO_HEALTH_LABEL:
            self._set_status(f"Sending {self.pending_command.label} ({attempt}/{total_attempts})")
        self.reader.send_frame(self.pending_command.label, self.pending_command.frame)

    def _complete_pending_command(self, response: str) -> None:
        if self.pending_command is None:
            return
        label = self.pending_command.label
        self.pending_command = None
        self._set_connected_ui(self.reader is not None)
        if label != AUTO_HEALTH_LABEL:
            self._set_status(f"{label} {response}")
            self._log(f"{label} {response}")

    def send_set_outputs(self) -> None:
        lpv_mask = sum((var.get() & 1) << idx for idx, var in enumerate(self.lpv_cmd_vars))
        hpv_mask = sum((var.get() & 1) << idx for idx, var in enumerate(self.hpv_cmd_vars))
        heater_mask = sum((var.get() & 1) << idx for idx, var in enumerate(self.htr_cmd_vars))
        spark_on = self.sp_cmd_var.get() & 1
        extra = struct.pack(">HBBBB", lpv_mask, hpv_mask, heater_mask, spark_on, 0)
        self._start_request("SET_OUTPUTS", CSP_PORT_COMMAND, OP_SET_OUTPUTS, extra, "sensor")

    def send_set_lpv_outputs(self) -> None:
        lpv_mask = sum((var.get() & 1) << idx for idx, var in enumerate(self.lpv_cmd_vars))
        extra = struct.pack(">H", lpv_mask)
        self._start_request("SET_LPV_OUTPUTS", CSP_PORT_COMMAND, OP_SET_LPV_OUTPUTS, extra, "sensor")

    def send_sensor_tmreq(self) -> None:
        self._start_request("GET_SENSOR_SNAPSHOT", CSP_PORT_TELEMETRY, OP_GET_SENSOR_SNAPSHOT, b"", "sensor")

    def send_sv_tmreq(self) -> None:
        self._start_request("GET_SOLVALVE_STATE", CSP_PORT_TELEMETRY, OP_GET_SOLVALVE_STATE, b"", "solvalve")

    def send_health_req(self) -> None:
        self._start_request("GET_HEALTH", CSP_PORT_DIAGNOSTICS, OP_GET_HEALTH, b"", "health")

    def send_csp_ping(self) -> None:
        self._start_standard_request("CSP_PING", CSP_PORT_PING, CSP_PING_PAYLOAD, "ping")

    def send_csp_reboot(self) -> None:
        confirmed = messagebox.askyesno(
            "Reset PSC",
            "Send CSP_REBOOT to PS address 0x10?",
            icon="warning",
        )
        if not confirmed:
            return
        payload = struct.pack(">I", CSP_REBOOT_MAGIC)
        self._send_standard_no_response("CSP_REBOOT", CSP_PORT_REBOOT, payload)

    def _poll_auto_health(self) -> None:
        if (
            self.auto_health_poll_var.get() != 0
            and self.reader is not None
            and self.serial_ready
            and not self._has_pending_command()
        ):
            self._start_request(
                AUTO_HEALTH_LABEL,
                CSP_PORT_DIAGNOSTICS,
                OP_GET_HEALTH,
                b"",
                "health",
                quiet=True,
            )
        self.after(AUTO_HEALTH_POLL_MS, self._poll_auto_health)

    def send_mode(self) -> None:
        mode = self.mode_var.get().strip()
        if mode not in MODE_TO_VALUE:
            mode = "normal_mode"
            self.mode_var.set(mode)
        self._start_request("SET_MODE", CSP_PORT_COMMAND, OP_SET_MODE, bytes((MODE_TO_VALUE[mode],)), "status")

    def _read_uint32_ms_param(
        self,
        label: str,
        variable: tk.StringVar,
        allow_zero: bool,
    ) -> int | None:
        text = variable.get().strip()
        try:
            value = int(text, 10)
        except ValueError:
            messagebox.showerror("Invalid thruster parameter", f"{label} must be an integer in ms.")
            return None
        if value < 0 or value > 0xFFFFFFFF:
            messagebox.showerror("Invalid thruster parameter", f"{label} must be 0..4294967295 ms.")
            return None
        if not allow_zero and value == 0:
            messagebox.showerror("Invalid thruster parameter", f"{label} must be greater than 0 ms.")
            return None
        variable.set(str(value))
        return value

    def send_thruster_start(self) -> None:
        param_specs = (
            ("Burn ms", self.burn_time_var, False),
            ("SV-O3 open ms", self.sv_o3_open_delay_var, True),
            ("SV-F3 open ms", self.sv_f3_open_delay_var, True),
            ("SP start ms", self.spark_start_delay_var, True),
            ("SP duration ms", self.spark_duration_var, True),
            ("SV-O3 close ms", self.sv_o3_close_delay_var, True),
            ("SV-F3 close ms", self.sv_f3_close_delay_var, True),
        )
        values: list[int] = []
        for label, variable, allow_zero in param_specs:
            value = self._read_uint32_ms_param(label, variable, allow_zero)
            if value is None:
                return
            values.append(value)
        self._start_request(
            "THRUSTER_START",
            CSP_PORT_COMMAND,
            OP_THRUSTER_START,
            struct.pack(">7I", *values),
            "status",
        )

    def send_par_start(self) -> None:
        oxidizer_pt = self.par_oxidizer_pt_var.get().strip()
        fuel_pt = self.par_fuel_pt_var.get().strip()
        if oxidizer_pt not in PAR_OXIDIZER_PT_OPTIONS:
            messagebox.showerror("Invalid PAR parameter", "Ox PT must be PT-O3 or PT-O4.")
            return
        if fuel_pt not in PAR_FUEL_PT_OPTIONS:
            messagebox.showerror("Invalid PAR parameter", "Fuel PT must be PT-F3 or PT-F4.")
            return

        selector = 0
        if oxidizer_pt == "PT-O4":
            selector |= 0x01
        if fuel_pt == "PT-F4":
            selector |= 0x02

        self._start_request("PAR_START", CSP_PORT_COMMAND, OP_PAR_START, bytes((selector,)), "status")

    def send_par_stop(self) -> None:
        self._start_request("PAR_STOP", CSP_PORT_COMMAND, OP_PAR_STOP, b"", "status")

    def send_sim_start(self) -> None:
        self._start_request("SIM_START", CSP_PORT_COMMAND, OP_SIM_START, b"", "status")

    def send_sim_stop(self) -> None:
        self._start_request("SIM_STOP", CSP_PORT_COMMAND, OP_SIM_STOP, b"", "status")

    def _read_float_param(self, dialog_title: str, label: str, variable: tk.StringVar) -> float | None:
        text = variable.get().strip()
        try:
            value = float(text)
        except ValueError:
            messagebox.showerror(dialog_title, f"{label} must be a number.")
            return None
        if not math.isfinite(value):
            messagebox.showerror(dialog_title, f"{label} must be finite.")
            return None
        return value

    def send_sim_sensor_values(self) -> None:
        pt_millibar: list[int] = []
        for idx, variable in enumerate(self.sim_pt_bar_vars):
            value = self._read_float_param("Invalid SIM pressure", PT_LABELS[idx], variable)
            if value is None:
                return
            millibar = int(round(value * 1000.0))
            if millibar < -0x80000000 or millibar > 0x7FFFFFFF:
                messagebox.showerror("Invalid SIM pressure", f"{PT_LABELS[idx]} is outside int32 mbar range.")
                return
            variable.set(f"{millibar / 1000.0:.3f}")
            pt_millibar.append(millibar)

        tc_millikelvin: list[int] = []
        for idx, variable in enumerate(self.sim_tc_kelvin_vars):
            value = self._read_float_param("Invalid SIM temperature", TC_LABELS[idx], variable)
            if value is None:
                return
            millikelvin = int(round(value * 1000.0))
            if millikelvin <= 0 or millikelvin > 0x7FFFFFFF:
                messagebox.showerror("Invalid SIM temperature", f"{TC_LABELS[idx]} must be greater than 0 K.")
                return
            variable.set(f"{millikelvin / 1000.0:.3f}")
            tc_millikelvin.append(millikelvin)

        payload = struct.pack(f">{PT_COUNT + TC_COUNT}i", *(pt_millibar + tc_millikelvin))
        self._start_request(
            "SET_SIM_SENSOR_VALUES",
            CSP_PORT_COMMAND,
            OP_SET_SIM_SENSOR_VALUES,
            payload,
            "sensor",
        )

    def send_all_off(self) -> None:
        for variable in self.lpv_cmd_vars + self.hpv_cmd_vars + self.htr_cmd_vars:
            variable.set(0)
        self.sp_cmd_var.set(0)
        self.send_set_outputs()

    def _poll_queue(self) -> None:
        while True:
            try:
                message_type, payload = self.rx_queue.get_nowait()
            except queue.Empty:
                break

            if message_type == "packet":
                self._handle_csp_packet(payload)
            elif message_type == "parse_error":
                self.ignored_count += 1
                self.ignored_count_var.set(str(self.ignored_count))
                self._log(f"Ignored malformed CSP frame: {payload}")
            elif message_type == "serial_error":
                self.serial_error_count += 1
                self.serial_error_count_var.set(str(self.serial_error_count))
                self._log(f"Serial error: {payload}")
                messagebox.showerror("Serial error", str(payload))
                self.disconnect_serial(wait=False)
            elif message_type == "opened":
                self.serial_ready = True
                self._set_status(str(payload))
                self._set_connected_ui(True)
                self._log(str(payload))
            elif message_type == "tx":
                self._handle_tx(payload)
            elif message_type == "closed":
                self._log(str(payload))

        self._check_command_timeout()
        self.after(20, self._poll_queue)

    def _handle_csp_packet(self, packet: CspPacket) -> None:
        self.packet_count += 1
        self.packet_count_var.set(str(self.packet_count))
        self.last_packet_time = time.monotonic()
        self.last_update_var.set(datetime.now().strftime("%H:%M:%S"))

        if self.pending_command is None:
            self._log(f"RX unexpected CSP packet {self._packet_summary(packet)}")
            return

        pending = self.pending_command
        if not self._packet_matches_pending(packet, pending):
            self.ignored_count += 1
            self.ignored_count_var.set(str(self.ignored_count))
            self._log(f"Ignored unmatched CSP packet {self._packet_summary(packet)}")
            return

        try:
            if pending.expected_kind == "sensor":
                status, detail, sensor = parse_sensor_response(
                    packet.payload,
                    pending.transaction_id,
                    pending.opcode,
                )
                if sensor is not None:
                    self._apply_sensor_packet(sensor)
                    self._complete_pending_command("OK")
                else:
                    self._complete_pending_command(status_text(status, detail))
            elif pending.expected_kind == "solvalve":
                status, detail, solvalve = parse_solvalve_response(packet.payload, pending.transaction_id)
                if solvalve is not None:
                    self._apply_solvalve_packet(solvalve)
                    self._complete_pending_command("OK")
                else:
                    self._complete_pending_command(status_text(status, detail))
            elif pending.expected_kind == "health":
                status, detail, health = parse_health_response(packet.payload, pending.transaction_id)
                if health is not None:
                    self._apply_health_packet(health, log_health=(pending.label != AUTO_HEALTH_LABEL))
                    self._complete_pending_command("OK")
                else:
                    self._complete_pending_command(status_text(status, detail))
            elif pending.expected_kind == "ping":
                if packet.payload == pending.expected_payload:
                    self._complete_pending_command("OK")
                else:
                    self._complete_pending_command(
                        f"payload mismatch rx={packet.payload.hex(' ')}"
                    )
            else:
                status, detail = parse_common_status(packet.payload, pending.opcode, pending.transaction_id)
                if pending.label == "SET_MODE" and status == 0:
                    mode = self.mode_var.get().strip()
                    if mode in MODE_TO_VALUE:
                        self.current_mode_value = MODE_TO_VALUE[mode]
                        self.tm_mode_var.set(mode_text(self.current_mode_value))
                elif pending.label == "SIM_START" and status == 0:
                    self.sensor_override_active = True
                    self.sensor_source_var.set("SIM OVERRIDE")
                elif pending.label == "SIM_STOP" and status == 0:
                    self.sensor_override_active = False
                    self.sensor_source_var.set("REAL")
                self._complete_pending_command(status_text(status, detail))
        except ValueError as exc:
            self.ignored_count += 1
            self.ignored_count_var.set(str(self.ignored_count))
            self._log(f"Response parse error: {exc}; {self._packet_summary(packet)}")
            return

        if pending.label != AUTO_HEALTH_LABEL:
            self._log(f"RX {self._packet_summary(packet)} payload={packet.payload.hex(' ')}")

    def _packet_matches_pending(self, packet: CspPacket, pending: PendingCommand) -> bool:
        csp_id = packet.csp_id
        return (
            csp_id.source == PSC_CSP_ADDRESS
            and csp_id.destination == OBC_CSP_ADDRESS
            and csp_id.destination_port == pending.source_port
            and csp_id.source_port == pending.port
        )

    def _packet_summary(self, packet: CspPacket) -> str:
        csp_id = packet.csp_id
        return (
            f"S{csp_id.source}->D{csp_id.destination} "
            f"SP{csp_id.source_port}->DP{csp_id.destination_port} "
            f"len={len(packet.payload)}"
        )

    def _apply_sensor_packet(self, packet: SensorPacket) -> None:
        self.tick_var.set(str(packet.tick_ms))
        self.current_mode_value = packet.current_mode
        self.tm_mode_var.set(mode_text(packet.current_mode))
        for idx, (value_var, value) in enumerate(zip(self.pt_vars, packet.pt_values)):
            value_var.set(str(value))
            self._refresh_pt_display(idx)
        for idx, (value_var, value) in enumerate(zip(self.pt_pressure_vars, packet.pt_pressure_values)):
            value_var.set(f"{value / 1000.0:.3f}")
            self._refresh_pt_display(idx)
        for idx, (value_var, value) in enumerate(zip(self.tc_vars, packet.tc_values)):
            value_var.set(str(value))
            self._refresh_tc_display(idx)
        for idx, (value_var, value) in enumerate(zip(self.tc_temp_vars, packet.tc_temperature_values)):
            if value > 0:
                value_var.set(f"{value / 1000.0:.3f}")
            else:
                value_var.set("-")
            self._refresh_tc_display(idx)
        self._log(
            f"Sensor TM tick={packet.tick_ms} current={mode_text(packet.current_mode)} "
            f"requested={mode_text(packet.requested_mode)} validity=0x{packet.validity_mask:04X}"
        )

    def _apply_solvalve_packet(self, packet: SolvalvePacket) -> None:
        self.tick_var.set(str(packet.tick_ms))
        self.current_mode_value = packet.current_mode
        self.tm_mode_var.set(mode_text(packet.current_mode))
        for idx, variable in enumerate(self.lpv_tm_vars):
            variable.set(str(mask_bit(packet.lpv_mask, idx)))
        for idx, variable in enumerate(self.hpv_tm_vars):
            variable.set(str(mask_bit(packet.hpv_mask, idx)))
        for idx, variable in enumerate(self.htr_tm_vars):
            variable.set(str(mask_bit(packet.heater_mask, idx)))
        self.sp_tm_var.set(str(packet.spark_on & 1))
        self._log(
            f"SV TM tick={packet.tick_ms} mode={mode_text(packet.current_mode)} "
            f"LPV=0x{packet.lpv_mask:03X} HPV=0x{packet.hpv_mask:02X} "
            f"HTR=0x{packet.heater_mask:01X} SP={packet.spark_on & 1}"
        )

    def _apply_health_packet(self, packet: HealthPacket, log_health: bool = True) -> None:
        self.current_mode_value = packet.current_mode
        self.tm_mode_var.set(mode_text(packet.current_mode))
        self.health_var.set(f"L{packet.link_state}/E{packet.last_error}")
        for debug in packet.debug_messages:
            self._log(
                f"DEBUG #{debug.debug_sequence} "
                f"{debug_source_text(debug.debug_source)}."
                f"{debug_event_text(debug.debug_source, debug.debug_event)} "
                f"t={debug.debug_elapsed_ms}ms mode={mode_text(debug.debug_mode)}"
            )
        counter_names = (
            "uart",
            "dma",
            "tx_to",
            "tx_fail",
            "proto",
            "drop",
            "hwm",
            "disc",
            "rec_try",
            "rec_ok",
            "rec_fail",
        )
        counters = " ".join(f"{name}={value}" for name, value in zip(counter_names, packet.counters))
        if log_health:
            self._log(
                f"Health uptime={packet.uptime_ms}ms mode={mode_text(packet.current_mode)} link={packet.link_state} "
                f"last_error={packet.last_error} {counters}"
            )

    def _handle_tx(self, request: TxRequest) -> None:
        if request.label != AUTO_HEALTH_LABEL:
            self._log(f"TX {request.label} {request.frame.hex(' ')}")
        if self.pending_command is not None:
            total_attempts = COMMAND_MAX_RETRIES + 1
            attempt = self.pending_command.retries_done + 1
            self.pending_command.waiting_for_tx = False
            self.pending_command.deadline = time.monotonic() + COMMAND_RESPONSE_TIMEOUT_SEC
            if self.pending_command.label != AUTO_HEALTH_LABEL:
                self._set_status(f"Waiting {self.pending_command.label} ({attempt}/{total_attempts})")

    def _check_command_timeout(self) -> None:
        if self.pending_command is None:
            return
        if self.pending_command.waiting_for_tx:
            return
        if time.monotonic() < self.pending_command.deadline:
            return

        label = self.pending_command.label
        if self.pending_command.retries_done < COMMAND_MAX_RETRIES:
            self.pending_command.retries_done += 1
            self._log(
                f"{label} timeout after {COMMAND_RESPONSE_TIMEOUT_SEC:.1f}s, "
                f"retry {self.pending_command.retries_done}/{COMMAND_MAX_RETRIES}"
            )
            self._send_pending_command_attempt()
            return

        self.pending_command = None
        self.tc_timeout_count += 1
        self.tc_timeout_count_var.set(str(self.tc_timeout_count))
        self._set_status(f"{label} timeout")
        self._set_connected_ui(self.reader is not None)
        self._log(
            f"{label} failed after {COMMAND_MAX_RETRIES} retries "
            f"({COMMAND_RESPONSE_TIMEOUT_SEC:.1f}s each)"
        )

    def _update_packet_age(self) -> None:
        if self.last_packet_time > 0.0:
            age = time.monotonic() - self.last_packet_time
            self.age_var.set(f"{age:.1f}s")
        self.after(500, self._update_packet_age)

    def _set_connected_ui(self, connected: bool) -> None:
        serial_active = self.reader is not None
        command_ready = connected and self.serial_ready and not self._has_pending_command()
        normal_command_ready = command_ready and self.current_mode_value == MODE_TO_VALUE["normal_mode"]
        thruster_command_ready = command_ready and self.current_mode_value == MODE_TO_VALUE["normal_mode"]
        sim_start_ready = command_ready and self.current_mode_value in (
            MODE_TO_VALUE["normal_mode"],
            MODE_TO_VALUE["diagnostic_mode"],
        )
        sim_sensor_ready = command_ready and self.sensor_override_active
        self.connect_button.configure(state="disabled" if serial_active else "normal")
        self.disconnect_button.configure(state="normal" if serial_active else "disabled")
        self.port_combo.configure(state="disabled" if serial_active else "readonly")
        self.baud_combo.configure(state="disabled" if serial_active else "normal")
        self.request_sensor_tm_button.configure(state="normal" if command_ready else "disabled")
        self.request_sv_tm_button.configure(state="normal" if command_ready else "disabled")
        self.request_health_button.configure(state="normal" if command_ready else "disabled")
        self.ping_button.configure(state="normal" if command_ready else "disabled")
        self.reboot_button.configure(state="normal" if command_ready else "disabled")
        self.send_command_button.configure(state="normal" if command_ready else "disabled")
        self.all_off_button.configure(state="normal" if command_ready else "disabled")
        self.send_mode_button.configure(state="normal" if command_ready else "disabled")
        self.thruster_start_button.configure(state="normal" if thruster_command_ready else "disabled")
        self.par_start_button.configure(state="normal" if normal_command_ready else "disabled")
        self.par_stop_button.configure(state="normal" if command_ready else "disabled")
        self.sim_start_button.configure(state="normal" if sim_start_ready else "disabled")
        self.send_sim_sensor_button.configure(state="normal" if sim_sensor_ready else "disabled")
        self.sim_stop_button.configure(state="normal" if command_ready else "disabled")
        self.mode_combo.configure(state="readonly" if not self._has_pending_command() else "disabled")
        for entry in self.thruster_param_entries:
            entry.configure(state="normal" if not self._has_pending_command() else "disabled")
        for combo in self.par_sensor_combos:
            combo.configure(state="readonly" if not self._has_pending_command() else "disabled")
        for entry in self.sim_sensor_entries:
            entry.configure(state="normal" if not self._has_pending_command() else "disabled")

    def _set_status(self, text: str) -> None:
        self.status_var.set(text)

    def _log(self, text: str) -> None:
        timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
        self.log_text.configure(state="normal")
        self.log_text.insert("end", f"[{timestamp}] {text}\n")
        self.log_text.see("end")
        self.log_text.configure(state="disabled")

    def _on_close(self) -> None:
        self.disconnect_serial(wait=True)
        self.destroy()


def main() -> None:
    app = PscCspMonitorApp()
    app.mainloop()


if __name__ == "__main__":
    main()
