#!/usr/bin/env python3
"""Mutation tests for verify_harmony_csp_contract.py."""

from __future__ import annotations

import tempfile
import unittest
from pathlib import Path
from unittest import mock

from tools import verify_harmony_csp_contract as verifier


VALID_CSV = """Pin Number,Pin ID,Custom Name,Function,Direction,Latch,Open Drain,PIO Interrupt,Pull Up,Pull Down,Glitch/Debounce Filter,Drive Strength
37,PA22,UART1_DE,GPIO,Out,Low,No,Disabled,No,No,Disabled,Low
56,PA24,UART1_nRE,GPIO,Out,Low,No,Disabled,No,No,Disabled,Low
"""

VALID_INIT = """
void SYS_Initialize(void *data)
{
    CLOCK_Initialize();
    PIO_Initialize();
    AFEC0_Initialize();
}
"""

VALID_MAIN = """
int main(void)
{
    SYS_Initialize(NULL);
    xTaskCreate(task, "task", 128U, NULL, 1U, NULL);
    vTaskStartScheduler();
}
"""

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

VALID_USART_PREFIX = """
bool __attribute__((weak)) USART1_UartCommRxReadyHook(void)
{
    return false;
}

bool __attribute__((weak)) USART1_UartCommErrorHook(uint32_t errorStatus)
{
    (void)errorStatus;
    return false;
}
"""

VALID_USART_ISR = """
void USART1_InterruptHandler(void)
{
    uint32_t errorStatus = USART1_REGS->US_CSR & US_CSR_USART_OVRE_Msk;
    if (errorStatus != 0U)
    {
        if (USART1_UartCommErrorHook(errorStatus) == false)
        {
            USART1_ErrorClear();
        }
    }
    if ((USART1_REGS->US_CSR & US_CSR_USART_RXRDY_Msk) != 0U)
    {
        if (USART1_UartCommRxReadyHook() == false)
        {
            USART1_ISR_RX_Handler();
        }
    }
}
"""


class FixtureCase(unittest.TestCase):
    def verify_file(self, path_name: str, content: str, function) -> verifier.Verification:
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / Path(getattr(verifier, path_name)).name
            path.write_text(content, encoding="utf-8")
            with mock.patch.object(verifier, path_name, path):
                check = verifier.Verification()
                function(check)
        return check

    def verify_initialization(
        self, init_source: str, main_source: str
    ) -> verifier.Verification:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            init_path = root / "initialization.c"
            main_path = root / "main.c"
            init_path.write_text(init_source, encoding="utf-8")
            main_path.write_text(main_source, encoding="utf-8")
            with (
                mock.patch.object(verifier, "INIT_PATH", init_path),
                mock.patch.object(verifier, "MAIN_PATH", main_path, create=True),
            ):
                check = verifier.Verification()
                verifier.verify_initialization_order(check)
        return check

    def verify_usart_source(self, isr_source: str) -> verifier.Verification:
        return self.verify_file(
            "USART_PATH",
            VALID_USART_PREFIX + isr_source,
            verifier.verify_usart,
        )


class InitializationOrderTests(FixtureCase):
    def test_valid_startup_order_is_accepted(self) -> None:
        check = self.verify_initialization(VALID_INIT, VALID_MAIN)

        self.assertEqual([], check.failures)

    def test_each_task_api_before_system_initialization_is_rejected(self) -> None:
        for api in TASK_START_APIS:
            with self.subTest(api=api):
                main_source = f"""
int main(void)
{{
    {api}(NULL);
    SYS_Initialize(NULL);
}}
"""
                check = self.verify_initialization(VALID_INIT, main_source)

                self.assertTrue(check.failures, api)

    def test_duplicate_system_initialization_call_is_rejected(self) -> None:
        main_source = """
int main(void)
{
    SYS_Initialize(NULL);
    SYS_Initialize(NULL);
    xTaskCreate(NULL);
}
"""
        check = self.verify_initialization(VALID_INIT, main_source)

        self.assertTrue(check.failures)

    def test_each_task_api_before_pio_initialization_is_rejected(self) -> None:
        for api in TASK_START_APIS:
            with self.subTest(api=api):
                init_source = f"""
void SYS_Initialize(void *data)
{{
    {api}(NULL);
    PIO_Initialize();
}}
"""
                check = self.verify_initialization(init_source, VALID_MAIN)

                self.assertTrue(check.failures, api)

    def test_conditional_system_initialization_is_rejected(self) -> None:
        main_source = """
int main(void)
{
    if (ready)
    {
        SYS_Initialize(NULL);
    }
    else
    {
        xTaskCreate(NULL);
    }
}
"""
        check = self.verify_initialization(VALID_INIT, main_source)

        self.assertTrue(check.failures)

    def test_conditional_pio_initialization_is_rejected(self) -> None:
        init_source = """
void SYS_Initialize(void *data)
{
    if (ready)
    {
        PIO_Initialize();
    }
    xTaskCreate(NULL);
}
"""
        check = self.verify_initialization(init_source, VALID_MAIN)

        self.assertTrue(check.failures)


