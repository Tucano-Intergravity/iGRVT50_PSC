# SAMV71 CSP/RS485 porting baseline verification

Date: 2026-08-06
Worktree: `C:\PSC\SAM_CTL_Control - IO\.worktrees\libcsp-rs485-porting`

## Baseline identity

- Initial revision: `7473a458a64603f08d671a56d9bd85e3f1b2b5f1`.
- Initial `git status --short`: no output (clean).
- Baseline-repair revision: `e66193cca8fea513922c2e5ca2c97ba5986d1b8d` (`build: restore fresh-worktree flag sentinels`).
- XC32: `pic32m-gcc.exe (Microchip XC32 Compiler v5.10) 13.2.1 20231009`, build date `Feb 18 2026`.

## Fresh-worktree build repair

The generated `sam_ctl.X/nbproject/Makefile-default.mk` names ignored MPLAB X
option-hash files as hard prerequisites but supplies no rule to recreate them.
The project-level `.gitignore` deliberately continues to ignore
`sam_ctl.X/.generated_files/flags/`.

RED reproduction: after validating that the following eight sentinels existed,
only these exact ignored files were removed from this worktree:

- `5f1a615de835c0dadf6b9899153dbf90212f958c`
- `65dbc0c684311d088ab529d7aba06906e755a801`
- `796fb4efb919673c9559e4457c577b080902dfa3`
- `83d7bb6a13d39165ef12690ac0d4d5894283c282`
- `8dcb86dd5ebb17a59f67b6579a360e9133f164df`
- `9c73f3ab4b7262cbf5edb51042dd0da91ea90670`
- `bb3d27b0a100815fcc3f4dcd0b159f799dbe90ad`
- `e40538f852addb3babb32ce5e602b4b8486438b0`

The debug command then exited 2 before compilation with:

```text
No rule to make target '.generated_files/flags/default/8dcb86dd5ebb17a59f67b6579a360e9133f164df', needed by 'build/default/debug/iGRVT50/source/hpsolvalve.o'.  Stop.
```

`sam_ctl.X/Makefile` now has a generic pattern rule that creates a missing
sentinel in the ignored directory. It does not modify generated Makefiles or
change the ignore policy. The first fallback recipe used `cp /dev/null`, which
failed under the bundled Windows tools; the final recipe uses `echo. > "$@"`.

GREEN verification, run after the eight-file removal:

```powershell
& 'C:\Program Files\Microchip\MPLABX\v6.30\gnuBins\GnuWin32\bin\make.exe' -C sam_ctl.X -f Makefile CONF=default build
# exit 0

& 'C:\Program Files\Microchip\MPLABX\v6.30\gnuBins\GnuWin32\bin\make.exe' -C sam_ctl.X -f Makefile CONF=default TYPE_IMAGE=DEBUG_RUN build
# exit 0
```

No compiler warnings were emitted by either successful baseline build. The
linker emitted its normal informational `Loading file` line.

## Baseline artifacts

| Artifact | Size (bytes) |
| --- | ---: |
| `sam_ctl.X/dist/default/production/sam_ctl.X.production.hex` | 216947 |
| `sam_ctl.X/dist/default/production/sam_ctl.X.production.elf` | 742112 |
| `sam_ctl.X/dist/default/production/sam_ctl.X.production.map` | 589785 |
| `sam_ctl.X/dist/default/debug/sam_ctl.X.debug.elf` | 742248 |
| `sam_ctl.X/dist/default/debug/sam_ctl.X.debug.map` | 583620 |

## csp-rs485 authorization gate

- Source checkout inspected read-only: `C:\PSC\csp-rs485`.
- Repository remote/owner namespace: `https://github.com/yjsong-intergravity/csp-rs485.git` (`yjsong-intergravity`).
- Inspected revision: `56addf6e936e78e3090b43ef5c3c8d60542f3b94` (tag `1.0.0`); its working tree was clean.
- Authorization source: user-confirmed authorization in this Codex task on
  2026-08-06: `둘다 오케이. 진행`. This confirmation explicitly covers copying,
  using, and redistributing the eight common csp-rs485 files inside this
  firmware repository.
- This records user confirmation only; it does not assert an upstream license
  or legal ownership beyond that authorization.

Covered paths verified in the source checkout:

- `csp_rs485/include/csp_rs485_link.h`
- `csp_rs485/include/csp_rs485_port.h`
- `csp_rs485/include/csp_rs485_profile.h`
- `csp_rs485/src/csp_rs485_internal.h`
- `csp_rs485/src/csp_rs485_freertos.c`
- `csp_rs485/src/csp_rs485_kiss.c`
- `csp_rs485/src/csp_rs485_link.c`
- `csp_rs485/src/csp_rs485_supervisor.c`

Result: the dependency authorization gate is satisfied for those eight common
files only. No csp-rs485 file was copied during this task.
