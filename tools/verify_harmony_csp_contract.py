#!/usr/bin/env python3
"""Verify the generated Harmony contract required by the CSP RS485 port."""

from __future__ import annotations

import ast
import csv
import re
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
CSV_PATH = ROOT / "src/config/default/pin_configurations.csv"
PIO_C_PATH = ROOT / "src/config/default/peripheral/pio/plib_pio.c"
PIO_H_PATH = ROOT / "src/config/default/peripheral/pio/plib_pio.h"
USART_PATH = ROOT / "src/config/default/peripheral/usart/plib_usart1.c"
NVIC_PATH = ROOT / "src/config/default/peripheral/nvic/plib_nvic.c"
INIT_PATH = ROOT / "src/config/default/initialization.c"

PIN_CONTRACT = {
    "PA22": ("UART1_DE", 22),
    "PA24": ("UART1_nRE", 24),
}
REQUIRED_PORTA_MASK = (1 << 22) | (1 << 24)


class Verification:
    def __init__(self) -> None:
        self.failures: list[str] = []

    def fail(self, path: Path, invariant: str, context: str) -> None:
        try:
            shown_path = path.relative_to(ROOT)
        except ValueError:
            shown_path = path
        self.failures.append(f"FAIL {shown_path}: {invariant}; {context}")

    def read_text(self, path: Path) -> str | None:
        try:
            return path.read_text(encoding="utf-8")
        except (OSError, UnicodeError) as exc:
            self.fail(path, "file must be present and readable", str(exc))
            return None


def strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    return re.sub(r"//[^\r\n]*", "", text)


def matching_delimiter(text: str, start: int, opening: str, closing: str) -> int:
    if start >= len(text) or text[start] != opening:
        raise ValueError(f"expected {opening!r} at offset {start}")
    depth = 0
    for index in range(start, len(text)):
        if text[index] == opening:
            depth += 1
        elif text[index] == closing:
            depth -= 1
            if depth == 0:
                return index
    raise ValueError(f"unmatched {opening!r} at offset {start}")


def function_bodies(text: str, name: str) -> list[str]:
    bodies: list[str] = []
    pattern = re.compile(rf"\b{re.escape(name)}\s*\(")
    for match in pattern.finditer(text):
        open_paren = text.find("(", match.start())
        try:
            close_paren = matching_delimiter(text, open_paren, "(", ")")
        except ValueError:
            continue
        tail = text[close_paren + 1 :]
        next_brace = tail.find("{")
        next_semicolon = tail.find(";")
        if next_brace < 0 or (0 <= next_semicolon < next_brace):
            continue
        open_brace = close_paren + 1 + next_brace
        try:
            close_brace = matching_delimiter(text, open_brace, "{", "}")
        except ValueError:
            continue
        bodies.append(text[open_brace + 1 : close_brace])
    return bodies


def c_integer(expression: str) -> int:
    expression = re.sub(
        r"\(\s*(?:u?int(?:8|16|32|64)_t|unsigned(?:\s+(?:char|short|int|long))?|size_t)\s*\)",
        "",
        expression,
    )
    expression = re.sub(
        r"(?i)(\b(?:0x[0-9a-f]+|\d+))[ul]+\b", r"\1", expression
    )
    parsed = ast.parse(expression.strip(), mode="eval")

    binary_ops = {
        ast.BitOr: lambda left, right: left | right,
        ast.BitAnd: lambda left, right: left & right,
        ast.BitXor: lambda left, right: left ^ right,
        ast.LShift: lambda left, right: left << right,
        ast.RShift: lambda left, right: left >> right,
        ast.Add: lambda left, right: left + right,
        ast.Sub: lambda left, right: left - right,
    }
    unary_ops = {
        ast.Invert: lambda value: ~value,
        ast.UAdd: lambda value: value,
        ast.USub: lambda value: -value,
    }

    def evaluate(node: ast.AST) -> int:
        if isinstance(node, ast.Expression):
            return evaluate(node.body)
        if isinstance(node, ast.Constant) and type(node.value) is int:
            return node.value
        if isinstance(node, ast.BinOp) and type(node.op) in binary_ops:
            return binary_ops[type(node.op)](evaluate(node.left), evaluate(node.right))
        if isinstance(node, ast.UnaryOp) and type(node.op) in unary_ops:
            return unary_ops[type(node.op)](evaluate(node.operand))
        raise ValueError(f"unsupported constant expression: {expression!r}")

    return evaluate(parsed) & 0xFFFFFFFF


