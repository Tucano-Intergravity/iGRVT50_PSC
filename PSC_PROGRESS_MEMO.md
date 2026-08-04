# PSC Progress Memo

Last updated: 2026-08-04 KST

Read this file first when resuming the PSC / iGRVT50 SAMV71 board test work.

## Project

- Workspace: `C:\PSC\SAM_CTL_Control - IO`
- MPLAB project: `C:\PSC\SAM_CTL_Control - IO\sam_ctl.X`
- GitHub remote: `https://github.com/Tucano-Intergravity/iGRVT50_PSC.git`
- Branch: `main`
- Latest pushed work: check `git log -1 --oneline` after pull.

## Current Firmware Goal

The firmware is configured for PSC board-level sensor telemetry and solenoid telecommand testing.

- RS485-style TMTC uses USART1 at `921600 8N1`.
- The old `HELLO\r\n` test output is inactive.
- Telemetry is not sent periodically.
- A valid `TMREQ` sends exactly one telemetry packet immediately.
- A valid `SVCON` returns `$iGRVT50,Ack\r\n`, then applies solenoid states.
- LPV/HPV automatic toggle or cycle test code is removed from runtime operation.
- Solenoid states change only by valid TMTC command or explicit debug-shell command.

## TMTC Packets

Telemetry packet sent in response to `TMREQ`:

```text
$iGRVT50,<SystemTick>,<PT1 mV>,<PT2 mV>,<PT3 mV>,<PT4 mV>,<PT5 mV>,<PT6 mV>,<PT7 mV>,<PT8 mV>,<PT9 mV>,<TC1 uV>,<TC2 uV>,<TC3 uV>,<TC4 uV>\r\n
```

- `SystemTick` is the 32-bit FreeRTOS tick in milliseconds.
- Tick range is `0..4294967295`, then wraps to `0`.
- Wrap period is `2^32 ms`, about 49 days 17 hours 2 minutes 47.296 seconds.

TM request:

```text
$iGRVT50,TMREQ\r\n
```

- No Ack.
- Firmware sends one telemetry packet immediately.

Solenoid control:

```text
$iGRVT50,SVCON,<LPV1 0/1>,<LPV2 0/1>,<LPV3 0/1>,<LPV4 0/1>,<LPV5 0/1>,<LPV6 0/1>,<LPV7 0/1>,<LPV8 0/1>,<LPV9 0/1>,<LPV10 0/1>,<LPV11 0/1>,<LPV12 0/1>,<HPV1 0/1>,<HPV2 0/1>,<HPV3 0/1>,<HPV4 0/1>,<HPV5 0/1>,<HPV6 0/1>,<HPV7 0/1>,<HPV8 0/1>\r\n
```

SVCON response:

```text
$iGRVT50,Ack\r\n
```

Diagnostic request:

```text
$iGRVT50,DIAG\r\n
```

Diagnostic response:

```text
$iGRVT50,DIAG,<tick>,<rxBytes>,<rxDrops>,<rxErrors>,<lines>,<noHeader>,<headers>,<badFields>,<badBinary>,<unknownCmd>,<tmreq>,<svcon>,<ackSent>,<overflow>,<lastLen>\r\n
```

## Current Runtime Structure

Main runtime wiring is in `src/opu_task.c`.

- `RsTask` owns TMTC receive parsing and telemetry/Ack/DIAG transmit.
- USART1 RX bytes are captured in the USART1 RX interrupt into a ring buffer.
- `UartComm_SetRxNotifyTask()` notifies `RsTask` when RX bytes arrive.
- `RsTask` is notification-driven; it no longer polls UART every 1 ms.
- `RsTask_ProcessTelecommandLine()` parses `TMREQ`, `SVCON`, and `DIAG`.
- RX parser ignores bytes before `$`, restarts on a new `$`, and recovers from partial/glued retry frames.
- `RSTASK_NOTIFY_TM_EVENT` remains defined as a reserved internal event, but no default timer callback sends it.

## Timer Callback Structure

The blocking delay in `OpuTask` was replaced by a hardware timer event.

- TC1 channel 0 generates a 10 ms base tick.
- `TC1_CH0_Handler()` notifies `OpuTask` with `eIncrement`.
- `OpuTask` waits with `ulTaskNotifyTake(pdTRUE, portMAX_DELAY)`.
- Missed/coalesced ticks are serviced by passing the returned tick count into `OpuTimer_ServiceCallbacks()`.
- Software callbacks can be registered with:

```c
UInt8 OpuTimer_RegisterCallback(UInt32 periodMs, OpuTimerCallback callback, void *context);
```

- Periods are rounded up to the 10 ms base tick.
- Maximum registered callbacks: `OPU_TIMER_CALLBACK_MAX = 8`.

Default registered callbacks:

- `10 ms`: increments `s_opu10msCallbackCount`, refreshes WDT.
- `100 ms`: increments `s_opu100msCallbackCount`.
- `1000 ms`: increments `s_opu1000msCallbackCount`.

The old 100 ms callback debug output was removed:

