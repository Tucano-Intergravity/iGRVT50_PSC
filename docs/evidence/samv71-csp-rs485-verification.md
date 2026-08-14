# SAMV71 CSP RS485 Verification Evidence

- Date: 2026-08-14
- Branch: `PSC_CSP`
- Baseline commit: `44b7ddef920dc7b361f35cb374404775b2422941`
- Baseline source state: tracked files clean; untracked local files are `tools/csp-rs485/` and `tools/verify_hpv_ch2_config.py`.

## Authorization

The user approved use of `C:\PSC\csp-rs485` for this PSC firmware port on 2026-08-14.

Approved scope:

- Copy/use the reusable csp-rs485 core files required by the SAMV71 port.
- Convert USART1 to CSP-only operation.
- Stop using the legacy `$iGRVT50` ASCII USART1 transport in the CSP branch.

## Master/Responder Constraint

The PSC must not initiate RS485 traffic on its own. The OBC is the only master, and PSC transmission is allowed only as a response to an OBC request.

Implementation consequences:

- No periodic telemetry push over RS485.
- No spontaneous debug/status messages over RS485.
- No PSC-originated CSP ping, health, or command packets.
- USART1 DE/nRE direction control, if driven internally by PSC, is asserted only for response transmission and returns to receive mode afterward.

## Baseline Build

Commands run from `C:\PSC\SAM_CTL_Control - IO\sam_ctl.X`:

```powershell
& 'C:\Program Files\Microchip\MPLABX\v6.30\gnuBins\GnuWin32\bin\make.exe' -f Makefile CONF=default build
& 'C:\Program Files\Microchip\MPLABX\v6.30\gnuBins\GnuWin32\bin\make.exe' -f Makefile CONF=default TYPE_IMAGE=DEBUG_RUN build
```

Results:

- Production build: pass
- Debug build: pass
- `sam_ctl.X/dist/default/production/sam_ctl.X.production.hex`: 225944 bytes
- `sam_ctl.X/dist/default/production/sam_ctl.X.production.map`: 598727 bytes
- `sam_ctl.X/dist/default/debug/sam_ctl.X.debug.elf`: 766052 bytes

