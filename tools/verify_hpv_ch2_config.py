"""Static regression checks for DRV3946 CH2/EN2 configuration handling."""

from __future__ import annotations

import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def read_text(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


def main() -> int:
    opu = read_text("src/opu_task.c")
    hpsv = read_text("sam_ctl.X/iGRVT50/source/hpsolvalve.c")
    hpsv_header = read_text("sam_ctl.X/iGRVT50/header/hpsolvalve.h")
    dbg = read_text("src/dbg_task.c")
    header = read_text("src/sam_ctl.h")

    failures: list[str] = []

    if "DRV3946_CONFIG_A4_CH2_EN2_VALUE" not in header:
        failures.append("sam_ctl.h must expose the expected CONFIG_A4 CH2/EN2 value.")

    if "cfgA[4] = DRV3946_CONFIG_A4_CH2_EN2_VALUE;" not in opu:
        failures.append("DRV3946_Wake must write CONFIG_A4 with the CH2/EN2 value.")

    if "DRV3946_Read24( (UInt8)(0x10U + i), 0U, &e )" in opu:
        failures.append("DRV3946_Wake must not verify CONFIG_A writes against fixed node0.")

    if "DRV3946_Read24( (UInt8)(0x10U + i), g_drvNode, &e )" not in opu:
        failures.append("DRV3946_Wake must verify CONFIG_A writes against the active node.")

    if "DRV3946_Read24( DRV3946_CONFIG_A4_REG" not in hpsv:
        failures.append("HpSolValve_EnsureNodeConfigured must read CONFIG_A4 before trusting cached state.")

    if "DRV3946_CONFIG_A4_CH2_EN2_VALUE" not in hpsv:
        failures.append("HpSolValve_EnsureNodeConfigured must compare CONFIG_A4 with the CH2/EN2 value.")

    init_match = re.search(
        r'if\( !strcmp\(argv\[1\], "INIT"\) \)\s*\{(?P<body>.*?)\n\s*\}',
        dbg,
        re.S,
    )
    if init_match is None:
        failures.append("dbg_task.c must keep an hpv init command block.")
    else:
        init_body = init_match.group("body")
        if "DRV3946_Wake" not in init_body:
            failures.append("hpv init must call the full DRV3946_Wake sequence.")
        if "0x40U" in init_body:
            failures.append("hpv init must not use the legacy REINIT_NAD-only sequence.")

    if "0x130CU" in dbg:
        failures.append("Debug CONFIG_A helpers must not program legacy CONFIG_A4=0x130C.")

    if "HpSolValve_GetConfigOkMask" not in hpsv_header:
        failures.append("hpsolvalve.h must expose cached HPV config diagnostics.")

    if failures:
        print("HPV CH2 configuration regression check failed:")
        for failure in failures:
            print(f"- {failure}")
        return 1

    print("HPV CH2 configuration regression check passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
