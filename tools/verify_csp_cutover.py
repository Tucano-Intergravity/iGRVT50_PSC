#!/usr/bin/env python3
"""Verify the atomic legacy USART1 to CSP application cutover."""

from __future__ import annotations

import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path


APPLICATION_CSP_SOURCES = (
    "iGRVT50/source/csp/sam_csp_codec.c",
    "iGRVT50/source/csp/sam_csp_domain.c",
    "iGRVT50/source/csp/sam_csp_runtime.c",
    "iGRVT50/source/csp/sam_csp_service.c",
    "iGRVT50/source/csp/samv71_rs485_port.c",
)

PUBLIC_CSP_HEADERS = (
    "iGRVT50/header/csp/sam_csp_config.h",
    "iGRVT50/header/csp/sam_csp_domain.h",
    "iGRVT50/header/csp/sam_csp_protocol.h",
    "iGRVT50/header/csp/sam_csp_runtime.h",
    "iGRVT50/header/csp/sam_csp_service.h",
    "iGRVT50/header/csp/samv71_rs485_port.h",
)

LEGACY_FILES = (
    "sam_ctl.X/iGRVT50/header/uartcomm.h",
    "sam_ctl.X/iGRVT50/source/uartcomm.c",
    "src/rs422_func.c",
)

LEGACY_PATTERN = re.compile(
    r"UartComm_|uartcomm\.h|\bRsTask\b|RSTASK_NOTIFY|RS422_Init|"
    r"RS485_SetTransmit|usRs422Loop|\$iGRVT50|\bTCMD_|RS_TASK_PRIORITY"
)

