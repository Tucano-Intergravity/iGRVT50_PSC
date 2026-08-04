#!/usr/bin/env python3
"""
GUI UART monitor for PSC iGRVT50 sensor packets.

Packet format:
    $iGRVT50,<tick>,<mode>,<PT1 mV>...<PT9 mV>,<TC1 uV>...<TC4 uV>\r\n
"""

from __future__ import annotations

import queue
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


HEADER = "$iGRVT50"
COMMAND_SVCON = "SVCON"
COMMAND_TMREQ = "TMREQ"
COMMAND_DIAG = "DIAG"
COMMAND_MODE = "MODE"
ACK_DATA = "Ack"
PT_COUNT = 9
TC_COUNT = 4
LPV_COUNT = 12
HPV_COUNT = 8
EXPECTED_FIELD_COUNT = 1 + 1 + 1 + PT_COUNT + TC_COUNT
SVCON_FIELD_COUNT = 1 + 1 + LPV_COUNT + HPV_COUNT
ACK_FIELD_COUNT = 2
DIAG_VALUE_NAMES = (
    "tick",
    "rxBytes",
    "rxDrops",
    "rxErrors",
    "lines",
    "noHeader",
    "headers",
    "badFields",
    "badBinary",
    "unknownCmd",
    "tmreq",
    "svcon",
    "ackSent",
    "overflow",
    "lastLen",
)
DIAG_FIELD_COUNT = 1 + 1 + len(DIAG_VALUE_NAMES)
DEFAULT_BAUDRATE = 921600
SERIAL_READ_TIMEOUT_SEC = 0.02
SERIAL_PORT_SETTLE_SEC = 0.2
MAX_RX_BUFFER_SIZE = 4096
COMMAND_RESPONSE_TIMEOUT_SEC = 0.1
COMMAND_MAX_RETRIES = 5
MODE_OPTIONS = ("init_mode", "normal_mode", "run_mode", "diagnostic_mode")


@dataclass
class SensorPacket:
    tick: int
    mode: str
    pt_values: list[int]
    tc_values: list[int]
    raw_line: str


@dataclass
class DiagPacket:
    values: dict[str, int]
    raw_line: str


@dataclass
class TxRequest:
    text: str
    total_count: int
    repeat_count: int
    interval_sec: float
    next_send_time: float = 0.0


@dataclass
class PendingCommand:
    command: str
    packet: str
    retries_done: int = 0
    deadline: float = 0.0
    waiting_for_tx: bool = False


def parse_packet(line: str) -> SensorPacket:
    fields = line.strip().split(",")
    if len(fields) != EXPECTED_FIELD_COUNT:
        raise ValueError(f"field count {len(fields)} != {EXPECTED_FIELD_COUNT}")
    if fields[0] != HEADER:
        raise ValueError(f"header {fields[0]!r} != {HEADER!r}")

    tick = int(fields[1], 10)
    mode = fields[2]
    pt_start = 3
    tc_start = pt_start + PT_COUNT
    pt_values = [int(value, 10) for value in fields[pt_start:tc_start]]
    tc_values = [int(value, 10) for value in fields[tc_start : tc_start + TC_COUNT]]
    return SensorPacket(tick=tick, mode=mode, pt_values=pt_values, tc_values=tc_values, raw_line=line)


def is_ack_packet(line: str) -> bool:
    fields = line.strip().split(",")
    return len(fields) == ACK_FIELD_COUNT and fields[0] == HEADER and fields[1] == ACK_DATA


def parse_diag_packet(line: str) -> DiagPacket:
    fields = line.strip().split(",")
    if len(fields) != DIAG_FIELD_COUNT:
        raise ValueError(f"diag field count {len(fields)} != {DIAG_FIELD_COUNT}")
    if fields[0] != HEADER or fields[1] != COMMAND_DIAG:
        raise ValueError("not a DIAG packet")

    values = {name: int(value, 10) for name, value in zip(DIAG_VALUE_NAMES, fields[2:])}
    return DiagPacket(values=values, raw_line=line)


