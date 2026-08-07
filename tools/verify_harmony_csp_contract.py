#!/usr/bin/env python3
"""Verify the generated Harmony contract required by the CSP RS485 port."""

from __future__ import annotations

import ast
import csv
import re
import sys
from collections.abc import Callable
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
CSV_PATH = ROOT / "src/config/default/pin_configurations.csv"
PIO_C_PATH = ROOT / "src/config/default/peripheral/pio/plib_pio.c"
PIO_H_PATH = ROOT / "src/config/default/peripheral/pio/plib_pio.h"
USART_PATH = ROOT / "src/config/default/peripheral/usart/plib_usart1.c"
NVIC_PATH = ROOT / "src/config/default/peripheral/nvic/plib_nvic.c"
INIT_PATH = ROOT / "src/config/default/initialization.c"
MAIN_PATH = ROOT / "src/main.c"

PIN_CONTRACT = {
    "PA22": ("UART1_DE", 22),
    "PA24": ("UART1_nRE", 24),
}
REQUIRED_PORTA_MASK = (1 << 22) | (1 << 24)
TASK_START_APIS = (
    "xTaskCreate",
    "xTaskCreateStatic",
    "xTaskCreateRestricted",
    "xTaskCreateRestrictedStatic",
    "MPU_xTaskCreate",
    "MPU_xTaskCreateStatic",
    "vTaskStartScheduler",
    "MPU_vTaskStartScheduler",
    "SYS_Tasks",
)
TASK_START_PATTERN = re.compile(
    rf"\b(?:{'|'.join(re.escape(name) for name in TASK_START_APIS)})\s*\("
)


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


def strip_inactive_if_zero(text: str) -> str:
    output: list[str] = []
    frames: list[tuple[bool, bool]] = []
    active = True
    directive_re = re.compile(
        r"^[ \t]*#[ \t]*(if|ifdef|ifndef|elif|else|endif)\b(.*)$"
    )

    def blank(line: str) -> str:
        return re.sub(r"[^\r\n]", " ", line)

    def literal_zero(expression: str) -> bool:
        expression = strip_comments(expression).strip()
        while expression.startswith("(") and expression.endswith(")"):
            expression = expression[1:-1].strip()
        return re.fullmatch(r"0[uUlL]*", expression) is not None

    for line in text.splitlines(keepends=True):
        directive = directive_re.match(line.rstrip("\r\n"))
        if directive is None:
            output.append(line if active else blank(line))
            continue

        name, argument = directive.groups()
        if name in ("if", "ifdef", "ifndef"):
            known_false = name == "if" and literal_zero(argument)
            frames.append((active, known_false))
            active = active and not known_false
        elif name == "elif" and frames:
            parent_active, previous_known_false = frames[-1]
            known_false = previous_known_false and literal_zero(argument)
            frames[-1] = (parent_active, known_false)
            active = parent_active and not known_false
        elif name == "else" and frames:
            parent_active, previous_known_false = frames[-1]
            active = parent_active
            frames[-1] = (parent_active, False)
        elif name == "endif" and frames:
            parent_active, _known_false = frames.pop()
            active = parent_active
        output.append(blank(line))
    return "".join(output)


def c_code_only(text: str) -> str:
    characters = list(strip_inactive_if_zero(text))
    state = "code"
    index = 0
    while index < len(characters):
        character = characters[index]
        following = characters[index + 1] if index + 1 < len(characters) else ""
        if state == "code":
            if character == "/" and following == "/":
                characters[index] = characters[index + 1] = " "
                state = "line-comment"
                index += 2
                continue
            if character == "/" and following == "*":
                characters[index] = characters[index + 1] = " "
                state = "block-comment"
                index += 2
                continue
            if character in ('"', "'"):
                characters[index] = " "
                state = "string" if character == '"' else "character"
        elif state == "line-comment":
            if character in "\r\n":
                state = "code"
            else:
                characters[index] = " "
        elif state == "block-comment":
            if character == "*" and following == "/":
                characters[index] = characters[index + 1] = " "
                state = "code"
                index += 2
                continue
            if character not in "\r\n":
                characters[index] = " "
        else:
            quote = '"' if state == "string" else "'"
            if character == "\\":
                characters[index] = " "
                if index + 1 < len(characters):
                    if characters[index + 1] not in "\r\n":
                        characters[index + 1] = " "
                    index += 2
                    continue
            elif character == quote:
                characters[index] = " "
                state = "code"
            elif character not in "\r\n":
                characters[index] = " "
        index += 1
    return "".join(characters)


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
            reader = csv.DictReader(source, strict=True)
            required_columns = {"Pin ID", "Custom Name", "Function", "Direction", "Latch"}
            fieldnames = reader.fieldnames
            if fieldnames is not None and len(fieldnames) != len(set(fieldnames)):
                check.fail(
                    CSV_PATH,
                    "CSV header names must be unique",
                    f"columns={fieldnames!r}",
                )
                return
            if fieldnames is None or not required_columns.issubset(fieldnames):
                check.fail(
                    CSV_PATH,
                    "CSV must expose the Harmony pin-contract columns",
                    f"columns={fieldnames!r}",
                )
                return
            rows = list(reader)
    except (OSError, UnicodeError, csv.Error) as exc:
        check.fail(CSV_PATH, "CSV must be present, readable, and parseable", str(exc))
        return

    for line_number, row in enumerate(rows, start=2):
        if None in row or any(row.get(fieldname) is None for fieldname in fieldnames):
            check.fail(
                CSV_PATH,
                "CSV rows must contain exactly one field per header",
                f"line={line_number}, row={row!r}",
            )
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