class UsartHookTests(FixtureCase):
    def test_valid_false_guarded_fallbacks_are_accepted(self) -> None:
        check = self.verify_usart_source(VALID_USART_ISR)

        self.assertEqual([], check.failures)

    def test_cast_away_error_hook_result_is_rejected(self) -> None:
        isr_source = """
void USART1_InterruptHandler(void)
{
    uint32_t errorStatus = USART1_REGS->US_CSR & US_CSR_USART_OVRE_Msk;
    if (errorStatus != 0U)
    {
        (void)USART1_UartCommErrorHook(errorStatus);
        USART1_ErrorClear();
    }
    if ((USART1_REGS->US_CSR & US_CSR_USART_RXRDY_Msk) != 0U)
    {
        if (!USART1_UartCommRxReadyHook())
        {
            USART1_ISR_RX_Handler();
        }
    }
}
"""
        check = self.verify_usart_source(isr_source)

        self.assertTrue(any("error-status" in failure for failure in check.failures))

    def test_cast_away_rx_hook_result_is_rejected(self) -> None:
        isr_source = """
void USART1_InterruptHandler(void)
{
    uint32_t errorStatus = USART1_REGS->US_CSR & US_CSR_USART_OVRE_Msk;
    if (errorStatus != 0U)
    {
        if (!USART1_UartCommErrorHook(errorStatus))
        {
            USART1_ErrorClear();
        }
    }
    if ((USART1_REGS->US_CSR & US_CSR_USART_RXRDY_Msk) != 0U)
    {
        (void)USART1_UartCommRxReadyHook();
        USART1_ISR_RX_Handler();
    }
}
"""
        check = self.verify_usart_source(isr_source)

        self.assertTrue(any("RX-ready" in failure for failure in check.failures))

    def test_true_guard_semantics_are_rejected(self) -> None:
        isr_source = VALID_USART_ISR.replace(
            "USART1_UartCommRxReadyHook() == false",
            "USART1_UartCommRxReadyHook() == true",
        )
        check = self.verify_usart_source(isr_source)

        self.assertTrue(any("RX-ready" in failure for failure in check.failures))

    def test_fallback_outside_hook_guard_is_rejected(self) -> None:
        isr_source = """
void USART1_InterruptHandler(void)
{
    uint32_t errorStatus = USART1_REGS->US_CSR & US_CSR_USART_OVRE_Msk;
    if (errorStatus != 0U)
    {
        if (!USART1_UartCommErrorHook(errorStatus))
        {
        }
        USART1_ErrorClear();
    }
    if ((USART1_REGS->US_CSR & US_CSR_USART_RXRDY_Msk) != 0U)
    {
        if (!USART1_UartCommRxReadyHook())
        {
            USART1_ISR_RX_Handler();
        }
    }
}
"""
        check = self.verify_usart_source(isr_source)

        self.assertTrue(any("error-status" in failure for failure in check.failures))

    def test_hook_name_in_string_does_not_satisfy_rx_guard(self) -> None:
        isr_source = """
void USART1_InterruptHandler(void)
{
    uint32_t errorStatus = USART1_REGS->US_CSR & US_CSR_USART_OVRE_Msk;
    if (errorStatus != 0U)
    {
        if (!USART1_UartCommErrorHook(errorStatus))
        {
            USART1_ErrorClear();
        }
    }
    if ((USART1_REGS->US_CSR & US_CSR_USART_RXRDY_Msk) != 0U)
    {
        const char *ignored = "USART1_UartCommRxReadyHook()";
        USART1_ISR_RX_Handler();
    }
}
"""
        check = self.verify_usart_source(isr_source)

        self.assertTrue(any("RX-ready" in failure for failure in check.failures))

    def test_hook_in_if_zero_branch_does_not_satisfy_rx_guard(self) -> None:
        isr_source = """
void USART1_InterruptHandler(void)
{
    uint32_t errorStatus = USART1_REGS->US_CSR & US_CSR_USART_OVRE_Msk;
    if (errorStatus != 0U)
    {
        if (!USART1_UartCommErrorHook(errorStatus))
        {
            USART1_ErrorClear();
        }
    }
    if ((USART1_REGS->US_CSR & US_CSR_USART_RXRDY_Msk) != 0U)
    {
#if 0
        if (!USART1_UartCommRxReadyHook())
        {
            USART1_ISR_RX_Handler();
        }
#endif
        USART1_ISR_RX_Handler();
    }
}
"""
        check = self.verify_usart_source(isr_source)

        self.assertTrue(any("RX-ready" in failure for failure in check.failures))

    def test_error_side_effect_outside_hook_guard_is_rejected(self) -> None:
        isr_source = """
void USART1_InterruptHandler(void)
{
    uint32_t errorStatus = USART1_REGS->US_CSR & US_CSR_USART_OVRE_Msk;
    if (errorStatus != 0U)
    {
        usart1Obj.errorStatus = (USART_ERROR)errorStatus;
        if (!USART1_UartCommErrorHook(errorStatus))
        {
            USART1_ErrorClear();
        }
    }
    if ((USART1_REGS->US_CSR & US_CSR_USART_RXRDY_Msk) != 0U)
    {
        if (!USART1_UartCommRxReadyHook())
        {
            USART1_ISR_RX_Handler();
        }
    }
}
"""
        check = self.verify_usart_source(isr_source)

        self.assertTrue(any("error-status" in failure for failure in check.failures))

    def test_rx_side_effect_outside_hook_guard_is_rejected(self) -> None:
        isr_source = """
void USART1_InterruptHandler(void)
{
    uint32_t errorStatus = USART1_REGS->US_CSR & US_CSR_USART_OVRE_Msk;
    if (errorStatus != 0U)
    {
        if (!USART1_UartCommErrorHook(errorStatus))
        {
            USART1_ErrorClear();
        }
    }
    if ((USART1_REGS->US_CSR & US_CSR_USART_RXRDY_Msk) != 0U)
    {
        usart1Obj.rxBusyStatus = false;
        if (!USART1_UartCommRxReadyHook())
        {
            USART1_ISR_RX_Handler();
        }
    }
}
"""
        check = self.verify_usart_source(isr_source)

        self.assertTrue(any("RX-ready" in failure for failure in check.failures))

    def test_inactive_zero_spellings_do_not_satisfy_rx_guard(self) -> None:
        for zero in ("00", "0x0"):
            with self.subTest(zero=zero):
                isr_source = f"""
void USART1_InterruptHandler(void)
{{
    uint32_t errorStatus = USART1_REGS->US_CSR & US_CSR_USART_OVRE_Msk;
    if (errorStatus != 0U)
    {{
        if (!USART1_UartCommErrorHook(errorStatus))
        {{
            USART1_ErrorClear();
        }}
    }}
    if ((USART1_REGS->US_CSR & US_CSR_USART_RXRDY_Msk) != 0U)
    {{
#if {zero}
        if (!USART1_UartCommRxReadyHook())
        {{
            USART1_ISR_RX_Handler();
        }}
#endif
    }}
}}
"""
                check = self.verify_usart_source(isr_source)

                self.assertTrue(
                    any("RX-ready" in failure for failure in check.failures), zero
                )

    def test_if_one_else_branch_does_not_satisfy_rx_guard(self) -> None:
        isr_source = """
void USART1_InterruptHandler(void)
{
    uint32_t errorStatus = USART1_REGS->US_CSR & US_CSR_USART_OVRE_Msk;
    if (errorStatus != 0U)
    {
        if (!USART1_UartCommErrorHook(errorStatus))
        {
            USART1_ErrorClear();
        }
    }
    if ((USART1_REGS->US_CSR & US_CSR_USART_RXRDY_Msk) != 0U)
    {
#if 1
        /* Active branch intentionally has no fallback. */
#else
        if (!USART1_UartCommRxReadyHook())
        {
            USART1_ISR_RX_Handler();
        }
#endif
    }
}
"""
        check = self.verify_usart_source(isr_source)

        self.assertTrue(any("RX-ready" in failure for failure in check.failures))

    def test_nested_inactive_else_branch_does_not_satisfy_rx_guard(self) -> None:
        isr_source = """
void USART1_InterruptHandler(void)
{
    uint32_t errorStatus = USART1_REGS->US_CSR & US_CSR_USART_OVRE_Msk;
    if (errorStatus != 0U)
    {
        if (!USART1_UartCommErrorHook(errorStatus))
        {
            USART1_ErrorClear();
        }
    }
    if ((USART1_REGS->US_CSR & US_CSR_USART_RXRDY_Msk) != 0U)
    {
#if 1
#if 1
        /* Active nested branch intentionally has no fallback. */
#else
        if (!USART1_UartCommRxReadyHook())
        {
            USART1_ISR_RX_Handler();
        }
#endif
#endif
    }
}
"""
        check = self.verify_usart_source(isr_source)

        self.assertTrue(any("RX-ready" in failure for failure in check.failures))

    def test_unknown_directive_around_rx_contract_fails_closed(self) -> None:
        isr_source = """
void USART1_InterruptHandler(void)
{
    uint32_t errorStatus = USART1_REGS->US_CSR & US_CSR_USART_OVRE_Msk;
    if (errorStatus != 0U)
    {
        if (!USART1_UartCommErrorHook(errorStatus))
        {
            USART1_ErrorClear();
        }
    }
    if ((USART1_REGS->US_CSR & US_CSR_USART_RXRDY_Msk) != 0U)
    {
#if FEATURE_RX_HOOK
        if (!USART1_UartCommRxReadyHook())
        {
            USART1_ISR_RX_Handler();
        }
#endif
    }
}
"""
        check = self.verify_usart_source(isr_source)

        self.assertTrue(any("RX-ready" in failure for failure in check.failures))