LINK_FIELDS = (
    "state",
    "last_error",
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

SERVICE_FIELDS = (
    "malformed_packets",
    "allocation_failures",
    "send_failures",
    "rejected_peers",
    "dropped_ports",
)


def _read(path: Path, errors: list[str]) -> str:
    try:
        return path.read_text(encoding="utf-8")
    except (OSError, UnicodeError) as exc:
        errors.append(f"cannot read {path}: {exc}")
        return ""


def _strip_comments(text: str) -> str:
    """Remove C comments while preserving strings and source offsets."""

    output = list(text)
    index = 0
    state = "code"
    while index < len(text):
        char = text[index]
        following = text[index + 1] if index + 1 < len(text) else ""
        if state == "code":
            if char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            elif char == "/" and following == "/":
                output[index] = output[index + 1] = " "
                index += 1
                state = "line_comment"
            elif char == "/" and following == "*":
                output[index] = output[index + 1] = " "
                index += 1
                state = "block_comment"
        elif state == "string":
            if char == "\\":
                index += 1
            elif char == '"':
                state = "code"
        elif state == "char":
            if char == "\\":
                index += 1
            elif char == "'":
                state = "code"
        elif state == "line_comment":
            if char == "\n":
                state = "code"
            else:
                output[index] = " "
        elif state == "block_comment":
            if char == "*" and following == "/":
                output[index] = output[index + 1] = " "
                index += 1
                state = "code"
            elif char != "\n":
                output[index] = " "
        index += 1
    return "".join(output)


def _semantic_code(text: str) -> str:
    """Remove comments and literal contents, preserving positions and syntax."""

    stripped = _strip_comments(text)
    output = list(stripped)
    index = 0
    state = "code"
    while index < len(stripped):
        char = stripped[index]
        if state == "code":
            if char == '"':
                output[index] = " "
                state = "string"
            elif char == "'":
                output[index] = " "
                state = "char"
        else:
            if char == "\\":
                output[index] = " "
                if index + 1 < len(stripped):
                    output[index + 1] = " "
                    index += 1
            elif (state == "string" and char == '"') or (
                state == "char" and char == "'"
            ):
                output[index] = " "
                state = "code"
            elif char != "\n":
                output[index] = " "
        index += 1
    return "".join(output)


def _function_body(text: str, name: str) -> tuple[str, str] | None:
    without_comments = _strip_comments(text)
    semantic = _semantic_code(text)
    pattern = re.compile(rf"\b{re.escape(name)}\s*\([^;{{}}]*\)\s*\{{")
    matches = list(pattern.finditer(semantic))
    if len(matches) != 1:
        return None
    opening = semantic.find("{", matches[0].start())
    depth = 0
    for index in range(opening, len(semantic)):
        if semantic[index] == "{":
            depth += 1
        elif semantic[index] == "}":
            depth -= 1
            if depth == 0:
                return (
                    without_comments[opening + 1 : index],
                    semantic[opening + 1 : index],
                )
    return None


def _normalized(text: str) -> str:
    return text.replace("\\", "/")


def _verify_no_legacy(root: Path, errors: list[str]) -> None:
    for relative in LEGACY_FILES:
        if (root / relative).exists():
            errors.append(f"legacy file remains: {relative}")

    for top in (root / "src", root / "sam_ctl.X" / "iGRVT50"):
        if not top.exists():
            continue
        for path in sorted(top.rglob("*")):
            if path.suffix.lower() not in {".c", ".h"} or not path.is_file():
                continue
            text = _read(path, errors)
            match = LEGACY_PATTERN.search(text)
            if match:
                errors.append(
                    f"legacy transport token {match.group(0)!r} remains in "
                    f"{path.relative_to(root).as_posix()}"
                )


def _verify_graphs(root: Path, errors: list[str]) -> None:
    xml_path = root / "sam_ctl.X/nbproject/configurations.xml"
    make_path = root / "sam_ctl.X/nbproject/Makefile-default.mk"
    cmake_path = root / "cmake/sam_ctl/default/.generated/file.cmake"

    xml_text = _read(xml_path, errors)
    make_text = _normalized(_read(make_path, errors))
    cmake_text = _normalized(_read(cmake_path, errors))
    make_source_lines = "\n".join(
        line for line in make_text.splitlines() if line.startswith("SOURCEFILES=")
    )

    xml_items: set[str] = set()
    if xml_text:
        try:
            tree = ET.fromstring(xml_text)
            xml_items = {
                _normalized((item.text or "").strip()).removeprefix("../")
                for item in tree.iter("itemPath")
            }
        except ET.ParseError as exc:
            errors.append(f"invalid configurations.xml: {exc}")

    for source in APPLICATION_CSP_SOURCES:
        disk_path = root / "sam_ctl.X" / source
        if not disk_path.is_file():
            errors.append(f"application CSP source missing from disk: {source}")
        if source not in xml_items:
            errors.append(f"application CSP source missing from MPLAB XML: {source}")
        if source not in make_source_lines:
            errors.append(f"application CSP source missing from MPLAB Makefile: {source}")
        rule = re.compile(
            rf"(?m)^\$\{{OBJECTDIR\}}/[^\n:]*{re.escape(Path(source).stem)}\.o:"
            rf"\s+{re.escape(source)}\b"
        )
        if len(rule.findall(make_text)) != 2:
            errors.append(
                f"application CSP source lacks debug/production Makefile rules: {source}"
            )
        if source not in cmake_text:
            errors.append(f"application CSP source missing from CMake mirror: {source}")

    for header in PUBLIC_CSP_HEADERS:
        if not (root / "sam_ctl.X" / header).is_file():
            errors.append(f"public CSP header missing from disk: {header}")
        if header not in xml_items:
            errors.append(f"public CSP header missing from MPLAB XML: {header}")

    legacy_graph_paths = (
        "iGRVT50/source/uartcomm.c",
        "../src/rs422_func.c",
        "src/rs422_func.c",
    )
    for graph_name, graph_text in (
        ("MPLAB XML", _normalized(xml_text)),
        ("MPLAB Makefile", make_text),
        ("CMake mirror", cmake_text),
    ):
        for legacy in legacy_graph_paths:
            if legacy in graph_text:
                errors.append(f"legacy source remains in {graph_name}: {legacy}")


def _verify_hook_owner(root: Path, errors: list[str]) -> None:
    port_path = root / "sam_ctl.X/iGRVT50/source/csp/samv71_rs485_port.c"
    code = _semantic_code(_read(port_path, errors))
    for hook in (
        "USART1_UartCommRxReadyHook",
        "USART1_UartCommErrorHook",
    ):
        definitions = re.findall(
            rf"\bbool\s+{re.escape(hook)}\s*\([^;{{}}]*\)\s*\{{", code
        )
        if len(definitions) != 1:
            errors.append(f"expected one CSP hook definition for {hook}")


def _verify_opu(root: Path, errors: list[str]) -> None:
    text = _read(root / "src/opu_task.c", errors)
    opu = _function_body(text, "OpuTask")
    if opu is None:
        errors.append("cannot identify unique OpuTask definition")
    else:
        _, semantic = opu
        required_order = (
            "HpSolValve_Init",
            "StateMachine_Init",
            "TaskCreate",
            "SamCspRuntime_Init",
        )
        positions: list[int] = []
        for call in required_order:
            matches = list(re.finditer(rf"\b{call}\s*\(", semantic))
            if len(matches) != 1:
                errors.append(f"OpuTask must call {call} exactly once")
                positions.append(-1)
            else:
                positions.append(matches[0].start())
        if all(position >= 0 for position in positions) and positions != sorted(positions):
            errors.append(
                "OpuTask startup order must be HpSolValve_Init, StateMachine_Init, "
                "TaskCreate, SamCspRuntime_Init"
            )

    task_create = _function_body(text, "TaskCreate")
    if task_create is None:
        errors.append("cannot identify unique TaskCreate definition")
    else:
        _, semantic = task_create
        task_calls = list(re.finditer(r"\bxTaskCreate\s*\(", semantic))
        if len(task_calls) != 2:
            errors.append("TaskCreate must create exactly TcTask and AdcTask")
        if not re.search(r"\bxTaskCreate\s*\(\s*TcTask\b", semantic):
            errors.append("TaskCreate does not create TcTask")
        if not re.search(r"\bxTaskCreate\s*\(\s*AdcTask\b", semantic):
            errors.append("TaskCreate does not create AdcTask")


def _verify_stack_watermark_null_safety(text: str, errors: list[str]) -> None:
    semantic = _semantic_code(text)
    calls = list(re.finditer(r"\buxTaskGetStackHighWaterMark\s*\(", semantic))
    if not calls:
        errors.append("csp diagnostics do not read task stack high-watermarks")
        return
    safe_guard = re.search(
        r"\bif\s*\(\s*[A-Za-z_]\w*\s*(?:==|!=)\s*NULL\s*\)", semantic
    ) or re.search(
        r"[A-Za-z_]\w*\s*!=\s*NULL\s*\?\s*uxTaskGetStackHighWaterMark", semantic
    )
    if safe_guard is None:
        errors.append("stack high-watermark inspection is not null-safe")


def _verify_debug(root: Path, errors: list[str]) -> None:
    text = _read(root / "src/dbg_task.c", errors)
    without_comments = _strip_comments(text)
    registrations = re.findall(
        r"\bUsrCmdSet\s*\(\s*\"([^\"]+)\"\s*,\s*([A-Za-z_]\w*)",
        without_comments,
    )
    by_name = {name: handler for name, handler in registrations}
    if "uart" in by_name:
        errors.append("legacy uart debug command remains registered")
    if "rs485" in by_name:
        errors.append("raw rs485 debug command remains registered")
    handler_name = by_name.get("csp")
    if handler_name is None:
        errors.append("csp debug command is not registered")
        return

    handler = _function_body(text, handler_name)
    if handler is None:
        errors.append("cannot identify csp debug command handler")
        return
    literal_body, semantic_body = handler
    for call in (
        "SamCspRuntime_GetStatus",
        "csp_rs485_link_get_health",
        "sam_csp_service_get_counters",
        "xPortGetMinimumEverFreeHeapSize",
        "SamCspRuntime_GetRouterTaskHandle",
        "SamCspRuntime_GetServiceTaskHandle",
        "xTaskGetHandle",
    ):
        if not re.search(rf"\b{call}\s*\(", semantic_body):
            errors.append(f"csp diagnostics missing call: {call}")
    if not re.search(r"\bxTaskGetHandle\s*\(\s*\"csp-rs485\"\s*\)", literal_body):
        errors.append("csp diagnostics do not inspect the csp-rs485 task")
    for field in LINK_FIELDS:
        if not re.search(rf"(?:\.|->)\s*{re.escape(field)}\b", semantic_body):
            errors.append(f"csp diagnostics missing link field: {field}")
    for field in SERVICE_FIELDS:
        if not re.search(rf"(?:\.|->)\s*{re.escape(field)}\b", semantic_body):
            errors.append(f"csp diagnostics missing service field: {field}")

    _verify_stack_watermark_null_safety(text, errors)


def verify_repository(root: Path) -> list[str]:
    root = root.resolve()
    errors: list[str] = []
    _verify_no_legacy(root, errors)
    _verify_graphs(root, errors)
    _verify_hook_owner(root, errors)
    _verify_opu(root, errors)
    _verify_debug(root, errors)
    return errors


def main(argv: list[str] | None = None) -> int:
    arguments = sys.argv[1:] if argv is None else argv
    root = Path(arguments[0]) if arguments else Path(__file__).resolve().parents[1]
    errors = verify_repository(root)
    if errors:
        print("FAIL CSP USART1 cutover")
        for error in errors:
            print(f"- {error}")
        return 1
    print("PASS CSP USART1 cutover")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