def verify_csv(check: Verification) -> None:
    try:
        with CSV_PATH.open("r", encoding="utf-8-sig", newline="") as source:
            reader = csv.DictReader(source)
            required_columns = {"Pin ID", "Custom Name", "Function", "Direction", "Latch"}
            if reader.fieldnames is None or not required_columns.issubset(reader.fieldnames):
                check.fail(
                    CSV_PATH,
                    "CSV must expose the Harmony pin-contract columns",
                    f"columns={reader.fieldnames!r}",
                )
                return
            rows = list(reader)
    except (OSError, UnicodeError, csv.Error) as exc:
        check.fail(CSV_PATH, "CSV must be present, readable, and parseable", str(exc))
        return

    for pin_id, (custom_name, _bit) in PIN_CONTRACT.items():
        pin_rows = [row for row in rows if (row.get("Pin ID") or "").strip() == pin_id]
        if len(pin_rows) != 1:
            check.fail(
                CSV_PATH,
                f"{pin_id} must have exactly one row",
                f"found={len(pin_rows)}",
            )
            continue
        row = pin_rows[0]
        actual = {
            "Custom Name": (row.get("Custom Name") or "").strip(),
            "Function": (row.get("Function") or "").strip(),
            "Direction": (row.get("Direction") or "").strip(),
            "Latch": (row.get("Latch") or "").strip(),
        }
        expected = {
            "Custom Name": custom_name,
            "Function": "GPIO",
            "Direction": "Out",
            "Latch": "Low",
        }
        if actual != expected:
            check.fail(
                CSV_PATH,
                f"{pin_id} must be {custom_name},GPIO,Out,Low",
                f"actual={actual!r}",
            )


def register_assignment(
    check: Verification, body: str, register: str
) -> tuple[int, str] | None:
    target = (
        r"(?:\(\s*\(\s*pio_registers_t\s*\*\s*\)\s*PIO_PORT_A\s*\)"
        r"|PIOA_REGS)\s*->\s*" + re.escape(register)
    )
    matches = list(re.finditer(target + r"\s*=\s*([^;]+);", body))
    if len(matches) != 1:
        check.fail(
            PIO_C_PATH,
            f"PIO_Initialize must assign PORTA {register} exactly once",
            f"found={len(matches)}",
        )
        return None
    expression = matches[0].group(1).strip()
    try:
        return c_integer(expression), expression
    except (SyntaxError, ValueError, TypeError) as exc:
        check.fail(
            PIO_C_PATH,
            f"PORTA {register} mask must be a parseable integer expression",
            f"expression={expression!r}, error={exc}",
        )
        return None


def verify_pio_initialization(check: Verification) -> None:
    text = check.read_text(PIO_C_PATH)
    if text is None:
        return
    bodies = function_bodies(strip_comments(text), "PIO_Initialize")
    if len(bodies) != 1:
        check.fail(
            PIO_C_PATH,
            "PIO_Initialize must have exactly one definition",
            f"found={len(bodies)}",
        )
        return
    body = bodies[0]
    values: dict[str, int] = {}
    expressions: dict[str, str] = {}
    for register in ("PIO_PDR", "PIO_PER", "PIO_OER", "PIO_ODR", "PIO_ODSR"):
        result = register_assignment(check, body, register)
        if result is not None:
            values[register], expressions[register] = result

    checks = (
        ("PIO_PDR", 0, "must not assign PA22/PA24 to a peripheral"),
        ("PIO_PER", REQUIRED_PORTA_MASK, "must enable PIO control for PA22/PA24"),
        ("PIO_OER", REQUIRED_PORTA_MASK, "must output-enable PA22/PA24"),
        ("PIO_ODR", 0, "must not input-enable PA22/PA24"),
        ("PIO_ODSR", 0, "must initialize PA22/PA24 output latches low"),
    )
    for register, expected_bits, invariant in checks:
        if register not in values:
            continue
        actual_bits = values[register] & REQUIRED_PORTA_MASK
        if actual_bits != expected_bits:
            check.fail(
                PIO_C_PATH,
                f"PORTA {register} {invariant}",
                f"mask={values[register]:#010x} ({expressions[register]}), "
                f"PA22/PA24 bits={actual_bits:#010x}",
            )


