from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from tools.verify_csp_cutover import (
    APPLICATION_CSP_SOURCES,
    PUBLIC_CSP_HEADERS,
    verify_repository,
)


OPU_SOURCE = r'''
#include <csp/sam_csp_runtime.h>
static void TcTask(void *p) { (void)p; }
static void AdcTask(void *p) { (void)p; }
static void TaskCreate(void)
{
    xTaskCreate(TcTask, "tc", 1, 0, 0, 0);
    xTaskCreate(AdcTask, "adc", 1, 0, 0, 0);
}
void OpuTask(void *p)
{
    (void)p;
    HpSolValve_Init();
    StateMachine_Init();
    TaskCreate();
    (void)SamCspRuntime_Init();
}
'''


DEBUG_SOURCE = r'''
static UBaseType_t cspStackWatermark(TaskHandle_t handle)
{
    if (handle == NULL) {
        return 0U;
    }
    return uxTaskGetStackHighWaterMark(handle);
}
static int testCspFunc(int argc, char *argv[])
{
    sam_csp_runtime_status_t runtime;
    csp_rs485_health_t link;
    sam_csp_service_counters_t service;
    TaskHandle_t router;
    TaskHandle_t service_task;
    TaskHandle_t link_task;
    (void)argc; (void)argv;
    SamCspRuntime_GetStatus(&runtime);
    csp_rs485_link_get_health(&link);
    sam_csp_service_get_counters(&service);
    router = SamCspRuntime_GetRouterTaskHandle();
    service_task = SamCspRuntime_GetServiceTaskHandle();
    link_task = xTaskGetHandle("csp-rs485");
    printf("%d %d %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u",
        runtime.ready, runtime.init_code, link.state, link.last_error,
        link.uart_errors, link.dma_errors, link.tx_timeouts, link.tx_failures,
        link.protocol_errors, link.stream_dropped_bytes, link.stream_high_watermark,
        link.stream_discontinuities, link.recovery_attempts, link.recovery_successes,
        link.recovery_failures, service.malformed_packets, service.allocation_failures,
        service.send_failures, service.rejected_peers, service.dropped_ports,
        xPortGetMinimumEverFreeHeapSize(), cspStackWatermark(router),
        cspStackWatermark(service_task), cspStackWatermark(link_task));
    return 0;
}
static void registerCommands(void)
{
    UsrCmdSet("csp", testCspFunc, "CSP status", 'N', "");
    UsrCmdSet("tc", testTcFunc, "TC status", 'N', "");
}
'''


PORT_SOURCE = r'''
bool USART1_UartCommRxReadyHook(void) { return true; }
bool USART1_UartCommErrorHook(uint32_t status) { (void)status; return true; }
'''


class CutoverFixture:
    def __init__(self, root: Path) -> None:
        self.root = root
        self.write("src/opu_task.c", OPU_SOURCE)
        self.write("src/dbg_task.c", DEBUG_SOURCE)
        self.write("src/sam_ctl.h", "typedef unsigned int UInt32;\n")
        for source in APPLICATION_CSP_SOURCES:
            body = PORT_SOURCE if source.endswith("samv71_rs485_port.c") else "void placeholder(void) {}\n"
            self.write(f"sam_ctl.X/{source}", body)
        for header in PUBLIC_CSP_HEADERS:
            self.write(f"sam_ctl.X/{header}", "#pragma once\n")
        self._write_graphs()

    def write(self, relative: str, text: str) -> None:
        path = self.root / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text, encoding="utf-8")

    def read(self, relative: str) -> str:
        return (self.root / relative).read_text(encoding="utf-8")

    def replace(self, relative: str, old: str, new: str) -> None:
        text = self.read(relative)
        if old not in text:
            raise AssertionError(f"fixture mutation target absent: {old!r}")
        self.write(relative, text.replace(old, new, 1))

    def _write_graphs(self) -> None:
        items = "\n".join(
            f"<item><itemPath>{path}</itemPath></item>"
            for path in (*APPLICATION_CSP_SOURCES, *PUBLIC_CSP_HEADERS)
        )
        self.write("sam_ctl.X/nbproject/configurations.xml", f"<configuration>{items}</configuration>\n")

        source_list = " ".join(APPLICATION_CSP_SOURCES)
        rules = []
        for image in ("debug", "production"):
            for source in APPLICATION_CSP_SOURCES:
                stem = Path(source).stem
                rules.append(
                    f"${{OBJECTDIR}}/{image}/{stem}.o: {source}\n"
                    f"\t$(CC) -c {source} -o $@"
                )
        self.write(
            "sam_ctl.X/nbproject/Makefile-default.mk",
            f"SOURCEFILES={source_list}\n" + "\n".join(rules) + "\n",
        )
        cmake_sources = "\n".join(
            f'    "${{CMAKE_CURRENT_SOURCE_DIR}}/../../../sam_ctl.X/{source}"'
            for source in APPLICATION_CSP_SOURCES
        )
        self.write(
            "cmake/sam_ctl/default/.generated/file.cmake",
            f"set(SOURCE_FILES\n{cmake_sources}\n)\n",
        )


class VerifyCspCutoverTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.fixture = CutoverFixture(Path(self.temporary.name))

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def errors(self) -> list[str]:
        return verify_repository(self.fixture.root)

    def assert_error_contains(self, fragment: str) -> None:
        errors = self.errors()
        self.assertTrue(
            any(fragment in error for error in errors),
            f"expected {fragment!r} in {errors!r}",
        )

    def test_accepts_complete_atomic_cutover(self) -> None:
        self.assertEqual([], self.errors())

    def test_rejects_legacy_token_hidden_in_another_scoped_source(self) -> None:
        self.fixture.write("src/unrelated.c", "void f(void) { UartComm_Init(); }\n")
        self.assert_error_contains("legacy transport token")

    def test_rejects_source_missing_from_each_production_graph(self) -> None:
        source = APPLICATION_CSP_SOURCES[0]
        mutations = (
            ("sam_ctl.X/nbproject/configurations.xml", f"<item><itemPath>{source}</itemPath></item>", ""),
            ("sam_ctl.X/nbproject/Makefile-default.mk", f"SOURCEFILES={' '.join(APPLICATION_CSP_SOURCES)}", "SOURCEFILES="),
            ("cmake/sam_ctl/default/.generated/file.cmake", f"../../../sam_ctl.X/{source}", "../../../sam_ctl.X/removed.c"),
        )
        for path, old, new in mutations:
            with self.subTest(path=path):
                with tempfile.TemporaryDirectory() as directory:
                    fixture = CutoverFixture(Path(directory))
                    fixture.replace(path, old, new)
                    errors = verify_repository(fixture.root)
                    self.assertTrue(any("missing" in error or "lacks" in error for error in errors), errors)

    def test_rejects_public_header_missing_from_mplab_graph(self) -> None:
        header = PUBLIC_CSP_HEADERS[-1]
        self.fixture.replace(
            "sam_ctl.X/nbproject/configurations.xml",
            f"<item><itemPath>{header}</itemPath></item>",
            "",
        )
        self.assert_error_contains("public CSP header missing from MPLAB XML")

    def test_rejects_both_legacy_and_csp_hook_owners(self) -> None:
        self.fixture.write(
            "sam_ctl.X/iGRVT50/source/uartcomm.c",
            "bool USART1_UartCommRxReadyHook(void) { return true; }\n",
        )
        self.assert_error_contains("legacy file remains")

    def test_rejects_stale_cmake_only_legacy_source(self) -> None:
        path = "cmake/sam_ctl/default/.generated/file.cmake"
        self.fixture.write(path, self.fixture.read(path) + "iGRVT50/source/uartcomm.c\n")
        self.assert_error_contains("legacy source remains in CMake mirror")

    def test_rejects_wrong_opu_startup_order(self) -> None:
        self.fixture.replace(
            "src/opu_task.c",
            "StateMachine_Init();\n    TaskCreate();",
            "TaskCreate();\n    StateMachine_Init();",
        )
        self.assert_error_contains("OpuTask startup order")

    def test_rejects_retained_raw_rs485_command(self) -> None:
        self.fixture.replace(
            "src/dbg_task.c",
            'UsrCmdSet("tc", testTcFunc, "TC status", \'N\', "");',
            'UsrCmdSet("rs485", testTcFunc, "raw", \'N\', "");',
        )
        self.assert_error_contains("raw rs485 debug command")

    def test_rejects_incomplete_csp_diagnostics(self) -> None:
        self.fixture.replace("src/dbg_task.c", "link.tx_failures", "link.tx_timeouts")
        self.assert_error_contains("csp diagnostics missing link field: tx_failures")

    def test_comments_and_strings_do_not_satisfy_runtime_init_call(self) -> None:
        self.fixture.replace(
            "src/opu_task.c",
            "(void)SamCspRuntime_Init();",
            'puts("SamCspRuntime_Init()"); /* SamCspRuntime_Init(); */',
        )
        self.assert_error_contains("OpuTask must call SamCspRuntime_Init exactly once")

    def test_rejects_null_unsafe_stack_watermark_read(self) -> None:
        self.fixture.replace(
            "src/dbg_task.c",
            "if (handle == NULL) {\n        return 0U;\n    }\n    ",
            "",
        )
        self.assert_error_contains("stack high-watermark inspection is not null-safe")


if __name__ == "__main__":
    unittest.main()