def split_top_level_operator(expression: str, operator: str) -> tuple[str, str] | None:
    depth = 0
    split_at: int | None = None
    index = 0
    while index < len(expression):
        character = expression[index]
        if character == "(":
            depth += 1
        elif character == ")":
            depth -= 1
            if depth < 0:
                return None
        elif depth == 0 and expression.startswith(operator, index):
            if split_at is not None:
                return None
            split_at = index
            index += len(operator) - 1
        index += 1
    if depth != 0 or split_at is None:
        return None
    return expression[:split_at], expression[split_at + len(operator) :]


def c_integer_literal(expression: str) -> int | None:
    match = re.fullmatch(
        r"(?i)(0x[0-9a-f]+|\d+)[ul]*", strip_wrapping_parentheses(expression)
    )
    if match is None:
        return None
    literal = match.group(1)
    return int(literal, 16 if literal.lower().startswith("0x") else 10)


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
    expression = re.sub(
        r"\(\s*(?:u?int(?:8|16|32|64)_t|unsigned(?:\s+(?:char|short|int|long))?|size_t)\s*\)",
        "",
        replacement,
    )
    expression = strip_wrapping_parentheses(expression)
    masked = split_top_level_operator(expression, "&")
    shifted = (
        split_top_level_operator(strip_wrapping_parentheses(masked[0]), ">>")
        if masked is not None
        else None
    )
    register_ok = (
        shifted is not None
        and re.fullmatch(
            r"PIOA_REGS\s*->\s*PIO_PDSR", strip_wrapping_parentheses(shifted[0])
        )
        is not None
    )
    shift = c_integer_literal(shifted[1]) if shifted is not None else None
    mask = c_integer_literal(masked[1]) if masked is not None else None
    if not register_ok or shift != bit or mask != 1:
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


def function_call_arguments(text: str, name: str) -> list[str]:
    arguments: list[str] = []
    pattern = re.compile(rf"\b{re.escape(name)}\s*\(")
    for match in pattern.finditer(text):
        open_paren = text.find("(", match.start())
        try:
            close_paren = matching_delimiter(text, open_paren, "(", ")")
        except ValueError:
            continue
        arguments.append(text[open_paren + 1 : close_paren])
    return arguments


def split_top_level_arguments(arguments: str) -> list[str] | None:
    parts: list[str] = []
    start = 0
    depth = 0
    for index, character in enumerate(arguments):
        if character == "(":
            depth += 1
        elif character == ")":
            depth -= 1
            if depth < 0:
                return None
        elif character == "," and depth == 0:
            parts.append(arguments[start:index].strip())
            start = index + 1
    if depth != 0:
        return None
    parts.append(arguments[start:].strip())
    return parts


def hook_false_condition(condition: str, hook_call: re.Pattern[str]) -> bool:
    condition = strip_wrapping_parentheses(condition)
    if condition.startswith("!"):
        return hook_call.fullmatch(strip_wrapping_parentheses(condition[1:])) is not None

    for operator, false_literal in (("==", False), ("!=", True)):
        comparison = split_top_level_operator(condition, operator)
        if comparison is None:
            continue
        left = strip_wrapping_parentheses(comparison[0])
        right = strip_wrapping_parentheses(comparison[1])
        if hook_call.fullmatch(left) is not None:
            literal = right
        elif hook_call.fullmatch(right) is not None:
            literal = left
        else:
            return False
        if false_literal:
            return re.fullmatch(r"(?:true|1[uUlL]*)", literal) is not None
        return re.fullmatch(r"(?:false|0[uUlL]*)", literal) is not None
    return False


def condition_is_nonzero(
    condition: str, operand_matches: Callable[[str], bool]
) -> bool:
    condition = strip_wrapping_parentheses(condition)
    comparison = split_top_level_operator(condition, "!=")
    if comparison is None:
        return operand_matches(condition)
    left = strip_wrapping_parentheses(comparison[0])
    right = strip_wrapping_parentheses(comparison[1])
    if c_integer_literal(left) == 0:
        return operand_matches(right)
    if c_integer_literal(right) == 0:
        return operand_matches(left)
    return False