def logical_macro_lines(text: str) -> list[str]:
    logical: list[str] = []
    current = ""
    for line in text.splitlines():
        stripped = line.rstrip()
        current += stripped[:-1] if stripped.endswith("\\") else stripped
        if stripped.endswith("\\"):
            current += " "
        else:
            logical.append(current)
            current = ""
    if current:
        logical.append(current)
    return logical


def macro_definitions(text: str) -> dict[str, list[tuple[bool, str]]]:
    macros: dict[str, list[tuple[bool, str]]] = {}
    pattern = re.compile(r"^\s*#\s*define\s+([A-Za-z_]\w*)(\s*\(\s*\))?\s+(.+?)\s*$")
    for line in logical_macro_lines(strip_comments(text)):
        match = pattern.match(line)
        if match:
            macros.setdefault(match.group(1), []).append(
                (match.group(2) is not None, match.group(3).strip())
            )
    return macros


def one_macro(
    check: Verification,
    macros: dict[str, list[tuple[bool, str]]],
    name: str,
    function_like: bool,
) -> str | None:
    definitions = macros.get(name, [])
    if len(definitions) != 1:
        check.fail(
            PIO_H_PATH,
            f"{name} must have exactly one generated macro definition",
            f"found={len(definitions)}",
        )
        return None
    actual_function_like, replacement = definitions[0]
    if actual_function_like != function_like:
        check.fail(
            PIO_H_PATH,
            f"{name} macro form must be {'function-like' if function_like else 'object-like'}",
            f"replacement={replacement!r}",
        )
        return None
    return replacement


def strip_wrapping_parentheses(expression: str) -> str:
    expression = expression.strip()
    while expression.startswith("("):
        try:
            close = matching_delimiter(expression, 0, "(", ")")
        except ValueError:
            break
        if close != len(expression) - 1:
            break
        expression = expression[1:-1].strip()
    return expression


def verify_write_macro(
    check: Verification, name: str, replacement: str, register: str, operator: str, bit: int
) -> None:
    replacement = strip_wrapping_parentheses(replacement)
    pattern = re.compile(
        rf"^PIOA_REGS\s*->\s*{re.escape(register)}\s*{re.escape(operator)}\s*(.+?)\s*$"
    )
    match = pattern.match(replacement)
    if match is None:
        check.fail(
            PIO_H_PATH,
            f"{name} must operate on PIOA {register} with {operator}",
            f"replacement={replacement!r}",
        )
        return
    expression = match.group(1).strip()
    try:
        value = c_integer(expression)
    except (SyntaxError, ValueError, TypeError) as exc:
        check.fail(
            PIO_H_PATH,
            f"{name} mask must be a parseable integer expression",
            f"expression={expression!r}, error={exc}",
        )
        return
    expected = 1 << bit
    if value != expected:
        check.fail(
            PIO_H_PATH,
            f"{name} must address only PIOA bit {bit}",
            f"mask={value:#010x}, expected={expected:#010x}",
        )


def verify_get_macro(check: Verification, name: str, replacement: str, bit: int) -> None:
    compact = re.sub(r"\s+", "", replacement)
    register_ok = "PIOA_REGS->PIO_PDSR" in compact
    shift_match = re.search(r">>(\d+)(?:[uUlL]+)?", compact)
    mask_match = re.search(r"&(?:0x0*1|1)(?:[uUlL]+)?", compact, flags=re.IGNORECASE)
    shift = int(shift_match.group(1)) if shift_match else None
    if not register_ok or shift != bit or mask_match is None:
        check.fail(
            PIO_H_PATH,
            f"{name} must read PIOA PDSR bit {bit}",
            f"replacement={replacement!r}",
        )