class SerialReader(threading.Thread):
    def __init__(self, port: str, baudrate: int, rx_queue: queue.Queue[tuple[str, object]]) -> None:
        super().__init__(daemon=True)
        self._port = port
        self._baudrate = baudrate
        self._rx_queue = rx_queue
        self._tx_queue: queue.Queue[TxRequest] = queue.Queue()
        self._stop_event = threading.Event()
        self._serial_port = None

    def send_text(self, text: str, repeat_count: int = 1, interval_sec: float = 0.0) -> None:
        repeat = max(1, repeat_count)
        interval = max(0.0, interval_sec)
        self._tx_queue.put(TxRequest(text=text, total_count=repeat, repeat_count=repeat, interval_sec=interval))

    def stop(self) -> None:
        self._stop_event.set()
        if self._serial_port is not None:
            try:
                self._serial_port.close()
            except serial.SerialException:
                pass

    def run(self) -> None:
        rx_buffer = bytearray()
        pending_tx: list[TxRequest] = []

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
            self._rx_queue.put(("opened", f"Opened {self._port} at {self._baudrate} bps, 8N1"))

            while not self._stop_event.is_set():
                while True:
                    try:
                        pending_tx.append(self._tx_queue.get_nowait())
                    except queue.Empty:
                        break

                now = time.monotonic()
                for request in pending_tx[:]:
                    if request.next_send_time > now:
                        continue

                    self._serial_port.write(request.text.encode("ascii"))
                    self._serial_port.flush()
                    if request.total_count > 1:
                        attempt = (request.total_count - request.repeat_count) + 1
                        tx_text = f"{request.text.strip()} [{attempt}/{request.total_count}]"
                    else:
                        tx_text = request.text.strip()
                    self._rx_queue.put(("tx", tx_text))

                    request.repeat_count -= 1
                    if request.repeat_count <= 0:
                        pending_tx.remove(request)
                    else:
                        request.next_send_time = now + request.interval_sec

                chunk = self._serial_port.read(self._serial_port.in_waiting or 1)
                if not chunk:
                    continue

                rx_buffer.extend(chunk)
                if len(rx_buffer) > MAX_RX_BUFFER_SIZE:
                    rx_buffer.clear()
                    self._rx_queue.put(("ignored_line", "RX buffer overflow, resynchronized"))

                while b"\n" in rx_buffer:
                    raw_line, _, remainder = rx_buffer.partition(b"\n")
                    rx_buffer = bytearray(remainder)

                    line = raw_line.decode("ascii", errors="replace").strip()
                    if not line:
                        continue

                    header_pos = line.find(HEADER)
                    if header_pos < 0:
                        self._rx_queue.put(("ignored_line", line))
                        continue
                    if header_pos > 0:
                        self._rx_queue.put(("ignored_line", line[:header_pos]))
                        line = line[header_pos:]

                    if is_ack_packet(line):
                        self._rx_queue.put(("ack", line))
                        continue

                    try:
                        diag_packet = parse_diag_packet(line)
                    except ValueError:
                        diag_packet = None
                    if diag_packet is not None:
                        self._rx_queue.put(("diag", diag_packet))
                        continue

                    try:
                        packet = parse_packet(line)
                    except ValueError as exc:
                        self._rx_queue.put(("parse_error", f"{line} ({exc})"))
                        continue

                    self._rx_queue.put(("packet", packet))

        except serial.SerialException as exc:
            self._rx_queue.put(("serial_error", str(exc)))
        finally:
            if self._serial_port is not None and self._serial_port.is_open:
                self._serial_port.close()
            self._rx_queue.put(("closed", "Serial port closed"))


