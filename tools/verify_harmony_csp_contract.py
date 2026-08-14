#!/usr/bin/env python3
"""Verify generated Harmony assumptions required by the CSP RS485 port."""

from __future__ import annotations

import csv
import re
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
PA22_MASK = 1 << 22
PA24_MASK = 1 << 24
RS485_DIR_MASK = PA22_MASK | PA24_MASK


def fail(message: str) -> None:
    print(f"FAIL {message}", file=sys.stderr)
    raise SystemExit(1)


def read_text(relative: str) -> str:
    path = ROOT / relative
    if not path.exists():
        fail(f"missing {relative}")
    return path.read_text(encoding="utf-8")


def verify_pin_csv() -> None:
    path = ROOT / "src/config/default/pin_configurations.csv"
    with path.open(newline="", encoding="utf-8-sig") as file:
        rows = {row["Pin ID"]: row for row in csv.DictReader(file)}
    for pin, name in (("PA22", "UART1_DE"), ("PA24", "UART1_nRE")):
        row = rows.get(pin)
        if row is None:
            fail(f"{pin} missing from pin_configurations.csv")
        if row["Custom Name"] != name:
            fail(f"{pin} custom name mismatch: {row['Custom Name']}")
        if row["Function"] != "GPIO":
            fail(f"{pin} must be GPIO, got {row['Function']}")
        if row["Direction"] != "Out":
            fail(f"{pin} must be Out, got {row['Direction']}")
        if row["Latch"] != "Low":
            fail(f"{pin} latch must be Low, got {row['Latch']}")


def verify_pio_boot_safe() -> None:
    text = read_text("src/config/default/peripheral/pio/plib_pio.c")
    match = re.search(r"PIO_PORT_A\)->PIO_OER = 0x([0-9a-fA-F]+)U;", text)
    if match is None:
        fail("PORTA PIO_OER assignment missing")
    oer = int(match.group(1), 16)
    if (oer & RS485_DIR_MASK) != RS485_DIR_MASK:
        fail("PA22/PA24 are not output-enabled in PIO_Initialize")
    if "PIO_PORT_A)->PIO_ODSR = 0x0U;" not in text:
        fail("PORTA initial latch is not low")


def verify_pio_macros() -> None:
    text = read_text("src/config/default/peripheral/pio/plib_pio.h")
    required = [
        "UART1_DE_Set()",
        "UART1_DE_Clear()",
        "UART1_DE_PIN                  PIO_PIN_PA22",
        "UART1_nRE_Set()",
        "UART1_nRE_Clear()",
        "UART1_nRE_PIN                  PIO_PIN_PA24",
    ]
    for item in required:
        if item not in text:
            fail(f"missing PIO macro {item}")


def verify_usart_hooks() -> None:
    text = read_text("src/config/default/peripheral/usart/plib_usart1.c")
    required = [
        "bool __attribute__((weak)) USART1_UartCommRxReadyHook( void )",
        "bool __attribute__((weak)) USART1_UartCommErrorHook( uint32_t errorStatus )",
        "if( USART1_UartCommErrorHook( errorStatus ) == false )",
        "if( USART1_UartCommRxReadyHook() == false )",
    ]
    for item in required:
        if item not in text:
            fail(f"missing USART1 hook contract {item}")


def verify_nvic() -> None:
    text = read_text("src/config/default/peripheral/nvic/plib_nvic.c")
    if "NVIC_SetPriority(USART1_IRQn, 7);" not in text:
        fail("USART1 IRQ priority must remain 7")


def verify_freertos_config() -> None:
    config = read_text("src/config/default/FreeRTOSConfig.h")
    hooks = read_text("src/config/default/freertos_hooks.c")
    required_config = [
        "#define configSUPPORT_STATIC_ALLOCATION         1",
        "#define configUSE_TASK_NOTIFICATIONS            1",
        "#define INCLUDE_uxTaskGetStackHighWaterMark     1",
        "#define INCLUDE_xTaskGetHandle                  1",
        "#define INCLUDE_uxTaskGetStackHighWaterMark2    1",
    ]
    for item in required_config:
        if item not in config:
            fail(f"FreeRTOSConfig missing {item}")
    if "void vApplicationGetIdleTaskMemory(" not in hooks:
        fail("static idle task memory hook missing")


def main() -> int:
    verify_pin_csv()
    verify_pio_boot_safe()
    verify_pio_macros()
    verify_usart_hooks()
    verify_nvic()
    verify_freertos_config()
    print("PASS Harmony CSP contract")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