def verify_pin_macros(check: Verification) -> None:
    text = check.read_text(PIO_H_PATH)
    if text is None:
        return
    macros = macro_definitions(text)
    operations = {
        "Set": ("PIO_SODR", "="),
        "Clear": ("PIO_CODR", "="),
        "Toggle": ("PIO_ODSR", "^="),
        "OutputEnable": ("PIO_OER", "="),
        "InputEnable": ("PIO_ODR", "="),
    }
    for pin_id, (prefix, bit) in PIN_CONTRACT.items():
        for suffix, (register, operator) in operations.items():
            name = f"{prefix}_{suffix}"
            replacement = one_macro(check, macros, name, True)
            if replacement is not None:
                verify_write_macro(check, name, replacement, register, operator, bit)

        get_name = f"{prefix}_Get"
        replacement = one_macro(check, macros, get_name, True)
        if replacement is not None:
            verify_get_macro(check, get_name, replacement, bit)

        pin_name = f"{prefix}_PIN"
        replacement = one_macro(check, macros, pin_name, False)
        if replacement is not None:
            normalized = re.sub(r"[\s()]", "", replacement)
            expected = f"PIO_PIN_{pin_id}"
            if normalized != expected:
                check.fail(
                    PIO_H_PATH,
                    f"{pin_name} must map to {expected}",
                    f"replacement={replacement!r}",
                )


def find_if_blocks(text: str) -> list[tuple[str, str]]:
    blocks: list[tuple[str, str]] = []
    for match in re.finditer(r"\bif\s*\(", text):
        open_paren = text.find("(", match.start())
        try:
            close_paren = matching_delimiter(text, open_paren, "(", ")")
        except ValueError:
            continue
        open_brace = close_paren + 1
        while open_brace < len(text) and text[open_brace].isspace():
            open_brace += 1
        if open_brace >= len(text) or text[open_brace] != "{":
            continue
        try:
            close_brace = matching_delimiter(text, open_brace, "{", "}")
        except ValueError:
            continue
        blocks.append((text[open_paren + 1 : close_paren], text[open_brace + 1 : close_brace]))
    return blocks


def verify_usart(check: Verification) -> None:
    text = check.read_text(USART_PATH)
    if text is None:
        return
    cleaned = strip_comments(text)
    signatures = {
        "USART1_UartCommRxReadyHook": r"\s*void\s*",
        "USART1_UartCommErrorHook": r"\s*uint32_t(?:\s+[A-Za-z_]\w*)?\s*",
    }
    for name, parameters in signatures.items():
        pattern = re.compile(
            rf"\bbool\s+(?P<prefix>[^;{{}}]*?)\b{re.escape(name)}\s*"
            rf"\((?P<parameters>[^)]*)\)\s*\{{"
        )
        definitions = list(pattern.finditer(cleaned))
        if len(definitions) != 1:
            check.fail(
                USART_PATH,
                f"{name} must have exactly one bool definition",
                f"found={len(definitions)}",
            )
            continue
        definition = definitions[0]
        if re.search(r"\bweak\b", definition.group("prefix")) is None:
            check.fail(
                USART_PATH,
                f"{name} definition must retain the weak attribute",
                f"declaration={definition.group(0)!r}",
            )
        if re.fullmatch(parameters, definition.group("parameters")) is None:
            check.fail(
                USART_PATH,
                f"{name} must retain its generated hook signature",
                f"parameters={definition.group('parameters')!r}",
            )

    isr_bodies = function_bodies(cleaned, "USART1_InterruptHandler")
    if len(isr_bodies) != 1:
        check.fail(
            USART_PATH,
            "USART1_InterruptHandler must have exactly one definition",
            f"found={len(isr_bodies)}",
        )
        return
    if_blocks = find_if_blocks(isr_bodies[0])
    error_hook_re = re.compile(r"\bUSART1_UartCommErrorHook\s*\(\s*errorStatus\s*\)")
    error_path = any(
        "errorStatus" in condition
        and re.search(r"!=\s*0(?:x0+)?[uUlL]*\b", condition)
        and error_hook_re.search(block)
        for condition, block in if_blocks
    )
    if not error_path:
        check.fail(
            USART_PATH,
            "USART1 ISR error-status path must call USART1_UartCommErrorHook(errorStatus)",
            "no matching guarded call found",
        )

    rx_hook_re = re.compile(r"\bUSART1_UartCommRxReadyHook\s*\(\s*\)")
    rx_path = any(
        "US_CSR" in condition
        and "US_CSR_USART_RXRDY_Msk" in condition
        and rx_hook_re.search(block)
        for condition, block in if_blocks
    )
    if not rx_path:
        check.fail(
            USART_PATH,
            "USART1 ISR RX-ready path must call USART1_UartCommRxReadyHook()",
            "no matching guarded call found",
        )