class GetMacroTests(unittest.TestCase):
    def verify_get(self, replacement: str) -> verifier.Verification:
        check = verifier.Verification()
        verifier.verify_get_macro(check, "UART1_DE_Get", replacement, 22)
        return check

    def test_valid_get_variants_are_accepted(self) -> None:
        variants = (
            "((PIOA_REGS->PIO_PDSR >> 22U) & 0x1U)",
            "(((PIOA_REGS->PIO_PDSR) >> (0x16UL)) & (1UL))",
            "((uint32_t)(((PIOA_REGS->PIO_PDSR) >> 22u)) & ((uint32_t)1u))",
        )
        for replacement in variants:
            with self.subTest(replacement=replacement):
                check = self.verify_get(replacement)

                self.assertEqual([], check.failures)

    def test_extra_or_inverting_operators_are_rejected(self) -> None:
        replacements = (
            "(((PIOA_REGS->PIO_PDSR >> 22U) & 1U) ^ 1U)",
            "(((PIOA_REGS->PIO_PDSR >> 22U) & 1U) | 2U)",
            "(((PIOA_REGS->PIO_PDSR >> 22U) & 1U) + 1U)",
        )
        for replacement in replacements:
            with self.subTest(replacement=replacement):
                check = self.verify_get(replacement)

                self.assertTrue(check.failures)

    def test_commuted_mask_expression_is_accepted(self) -> None:
        check = self.verify_get("(1U & (PIOA_REGS->PIO_PDSR >> 22U))")

        self.assertEqual([], check.failures)