def error_status_operand(expression: str) -> bool:
    return strip_wrapping_parentheses(expression) == "errorStatus"


def rx_ready_operand(expression: str) -> bool:
    masked = split_top_level_operator(strip_wrapping_parentheses(expression), "&")
    if masked is None:
        return False
    operands = {
        re.sub(r"\s+", "", strip_wrapping_parentheses(masked[0])),
        re.sub(r"\s+", "", strip_wrapping_parentheses(masked[1])),
    }
    return operands == {"USART1_REGS->US_CSR", "US_CSR_USART_RXRDY_Msk"}


def hook_guards_fallback(
    outer_block: str,
    hook_call: re.Pattern[str],
    fallback_call: re.Pattern[str],
) -> bool:
    guarded_bodies = [
        block
        for condition, block in find_if_blocks(outer_block)
        if hook_false_condition(condition, hook_call)
    ]
    all_fallbacks = len(fallback_call.findall(outer_block))
    guarded_fallbacks = sum(
        len(fallback_call.findall(block)) for block in guarded_bodies
    )
    return all_fallbacks == 1 and guarded_fallbacks == 1


def verify_usart(check: Verification) -> None:
    text = check.read_text(USART_PATH)
    if text is None:
        return
    cleaned = c_code_only(text)
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
    error_hook_re = re.compile(r"USART1_UartCommErrorHook\s*\(\s*errorStatus\s*\)")
    error_fallback_re = re.compile(r"\bUSART1_ErrorClear\s*\(")
    error_path = any(
        condition_is_nonzero(condition, error_status_operand)
        and hook_guards_fallback(block, error_hook_re, error_fallback_re)
        for condition, block in if_blocks
    )
    if not error_path:
        check.fail(
            USART_PATH,
            "USART1 ISR error-status path must let a false USART1_UartCommErrorHook result guard the legacy fallback",
            "no matching guarded fallback found",
        )

    rx_hook_re = re.compile(r"USART1_UartCommRxReadyHook\s*\(\s*\)")
    rx_fallback_re = re.compile(r"\bUSART1_ISR_RX_Handler\s*\(")
    rx_path = any(
        condition_is_nonzero(condition, rx_ready_operand)
        and hook_guards_fallback(block, rx_hook_re, rx_fallback_re)
        for condition, block in if_blocks
    )
    if not rx_path:
        check.fail(
            USART_PATH,
            "USART1 ISR RX-ready path must let a false USART1_UartCommRxReadyHook result guard the legacy fallback",
            "no matching guarded fallback found",
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
    priorities: list[str] = []
    for arguments in function_call_arguments(bodies[0], "NVIC_SetPriority"):
        parts = split_top_level_arguments(arguments)
        if (
            parts is not None
            and len(parts) == 2
            and strip_wrapping_parentheses(parts[0]) == "USART1_IRQn"
        ):
            priorities.append(parts[1])
    if len(priorities) != 1:
        check.fail(
            NVIC_PATH,
            "NVIC_Initialize must set USART1_IRQn priority exactly once",
            f"found={len(priorities)}",
        )
        return
    expression = priorities[0]
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
    init_text = check.read_text(INIT_PATH)
    if init_text is None:
        return
    bodies = function_bodies(c_code_only(init_text), "SYS_Initialize")
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
    task_creation = TASK_START_PATTERN.search(body)
    if task_creation is not None and task_creation.start() < pio_calls[0].start():
        check.fail(
            INIT_PATH,
            "PIO_Initialize must run before task creation or scheduler startup",
            f"task API appears at offset {task_creation.start()}, PIO call at {pio_calls[0].start()}",
        )

    main_text = check.read_text(MAIN_PATH)
    if main_text is None:
        return
    main_bodies = function_bodies(c_code_only(main_text), "main")
    if len(main_bodies) != 1:
        check.fail(
            MAIN_PATH,
            "main must have exactly one definition",
            f"found={len(main_bodies)}",
        )
        return
    main_body = main_bodies[0]
    system_initialization = list(re.finditer(r"\bSYS_Initialize\s*\(", main_body))
    if len(system_initialization) != 1:
        check.fail(
            MAIN_PATH,
            "main must call SYS_Initialize exactly once",
            f"found={len(system_initialization)}",
        )
        return
    early_task = next(
        (
            match
            for match in TASK_START_PATTERN.finditer(main_body)
            if match.start() < system_initialization[0].start()
        ),
        None,
    )
    if early_task is not None:
        check.fail(
            MAIN_PATH,
            "SYS_Initialize must run before task creation or scheduler startup",
            "task API appears at offset "
            f"{early_task.start()}, SYS_Initialize call at {system_initialization[0].start()}",
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