- Removed `TcPrint()`.
- Removed `AdcPrint()`.
- Removed `s_opuDebugPrintDivider`.
- No timer callback sends telemetry by default.

## Sensor Acquisition

- PT acquisition runs in `AdcTask` every `50 ms` using `vTaskDelayUntil()`.
- TC acquisition runs in `TcTask` every `1000 ms`.
- TC conversion reads use `ADS1263_GetTemperatureTask()`, whose conversion waits use FreeRTOS delay instead of CPU busy-wait.
- `Sensor_GetScan()` returns the latest cached PT/TC values; it does not create the measurement period itself.

## Solenoid Control

- LPV count: 12 channels.
- HPV count: 8 channels.
- LPV module:
  - `sam_ctl.X/iGRVT50/header/lpsolvalve.h`
  - `sam_ctl.X/iGRVT50/source/lpsolvalve.c`
- HPV module:
  - `sam_ctl.X/iGRVT50/header/hpsolvalve.h`
  - `sam_ctl.X/iGRVT50/source/hpsolvalve.c`
- HPV mapping:
  - HPV1/2 = DRV3946 node0
  - HPV3/4 = node1
  - HPV5/6 = node2
  - HPV7/8 = node3
  - Odd valves use DRV channel 1, even valves use DRV channel 2.
- `HpSolValve_Init()` configures all four DRV3946 nodes at boot, then commands all channels OFF.
- Repeated identical HPV state requests are treated as no-op inside the HPV module, so repeated SVCON retries should not cause a close/open click.

Important current-limit note:

- Existing DRV3946 current formula in this codebase is:
  `I = (N + 17) / 272 * 3 A` for installed `R_IPROPI = 20k`.
- With that resistor value, 100 mA is below register range.
- The code clamps a 100 mA request to register `0`, about 188 mA.

## UART / RS485 Notes

- USART1 pins:
  - PA21 = RXD1
  - PB4 = TXD1
  - PA22 = DE GPIO
  - PA24 = nRE GPIO
- ISOW1432 transceiver.
- TX mode: `DE=1`, `nRE=1`.
- RX/idle mode: `DE=0`, `nRE=0`.
- `sam_ctl.X/iGRVT50/source/uartcomm.c` provides init, blocking transmit, RX ring read, loopback service, and ISR notification support.
- `UartComm_Init()` is idempotent and preserves captured RX bytes if called again.
- RX byte ring depth is 2048 bytes.
- USART1 fractional baud divider is applied after Harmony serial setup for 921600 bps.

## PC Monitoring Tools

- Repository GUI: `C:\PSC\SAM_CTL_Control - IO\tools\psc_uart_monitor_gui.py`
- Standalone GUI copy: `C:\PSC\psc_uart_monitor_gui.py`
- Double-click launcher: `C:\PSC\run_psc_uart_monitor_gui.bat`
- Default UART settings: `921600 8N1`.
- GUI parses `$iGRVT50` packets and displays PT1-PT9 in one row and TC1-TC4 in one row below PT.
- Sol Valve controls keep LPV1-LPV12 and HPV1-HPV8 checkboxes.
- `Request TM`, `Send SVCON`, and `All Off` buttons are grouped in a `Commands` box.
- GUI sends no artificial TX preamble now.
- Command timeout is `0.1 s`.
- `TMREQ` and `SVCON` automatically retry up to 5 times if no expected response arrives.

## Hardware Notes Captured So Far

- LSV0 / LPSV1 was confirmed working.
- HPV0 / HPSV1 has the real solenoid connected.
- Earlier connector guidance for HPV was pins 51 and 46, but confirm against schematic before rewiring.
- User found one pressure-sensor issue was connector contact related.
- PT1 input was once observed on PT2, so keep channel mapping in mind during future tests.

## Last Build

Build command:

```powershell
& 'C:\Program Files\Microchip\MPLABX\v6.30\gnuBins\GnuWin32\bin\make.exe' -f Makefile CONF=default build
```

Run from:

```text
C:\PSC\SAM_CTL_Control - IO\sam_ctl.X
```

Last known result:

- Build succeeded on 2026-08-04 after timer callback cleanup.
- HEX: `C:\PSC\SAM_CTL_Control - IO\sam_ctl.X\dist\default\production\sam_ctl.X.production.hex`

Known existing warnings:

- `dbg_task.c` command function pointer/type warnings may appear in full clean builds.
- `dbg_task.c` implicit `vTaskDelay` declaration warning may appear in full clean builds.
- These warnings predate the latest timer/TMTC changes.

## Suggested Resume Steps

1. Read this memo.
2. Run:

```powershell
git -C "C:\PSC\SAM_CTL_Control - IO" status --short --branch
```

3. If needed, inspect the latest commit:

```powershell
git -C "C:\PSC\SAM_CTL_Control - IO" log -1 --stat
```

4. Build from `sam_ctl.X` before flashing:

```powershell
& 'C:\Program Files\Microchip\MPLABX\v6.30\gnuBins\GnuWin32\bin\make.exe' -f Makefile CONF=default build
```
