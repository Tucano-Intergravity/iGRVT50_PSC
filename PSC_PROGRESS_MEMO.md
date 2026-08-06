# PSC Progress Memo

Last updated: 2026-08-05 KST

Read this file first when resuming the PSC / iGRVT50 SAMV71 board test work.

## Project

- Workspace: `C:\PSC\SAM_CTL_Control - IO`
- MPLAB project: `C:\PSC\SAM_CTL_Control - IO\sam_ctl.X`
- GitHub remote: `https://github.com/Tucano-Intergravity/iGRVT50_PSC.git`
- Branch: `main`
- Latest pushed work: `2ef1cb6 Add heater and spark plug telecommand control`

## Current Firmware Goal

The firmware is configured for PSC board-level sensor telemetry and solenoid telecommand testing.

- RS485-style TMTC uses USART1 at `921600 8N1`.
- The old `HELLO\r\n` test output is inactive.
- Telemetry is not sent periodically.
- A valid `TMREQ` sends exactly one telemetry packet immediately.
- A valid `SVCON` returns `$iGRVT50,Ack\r\n`, then applies actuator states.
- LPV/HPV automatic toggle or cycle test code is removed from runtime operation.
- Solenoid/heater/SP states change only by valid TMTC command or explicit debug-shell command.
- Heater1~4 are PWM duty-controlled heater outputs.
- Former Heater5/PC5 is now SP (spark plug) and is controlled as GPIO ON/OFF, not PWM.

## TMTC Packets

Telemetry packet sent in response to `TMREQ`:

```text
$iGRVT50,<SystemTick>,<mode>,<PT1 mV>,<PT2 mV>,<PT3 mV>,<PT4 mV>,<PT5 mV>,<PT6 mV>,<PT7 mV>,<PT8 mV>,<PT9 mV>,<TC1 uV>,<TC2 uV>,<TC3 uV>,<TC4 uV>\r\n
```

- `SystemTick` is the 32-bit FreeRTOS tick in milliseconds.
- `mode` is one of: `init_mode`, `normal_mode`, `run_mode`, `diagnostic_mode`.
- Tick range is `0..4294967295`, then wraps to `0`.
- Wrap period is `2^32 ms`, about 49 days 17 hours 2 minutes 47.296 seconds.

TM request:

```text
$iGRVT50,TMREQ\r\n
```

- No Ack.
- Firmware sends one telemetry packet immediately.

Mode request:

```text
$iGRVT50,MODE,<mode>\r\n
```

- Valid mode values: `0/init`, `1/normal`, `2/run`, `3/diagnostic`.
- Full text forms are also accepted: `init_mode`, `normal_mode`, `run_mode`, `diagnostic_mode`.
- Valid MODE command response is `$iGRVT50,Ack\r\n`.
- The requested mode is applied by the 100 Hz state-machine event.

Actuator control:

```text
$iGRVT50,SVCON,<LPV1 0/1>,<LPV2 0/1>,<LPV3 0/1>,<LPV4 0/1>,<LPV5 0/1>,<LPV6 0/1>,<LPV7 0/1>,<LPV8 0/1>,<LPV9 0/1>,<LPV10 0/1>,<LPV11 0/1>,<LPV12 0/1>,<HPV1 0/1>,<HPV2 0/1>,<HPV3 0/1>,<HPV4 0/1>,<HPV5 0/1>,<HPV6 0/1>,<HPV7 0/1>,<HPV8 0/1>,<HTR1 0/1>,<HTR2 0/1>,<HTR3 0/1>,<HTR4 0/1>,<SP 0/1>\r\n
```

- `HTR1`~`HTR4`: `1` means Heater 100% duty ON, `0` means OFF.
- `SP`: `1` means Spark Plug GPIO ON, `0` means OFF.

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
- `RsTask_ProcessTelecommandLine()` parses `TMREQ`, `MODE`, `SVCON`, and `DIAG`.
- RX parser ignores bytes before `$`, restarts on a new `$`, and recovers from partial/glued retry frames.
- `RSTASK_NOTIFY_TM_EVENT` remains defined as a reserved internal event, but no default timer callback sends it.
- Definition-document mode restrictions are not implemented yet; current `SVCON` applies all parsed fields.

## State Machine

- State machine files:
  - `sam_ctl.X/iGRVT50/header/statemachine.h`
  - `sam_ctl.X/iGRVT50/source/statemachine.c`