class PioTests(FixtureCase):
    EXPECTED_ASSIGNMENTS = """
PIOA_REGS->PIO_PDR = 0U;
PIOA_REGS->PIO_PER = 0x01400000U;
PIOA_REGS->PIO_OER = 0x01400000U;
PIOA_REGS->PIO_ODR = 0U;
PIOA_REGS->PIO_ODSR = 0U;
    """

    def test_assignments_in_string_are_rejected(self) -> None:
        escaped_assignments = self.EXPECTED_ASSIGNMENTS.strip().replace("\n", r"\n")
        source = f'''\
void PIO_Initialize(void)
{{
    const char *ignored = "{escaped_assignments}";
}}
'''
        check = self.verify_file(
            "PIO_C_PATH", source, verifier.verify_pio_initialization
        )

        self.assertTrue(check.failures)

    def test_assignments_in_if_zero_branch_are_rejected(self) -> None:
        source = f"""
void PIO_Initialize(void)
{{
#if 0
{self.EXPECTED_ASSIGNMENTS}
#endif
}}
"""
        check = self.verify_file(
            "PIO_C_PATH", source, verifier.verify_pio_initialization
        )

        self.assertTrue(check.failures)


class CsvTests(FixtureCase):
    def verify_csv_source(self, content: str) -> verifier.Verification:
        return self.verify_file("CSV_PATH", content, verifier.verify_csv)

    def test_valid_csv_is_accepted(self) -> None:
        check = self.verify_csv_source(VALID_CSV)

        self.assertEqual([], check.failures)

    def test_unterminated_quoted_row_is_rejected(self) -> None:
        check = self.verify_csv_source(VALID_CSV + '99,PA1,"unterminated\n')

        self.assertTrue(check.failures)

    def test_duplicate_header_is_rejected(self) -> None:
        source = """Pin ID,Pin ID,Custom Name,Function,Direction,Latch
PA22,PA22,UART1_DE,GPIO,Out,Low
PA24,PA24,UART1_nRE,GPIO,Out,Low
"""
        check = self.verify_csv_source(source)

        self.assertTrue(check.failures)

    def test_extra_field_is_rejected(self) -> None:
        source = VALID_CSV.replace(
            "37,PA22,UART1_DE,GPIO,Out,Low,No,Disabled,No,No,Disabled,Low",
            "37,PA22,UART1_DE,GPIO,Out,Low,No,Disabled,No,No,Disabled,Low,EXTRA",
        )
        check = self.verify_csv_source(source)

        self.assertTrue(check.failures)

    def test_missing_field_is_rejected(self) -> None:
        source = VALID_CSV.replace(
            "37,PA22,UART1_DE,GPIO,Out,Low,No,Disabled,No,No,Disabled,Low",
            "37,PA22,UART1_DE,GPIO,Out,Low,No,Disabled,No,No,Disabled",
        )
        check = self.verify_csv_source(source)

        self.assertTrue(check.failures)