class PscUartMonitorApp(tk.Tk):
    def __init__(self) -> None:
        super().__init__()
        self.title("PSC UART Sensor Monitor")
        self.geometry("1120x760")
        self.minsize(980, 700)

        self.rx_queue: queue.Queue[tuple[str, object]] = queue.Queue()
        self.reader: SerialReader | None = None
        self.packet_count = 0
        self.ignored_count = 0
        self.serial_error_count = 0
        self.tc_timeout_count = 0
        self.last_packet_time = 0.0
        self.pending_command: PendingCommand | None = None
        self.serial_ready = False

        self.port_var = tk.StringVar()
        self.baud_var = tk.StringVar(value=str(DEFAULT_BAUDRATE))
        self.status_var = tk.StringVar(value="Disconnected")
        self.mode_var = tk.StringVar(value="normal_mode")
        self.tm_mode_var = tk.StringVar(value="-")
        self.tick_var = tk.StringVar(value="-")
        self.packet_count_var = tk.StringVar(value="0")
        self.ignored_count_var = tk.StringVar(value="0")
        self.serial_error_count_var = tk.StringVar(value="0")
        self.tc_timeout_count_var = tk.StringVar(value="0")
        self.last_update_var = tk.StringVar(value="-")
        self.age_var = tk.StringVar(value="-")

        self.pt_vars = [tk.StringVar(value="-") for _ in range(PT_COUNT)]
        self.tc_vars = [tk.StringVar(value="-") for _ in range(TC_COUNT)]
        self.lpv_cmd_vars = [tk.IntVar(value=0) for _ in range(LPV_COUNT)]
        self.hpv_cmd_vars = [tk.IntVar(value=0) for _ in range(HPV_COUNT)]

        self._build_ui()
        self.refresh_ports()
        self.after(20, self._poll_queue)
        self.after(500, self._update_packet_age)
        self.protocol("WM_DELETE_WINDOW", self._on_close)

    def _build_ui(self) -> None:
        self.columnconfigure(0, weight=1)
        self.rowconfigure(5, weight=1)

        connection = ttk.LabelFrame(self, text="Connection")
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

        status = ttk.Frame(self)
        status.grid(row=1, column=0, padx=12, pady=6, sticky="ew")
        for col in range(9):
            status.columnconfigure(col, weight=1)

        self._add_status_item(status, 0, "Status", self.status_var)
        self._add_status_item(status, 1, "Tick", self.tick_var)
        self._add_status_item(status, 2, "Mode", self.tm_mode_var)
        self._add_status_item(status, 3, "Packets", self.packet_count_var)
        self._add_status_item(status, 4, "Ignored", self.ignored_count_var)
        self._add_status_item(status, 5, "Serial Err", self.serial_error_count_var)
        self._add_status_item(status, 6, "Last Update", self.last_update_var)
        self._add_status_item(status, 7, "Age", self.age_var)
        self._add_status_item(status, 8, "Timeouts", self.tc_timeout_count_var)

        values = ttk.Frame(self)
        values.grid(row=2, column=0, padx=12, pady=6, sticky="ew")
        values.columnconfigure(0, weight=1)

        pt_frame = ttk.LabelFrame(values, text="Pressure Transducers (mV)")
        pt_frame.grid(row=0, column=0, pady=(0, 6), sticky="ew")
        for col in range(PT_COUNT):
            pt_frame.columnconfigure(col, weight=1)

        for idx, value_var in enumerate(self.pt_vars):
            self._add_value_cell(pt_frame, 0, idx, f"PT{idx + 1}", value_var, "mV")

        tc_frame = ttk.LabelFrame(values, text="Thermocouples (uV)")
        tc_frame.grid(row=1, column=0, sticky="ew")
        for col in range(TC_COUNT):
            tc_frame.columnconfigure(col, weight=1)

        for idx, value_var in enumerate(self.tc_vars):
            self._add_value_cell(tc_frame, 0, idx, f"TC{idx + 1}", value_var, "uV")

        command_frame = ttk.LabelFrame(self, text="Telecommand - Sol Valves")
        command_frame.grid(row=3, column=0, padx=12, pady=6, sticky="ew")
        command_frame.columnconfigure(0, weight=3)
        command_frame.columnconfigure(1, weight=2)

        lpv_frame = ttk.Frame(command_frame)
        lpv_frame.grid(row=0, column=0, padx=(10, 8), pady=8, sticky="ew")
        for col in range(6):
            lpv_frame.columnconfigure(col, weight=1)
        ttk.Label(lpv_frame, text="LPV").grid(row=0, column=0, sticky="w", pady=(0, 4))
        for idx, variable in enumerate(self.lpv_cmd_vars):
            self._add_valve_check(lpv_frame, (idx // 6) + 1, idx % 6, f"LPV{idx + 1}", variable)

        hpv_frame = ttk.Frame(command_frame)
        hpv_frame.grid(row=0, column=1, padx=(8, 10), pady=8, sticky="ew")
        for col in range(4):
            hpv_frame.columnconfigure(col, weight=1)
        ttk.Label(hpv_frame, text="HPV").grid(row=0, column=0, sticky="w", pady=(0, 4))
        for idx, variable in enumerate(self.hpv_cmd_vars):
            self._add_valve_check(hpv_frame, (idx // 4) + 1, idx % 4, f"HPV{idx + 1}", variable)

        control_frame = ttk.LabelFrame(self, text="Commands")
        control_frame.grid(row=4, column=0, padx=12, pady=6, sticky="ew")
        control_frame.columnconfigure(0, weight=1)
        control_frame.columnconfigure(1, weight=1)

        mode_frame = ttk.Frame(control_frame)
        mode_frame.grid(row=0, column=0, padx=10, pady=10, sticky="w")
        ttk.Label(mode_frame, text="Mode").grid(row=0, column=0, padx=(0, 6))
        self.mode_combo = ttk.Combobox(
            mode_frame,
            textvariable=self.mode_var,
            values=MODE_OPTIONS,
            width=18,
            state="readonly",
        )
        self.mode_combo.grid(row=0, column=1, padx=(0, 6))
        self.send_mode_button = ttk.Button(
            mode_frame,
            text="Send MODE",
            command=self.send_mode,
            state="disabled",
        )
        self.send_mode_button.grid(row=0, column=2)

        command_buttons = ttk.Frame(control_frame)
        command_buttons.grid(row=0, column=1, padx=10, pady=10, sticky="e")
        self.request_tm_button = ttk.Button(
            command_buttons,
            text="Request TM",
            command=self.send_tmreq,
            state="disabled",
        )
        self.request_tm_button.grid(row=0, column=0, padx=(0, 6))
        self.send_command_button = ttk.Button(
            command_buttons,
            text="Send SVCON",
            command=self.send_telecommand,
            state="disabled",
        )
        self.send_command_button.grid(row=0, column=1, padx=(0, 6))
        self.all_off_button = ttk.Button(
            command_buttons,
            text="All Off",
            command=self.send_all_off,
            state="disabled",
        )
        self.all_off_button.grid(row=0, column=2)

        log_frame = ttk.LabelFrame(self, text="Log")
        log_frame.grid(row=5, column=0, padx=12, pady=(6, 12), sticky="nsew")
        log_frame.columnconfigure(0, weight=1)
        log_frame.rowconfigure(0, weight=1)

        self.log_text = tk.Text(log_frame, height=8, wrap="none", state="disabled")
        self.log_text.grid(row=0, column=0, sticky="nsew")

        log_scroll_y = ttk.Scrollbar(log_frame, orient="vertical", command=self.log_text.yview)
        log_scroll_y.grid(row=0, column=1, sticky="ns")
        self.log_text.configure(yscrollcommand=log_scroll_y.set)

        log_buttons = ttk.Frame(log_frame)
        log_buttons.grid(row=1, column=0, columnspan=2, sticky="e", pady=(6, 0))
        self.request_diag_button = ttk.Button(
            log_buttons,
            text="Request DIAG",
            command=self.send_diag,
            state="disabled",
        )
        self.request_diag_button.grid(row=0, column=0, padx=(0, 6))
        ttk.Button(log_buttons, text="Clear Log", command=self.clear_log).grid(row=0, column=1, padx=(0, 6))

    def _add_status_item(self, parent: ttk.Frame, column: int, label: str, variable: tk.StringVar) -> None:
        frame = ttk.Frame(parent)
        frame.grid(row=0, column=column, padx=4, sticky="ew")
        ttk.Label(frame, text=label).grid(row=0, column=0, sticky="w")
        ttk.Label(frame, textvariable=variable, font=("Segoe UI", 12, "bold")).grid(row=1, column=0, sticky="w")

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

    def _add_valve_check(self, parent: ttk.Frame, row: int, column: int, label: str, variable: tk.IntVar) -> None:
        check = ttk.Checkbutton(parent, text=label, variable=variable)
        check.grid(row=row, column=column, padx=4, pady=3, sticky="w")

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
        self.packet_count_var.set("0")
        self.ignored_count_var.set("0")
        self.serial_error_count_var.set("0")
        self.tc_timeout_count_var.set("0")
        self.tick_var.set("-")
        self.last_update_var.set("-")
        self.age_var.set("-")
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

    def build_telecommand_packet(self) -> str:
        states = [str(var.get() & 1) for var in self.lpv_cmd_vars]
        states.extend(str(var.get() & 1) for var in self.hpv_cmd_vars)
        return f"{HEADER},{COMMAND_SVCON},{','.join(states)}\r\n"

    def build_tmreq_packet(self) -> str:
        return f"{HEADER},{COMMAND_TMREQ}\r\n"

    def build_diag_packet(self) -> str:
        return f"{HEADER},{COMMAND_DIAG}\r\n"

    def build_mode_packet(self) -> str:
        mode = self.mode_var.get().strip()
        if mode not in MODE_OPTIONS:
            mode = "normal_mode"
            self.mode_var.set(mode)
        return f"{HEADER},{COMMAND_MODE},{mode}\r\n"

    def _has_pending_command(self) -> bool:
        return self.pending_command is not None

    def _start_pending_command(self, command: str, packet: str) -> None:
        if self.reader is None or not self.serial_ready:
            messagebox.showwarning("Not connected", "Connect to a serial port first.")
            return
        if self._has_pending_command():
            messagebox.showwarning("Command pending", "Wait for the current command response or timeout.")
            return

        self.pending_command = PendingCommand(command=command, packet=packet)
        self._send_pending_command_attempt()

    def _send_pending_command_attempt(self) -> None:
        if self.reader is None or self.pending_command is None:
            return

        total_attempts = COMMAND_MAX_RETRIES + 1
        attempt = self.pending_command.retries_done + 1
        self.pending_command.deadline = 0.0
        self.pending_command.waiting_for_tx = True
        self._set_connected_ui(True)
        self._set_status(f"Sending {self.pending_command.command} ({attempt}/{total_attempts})")
        self.reader.send_text(self.pending_command.packet)

    def _complete_pending_command(self, response: str) -> None:
        if self.pending_command is None:
            return

        command = self.pending_command.command
        self.pending_command = None
        self._set_status(f"{command} {response}")
        self._set_connected_ui(self.reader is not None)
        self._log(f"{command} {response}")

    def send_telecommand(self) -> None:
        self._start_pending_command(COMMAND_SVCON, self.build_telecommand_packet())

    def send_tmreq(self) -> None:
        self._start_pending_command(COMMAND_TMREQ, self.build_tmreq_packet())

    def send_diag(self) -> None:
        self._start_pending_command(COMMAND_DIAG, self.build_diag_packet())

    def send_mode(self) -> None:
        self._start_pending_command(COMMAND_MODE, self.build_mode_packet())

    def send_all_off(self) -> None:
        for variable in self.lpv_cmd_vars:
            variable.set(0)
        for variable in self.hpv_cmd_vars:
            variable.set(0)
        self.send_telecommand()

    def _poll_queue(self) -> None:
        while True:
            try:
                message_type, payload = self.rx_queue.get_nowait()
            except queue.Empty:
                break

            if message_type == "packet":
                self._handle_packet(payload)
            elif message_type == "diag":
                self._handle_diag(payload)
            elif message_type == "parse_error":
                self.ignored_count += 1
                self.ignored_count_var.set(str(self.ignored_count))
                self._log(f"Ignored malformed packet: {payload}")
            elif message_type == "ignored_line":
                self.ignored_count += 1
                self.ignored_count_var.set(str(self.ignored_count))
                self._log(f"Ignored non-packet data: {payload}")
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
            elif message_type == "status":
                self._set_status(str(payload))
                self._log(str(payload))
            elif message_type == "ack":
                self._handle_ack(str(payload))
            elif message_type == "tx":
                self._handle_tx(str(payload))
            elif message_type == "closed":
                self._log(str(payload))

        self._check_command_timeout()
        self.after(20, self._poll_queue)

    def _handle_packet(self, packet: SensorPacket) -> None:
        now = datetime.now()
        self.packet_count += 1
        self.last_packet_time = time.monotonic()

        self.tick_var.set(str(packet.tick))
        self.tm_mode_var.set(packet.mode)
        self.packet_count_var.set(str(self.packet_count))
        self.last_update_var.set(now.strftime("%H:%M:%S"))

        for value_var, value in zip(self.pt_vars, packet.pt_values):
            value_var.set(str(value))
        for value_var, value in zip(self.tc_vars, packet.tc_values):
            value_var.set(str(value))

        if self.pending_command is not None and self.pending_command.command == COMMAND_TMREQ:
            self._complete_pending_command("TM received")
        elif self.pending_command is None:
            self._set_status("Receiving")
        self._log(f"RX {packet.raw_line}")

    def _handle_diag(self, packet: DiagPacket) -> None:
        summary = " ".join(f"{name}={packet.values[name]}" for name in DIAG_VALUE_NAMES)
        self._log(f"RX {packet.raw_line}")
        self._log(f"DIAG {summary}")
        if self.pending_command is not None and self.pending_command.command == COMMAND_DIAG:
            self._complete_pending_command("DIAG received")

    def _handle_ack(self, line: str) -> None:
        if self.pending_command is not None and self.pending_command.command in (COMMAND_SVCON, COMMAND_MODE):
            self._log(f"RX {line}")
            self._complete_pending_command("Ack")
        else:
            self._log(f"RX unexpected Ack: {line}")

    def _handle_tx(self, line: str) -> None:
        self._log(f"TX {line}")
        if self.pending_command is not None:
            total_attempts = COMMAND_MAX_RETRIES + 1
            attempt = self.pending_command.retries_done + 1
            self.pending_command.waiting_for_tx = False
            self.pending_command.deadline = time.monotonic() + COMMAND_RESPONSE_TIMEOUT_SEC
            self._set_status(f"Waiting {self.pending_command.command} ({attempt}/{total_attempts})")

    def _check_command_timeout(self) -> None:
        if self.pending_command is None:
            return
        if self.pending_command.waiting_for_tx:
            return
        if time.monotonic() < self.pending_command.deadline:
            return

        command = self.pending_command.command
        if self.pending_command.retries_done < COMMAND_MAX_RETRIES:
            self.pending_command.retries_done += 1
            self._log(
                f"{command} timeout after {COMMAND_RESPONSE_TIMEOUT_SEC:.1f}s, "
                f"retry {self.pending_command.retries_done}/{COMMAND_MAX_RETRIES}"
            )
            self._send_pending_command_attempt()
            return

        self.pending_command = None
        self.tc_timeout_count += 1
        self.tc_timeout_count_var.set(str(self.tc_timeout_count))
        self._set_status(f"{command} timeout")
        self._set_connected_ui(self.reader is not None)
        self._log(
            f"{command} failed after {COMMAND_MAX_RETRIES} retries "
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
        self.connect_button.configure(state="disabled" if serial_active else "normal")
        self.disconnect_button.configure(state="normal" if serial_active else "disabled")
        self.port_combo.configure(state="disabled" if serial_active else "readonly")
        self.baud_combo.configure(state="disabled" if serial_active else "normal")
        self.request_tm_button.configure(state="normal" if command_ready else "disabled")
        self.request_diag_button.configure(state="normal" if command_ready else "disabled")
        self.send_command_button.configure(state="normal" if command_ready else "disabled")
        self.all_off_button.configure(state="normal" if command_ready else "disabled")
        self.send_mode_button.configure(state="normal" if command_ready else "disabled")
        self.mode_combo.configure(state="readonly" if not self._has_pending_command() else "disabled")

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
    app = PscUartMonitorApp()
    app.mainloop()


if __name__ == "__main__":
    main()