- Modes are `init_mode`, `normal_mode`, `run_mode`, and `diagnostic_mode`.
- `StateMachine_100HzEvent()` is called from the 10 ms OPU timer callback.
- Boot starts at `init_mode`; current code requests `normal_mode` after init entry.
- Add mode behavior inside `StateMachine_InitMode()`, `StateMachine_NormalMode()`, `StateMachine_RunMode()`, and `StateMachine_DiagnosticMode()`.
- `StateMachine_GetModeName()` is only for display/TM text conversion.

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
- The 10 ms callback also services LPV/HPV open-hold timing and runs the state-machine 100 Hz event.

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
- LPV open-hold behavior:
  - OPEN: 20 ms nominal, 100% duty, about 28 V output with 28 V supply.
  - HOLD: 20% duty.
  - PWM frequency: LPV1-LPV8 = 20 kHz; LPV9-LPV12 via TC0 = about 19.99 kHz.
  - LPV is voltage/duty control, not current regulation.
  - Repeated ON while already OPEN/HOLD is a no-op, so it should not restart the open pulse.
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
- HPV open-hold behavior:
  - OPEN: 20 ms nominal, DRV3946 force-duty mode, effectively 100% drive.
  - With 28 V supply and 35 ohm coil, OPEN current is about 0.8 A.
  - HOLD: DRV3946 current-regulation mode.
  - Requested HOLD current is 100 mA, but current register clamps to minimum register 0, about 188 mA with the current code formula.
  - With a 35 ohm coil, expected HOLD voltage is about 6.6 V.
- Repeated identical HPV state requests are treated as no-op inside the HPV module, so repeated SVCON retries should not cause a close/open click.

## Heater / SP Control

- Heater/SP implementation commit: `2ef1cb6 Add heater and spark plug telecommand control`.
- Heater channels:
  - Heater1: PE0 / TIOA9
  - Heater2: PE1 / TIOB9
  - Heater3: PE3 / TIOA10
  - Heater4: PE4 / TIOB10
- SP (spark plug):
  - PC5 direct GPIO ON/OFF.
  - PWM is not used.
  - Former Heater5 role is removed from `Heater_SetDuty()`.
- Source files:
  - `src/opu_task.c`: parses `HTR1`~`HTR4` and `SP` in `SVCON`; safe state forces Heater1~4 and SP OFF.
  - `src/pwm_func.c`: `Heater_SetDuty()` accepts channels 1~4 only; `SparkPlug_Set()` controls PC5 GPIO.
  - `src/sam_ctl.h`: declares `SparkPlug_Set()`.
  - `src/dbg_task.c`: `htr` command is Heater1~4 only; `sp <0|1>` command added; `off` also turns SP OFF.
- Verification after implementation:
  - MPLAB make build succeeded.
  - `python -m py_compile tools\psc_uart_monitor_gui.py` succeeded.
  - `git diff --check` succeeded.

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
- GUI parses the telemetry `mode` field after `SystemTick`.
- Telecommand controls now separate LPV, HPV, and Heater/SP groups.
- GUI includes LPV1-LPV12, HPV1-HPV8, HTR1-HTR4, and SP checkboxes.
- GUI has a mode selector and `Send MODE`.
- `Request TM`, `Send MODE`, `Send SVCON`, and `All Off` buttons are grouped in a `Commands` box.
- GUI sends no artificial TX preamble now.
- Command timeout is `0.1 s`.
- `TMREQ`, `MODE`, and `SVCON` automatically retry up to 5 times if no expected response arrives.

## Definition Document Work

- Source document reviewed: `C:\Users\USER\OneDrive\바탕 화면\PSC(추진제어기) 운영 모드, TC 및 TM 정의서.pdf`
- Review date: 2026-08-05 KST.
- No code changes were made during the definition-strategy review.
- User decision: terminal valves are `HPV1` and `HPV2`.
- SP is the already implemented spark plug on PC5 GPIO.
- Important definition gap versus current firmware:
  - Normal/Run mode must block `HPV1`, `HPV2`, and `SP` control.
  - Diagnostic mode may control all valves, heaters, terminal valves, and SP.
  - Current firmware does not yet enforce this mode-dependent actuator authority.
  - Current `MODE` TC is a direct mode request; definition expects higher-level TCs such as Thruster Start, Diag IN/OUT, PAR Start/Stop, EStop, and Ping.
  - Current `DIAG` command is a parser/communication diagnostic request, not the definition's `Diag IN/OUT` mode-control TC.
  - Current telemetry lacks actuator state and fault-code fields described by the definition.

Recommended staged implementation strategy:

1. Freeze TC/TM command names and packet formats before coding.
2. Add a mode-authority matrix for each TC and actuator class.
3. Implement `SVCON` safety filtering first:
   - In Normal/Run, force `HPV1=0`, `HPV2=0`, and `SP=0`.
   - In Diagnostic, allow `HPV1`, `HPV2`, and `SP`.
4. Add mode-entry safety hooks:
   - Entering Normal or Run must force `HPV1`, `HPV2`, and `SP` OFF.
   - EStop must force all actuators OFF and request Normal mode.
5. Extend the state machine for Thruster Start, Run duration, Run pre-check pass/fail, and Normal return.
6. Define FDI thresholds and fault-code ownership before implementing automatic Run fault handling.
7. Extend TMREQ response to include LPV/HPV/HTR/SP state and fault code if the packet format is confirmed.
8. Update the GUI after firmware TC/TM formats are frozen.

## Hardware Notes Captured So Far

- LSV0 / LPSV1 was confirmed working.
- HPV0 / HPSV1 has the real solenoid connected.
- 2026-08-05: LPV and HPV operation confirmed during board test.
- LPV hold did not hold reliably at 10% duty, so LPV HOLD duty was raised to 20%.
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

- Build succeeded on 2026-08-05 after Heater/SP telecommand control was implemented.
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