class NvicTests(FixtureCase):
    def test_parenthesized_priority_is_accepted(self) -> None:
        source = """
void NVIC_Initialize(void)
{
    NVIC_SetPriority(USART1_IRQn, (7U));
}
"""
        check = self.verify_file("NVIC_PATH", source, verifier.verify_nvic)

        self.assertEqual([], check.failures)

    def test_wrong_parenthesized_priority_is_rejected(self) -> None:
        source = """
void NVIC_Initialize(void)
{
    NVIC_SetPriority(USART1_IRQn, (6U));
}
"""
        check = self.verify_file("NVIC_PATH", source, verifier.verify_nvic)

        self.assertTrue(check.failures)

    def test_priority_call_in_string_is_rejected(self) -> None:
        source = '''
void NVIC_Initialize(void)
{
    const char *ignored = "NVIC_SetPriority(USART1_IRQn, 7)";
}
'''
        check = self.verify_file("NVIC_PATH", source, verifier.verify_nvic)

        self.assertTrue(check.failures)

    def test_priority_call_in_if_zero_branch_is_rejected(self) -> None:
        source = """
void NVIC_Initialize(void)
{
#if 0
    NVIC_SetPriority(USART1_IRQn, 7);
#endif
}
"""
        check = self.verify_file("NVIC_PATH", source, verifier.verify_nvic)

        self.assertTrue(check.failures)


if __name__ == "__main__":
    unittest.main(verbosity=2)