def verify_nvic(check: Verification) -> None:
    text = check.read_text(NVIC_PATH)
    if text is None:
        return
    bodies = function_bodies(strip_comments(text), "NVIC_Initialize")
    if len(bodies) != 1:
        check.fail(
            NVIC_PATH,
            "NVIC_Initialize must have exactly one definition",
            f"found={len(bodies)}",
        )
        return
    matches = list(
        re.finditer(
            r"\bNVIC_SetPriority\s*\(\s*USART1_IRQn\s*,\s*([^,)]+)\s*\)",
            bodies[0],
        )
    )
    if len(matches) != 1:
        check.fail(
            NVIC_PATH,
            "NVIC_Initialize must set USART1_IRQn priority exactly once",
            f"found={len(matches)}",
        )
        return
    expression = matches[0].group(1).strip()
    try:
        priority = c_integer(expression)
    except (SyntaxError, ValueError, TypeError) as exc:
        check.fail(
            NVIC_PATH,
            "USART1 IRQ priority must be a parseable integer",
            f"expression={expression!r}, error={exc}",
        )
        return
    if priority != 7:
        check.fail(
            NVIC_PATH,
            "USART1_IRQn priority must remain 7",
            f"actual={priority}",
        )


def verify_initialization_order(check: Verification) -> None:
    text = check.read_text(INIT_PATH)
    if text is None:
        return
    bodies = function_bodies(strip_comments(text), "SYS_Initialize")
    if len(bodies) != 1:
        check.fail(
            INIT_PATH,
            "SYS_Initialize must have exactly one definition",
            f"found={len(bodies)}",
        )
        return
    body = bodies[0]
    pio_calls = list(re.finditer(r"\bPIO_Initialize\s*\(\s*\)\s*;", body))
    if len(pio_calls) != 1:
        check.fail(
            INIT_PATH,
            "SYS_Initialize must call PIO_Initialize exactly once",
            f"found={len(pio_calls)}",
        )
        return
    task_creation = re.search(
        r"\b(?:xTaskCreate(?:Static)?|vTaskStartScheduler|SYS_Tasks)\s*\(", body
    )
    if task_creation is not None and task_creation.start() < pio_calls[0].start():
        check.fail(
            INIT_PATH,
            "PIO_Initialize must run before task creation or scheduler startup",
            f"task API appears at offset {task_creation.start()}, PIO call at {pio_calls[0].start()}",
        )


def main() -> int:
    check = Verification()
    verify_csv(check)
    verify_pio_initialization(check)
    verify_pin_macros(check)
    verify_usart(check)
    verify_nvic(check)
    verify_initialization_order(check)

    if check.failures:
        for failure in check.failures:
            print(failure)
        print(f"FAIL Harmony CSP contract ({len(check.failures)} invariants)")
        return 1
    print("PASS Harmony CSP contract")
    return 0


if __name__ == "__main__":
    sys.exit(main())
