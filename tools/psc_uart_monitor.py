#!/usr/bin/env python3
"""
Simple UART monitor for PSC iGRVT50 sensor packets.

Packet format:
    $iGRVT50,<tick>,<PT1 mV>...<PT9 mV>,<TC1 uV>...<TC4 uV>\r\n
"""

from __future__ import annotations

import argparse
import sys
from datetime import datetime

try:
    import serial
    from serial.tools import list_ports
except ImportError:
    print("pyserial is required. Install it with: python -m pip install pyserial")
    sys.exit(1)


HEADER = "$iGRVT50"
PT_COUNT = 9
TC_COUNT = 4
EXPECTED_FIELD_COUNT = 1 + 1 + PT_COUNT + TC_COUNT


def build_arg_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Monitor PSC sensor UART packets.")
    parser.add_argument("-p", "--port", help="Serial port, for example COM3.")
    parser.add_argument("-b", "--baud", type=int, default=921600, help="Baud rate. Default: 921600.")
    parser.add_argument("--timeout", type=float, default=1.0, help="Read timeout in seconds. Default: 1.0.")
    parser.add_argument("--list", action="store_true", help="List available serial ports and exit.")
    parser.add_argument("--raw", action="store_true", help="Print raw lines when packet parsing fails.")
    return parser


def list_serial_ports() -> list[str]:
    ports = list(list_ports.comports())
    if not ports:
        print("No serial ports found.")
        return []

    names: list[str] = []
    print("Available serial ports:")
    for port in ports:
        names.append(port.device)
        print(f"  {port.device:8s} {port.description}")
    return names


def choose_port_from_prompt() -> str:
    names = list_serial_ports()
    if not names:
        sys.exit(1)

    selected = input("Port to open: ").strip()
    if not selected:
        selected = names[0]
        print(f"Using {selected}")
    return selected


def parse_packet(line: str) -> tuple[int, list[int], list[int]]:
    fields = line.strip().split(",")
    if len(fields) != EXPECTED_FIELD_COUNT:
        raise ValueError(f"field count {len(fields)} != {EXPECTED_FIELD_COUNT}")
    if fields[0] != HEADER:
        raise ValueError(f"header {fields[0]!r} != {HEADER!r}")

    tick = int(fields[1], 10)
    pt_values = [int(value, 10) for value in fields[2 : 2 + PT_COUNT]]
    tc_values = [int(value, 10) for value in fields[2 + PT_COUNT : 2 + PT_COUNT + TC_COUNT]]
    return tick, pt_values, tc_values


def format_values(prefix: str, unit: str, values: list[int]) -> str:
    return " ".join(f"{prefix}{idx + 1}:{value}{unit}" for idx, value in enumerate(values))


def monitor(port: str, baud: int, timeout: float, raw_on_error: bool) -> None:
    serial_port = serial.Serial(
        port=port,
        baudrate=baud,
        bytesize=serial.EIGHTBITS,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        timeout=timeout,
    )

    with serial_port:
        print(f"Opened {serial_port.port} at {serial_port.baudrate} bps, 8N1.")
        print("Waiting for packets. Press Ctrl+C to stop.")

        while True:
            raw = serial_port.readline()
            if not raw:
                continue

            line = raw.decode("ascii", errors="replace").strip()
            timestamp = datetime.now().strftime("%H:%M:%S")

            try:
                tick, pt_values, tc_values = parse_packet(line)
            except ValueError as exc:
                if raw_on_error:
                    print(f"[{timestamp}] RAW {line} ({exc})")
                continue

            print(f"[{timestamp}] tick={tick}")
            print(f"  {format_values('PT', 'mV', pt_values)}")
            print(f"  {format_values('TC', 'uV', tc_values)}")


def main() -> int:
    parser = build_arg_parser()
    args = parser.parse_args()

    if args.list:
        list_serial_ports()
        return 0

    port = args.port if args.port else choose_port_from_prompt()

    try:
        monitor(port, args.baud, args.timeout, args.raw)
    except serial.SerialException as exc:
        print(f"Serial error: {exc}")
        return 1
    except KeyboardInterrupt:
        print("\nStopped.")
        return 0

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
