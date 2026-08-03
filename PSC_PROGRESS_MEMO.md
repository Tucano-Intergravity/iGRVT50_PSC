# PSC Progress Memo

Last updated: 2026-08-03 17:21 KST

Read this file first when resuming the PSC / iGRVT50 SAMV71 board test work.

## Project

- Workspace: `C:\PSC\SAM_CTL_Control - IO`
- MPLAB project: `C:\PSC\SAM_CTL_Control - IO\sam_ctl.X`
- GitHub remote: `https://github.com/Tucano-Intergravity/iGRVT50_PSC.git`
- Branch: `main`
- Last pushed commit before latest local edits: `1fc6154` (`Initial commit: PSC control firmware test setup`)

## Current Test Goal

The firmware is configured for board-level solenoid and sensor testing.

- RS485 line on USART1 is used for TMTC at `921600 8N1`.
- The old `HELLO\r\n` test message is no longer the active output.
- The RS485 status output is emitted from `RsTask`.
- Sensor telemetry is sent only when a valid `TMREQ` packet is received.
- Active packet:
  `$iGRVT50,<SystemTick>,<PT1 mV>,<PT2 mV>,<PT3 mV>,<PT4 mV>,<PT5 mV>,<PT6 mV>,<PT7 mV>,<PT8 mV>,<PT9 mV>,<TC1 uV>,<TC2 uV>,<TC3 uV>,<TC4 uV>\r\n`
- `SystemTick` is the 32-bit FreeRTOS tick in milliseconds (`0..4294967295`, then wraps to `0`).
- The `SystemTick` wrap period is `2^32 ms`, about 49 days 17 hours 2 minutes 47.296 seconds.
- SVCON telecommand packet:
  `$iGRVT50,SVCON,<LPV1 0/1>,<LPV2 0/1>,<LPV3 0/1>,<LPV4 0/1>,<LPV5 0/1>,<LPV6 0/1>,<LPV7 0/1>,<LPV8 0/1>,<LPV9 0/1>,<LPV10 0/1>,<LPV11 0/1>,<LPV12 0/1>,<HPV1 0/1>,<HPV2 0/1>,<HPV3 0/1>,<HPV4 0/1>,<HPV5 0/1>,<HPV6 0/1>,<HPV7 0/1>,<HPV8 0/1>\r\n`
- SVCON response packet:
  `$iGRVT50,Ack\r\n`
- SVCON Ack means the packet was received and validated. It is sent before valve state application so the first DRV3946 wake/config path does not cause a GUI timeout.
- TMREQ telecommand packet:
  `$iGRVT50,TMREQ\r\n`
- TMREQ has no Ack; firmware sends one telemetry packet immediately.
- The firmware does not send telemetry periodically anymore.
- `RsTask` no longer runs the automatic LPV1/HPV1 test pattern.
- Solenoid states are changed only by a valid telecommand packet or an explicit debug command.
- Legacy LPV/HPV periodic cycle test code has been removed.
- Repeated identical HPV ON/OFF requests are treated as no-op inside the HPV module, so SVCON retries should not briefly re-close and re-open HPV1.
- USART1 RX was changed from Harmony 1-byte read/callback re-arm to a continuous RXRDY interrupt byte ring, because the first command frame appeared to be dropped at 921600 bps.
- HPV DRV3946 CONFIG/wake is now performed during `HpSolValve_Init()` at boot. This moves the slow first-use driver initialization out of the first `SVCON` command.
- `UartComm_Init()` is idempotent and is called before HPV boot init, so USART1 RX interrupts are armed before the slow DRV3946 setup starts.
- The GUI prepends a 512-byte `U` preamble to each TMREQ/SVCON/DIAG write to absorb first-byte loss from an idle RS485 transmitter. The log still shows only the actual packet.
- `RsTask` priority is now `tskIDLE_PRIORITY + 3` and its RX polling period is 1 ms. This is to keep TMREQ/SVCON responses below the 0.1 s GUI retry timeout while ADC/TC tasks are busy.
- Added `DIAG` telecommand for debugging without the board debug port:
  `$iGRVT50,DIAG\r\n`
- DIAG response:
  `$iGRVT50,DIAG,<tick>,<rxBytes>,<rxDrops>,<rxErrors>,<lines>,<noHeader>,<headers>,<badFields>,<badBinary>,<unknownCmd>,<tmreq>,<svcon>,<ackSent>,<overflow>,<lastLen>\r\n`
- DIAG interpretation: if `rxBytes` does not increase for the first failed GUI attempt, the first write is being lost before the MCU UART. If `rxBytes` increases but `noHeader`/`badFields` increases, the MCU received bytes but the parser rejected the line.
- DIAG sample on 2026-08-03 17:03 showed `noHeader=7`, `svcon=1`, `ackSent=1` after a two-attempt SVCON, so the MCU had seen headerless/partial lines. Firmware RX now ignores bytes until `$`, restarts on any new `$`, and stores only packet candidates.

## Current Runtime Pattern

Active code path: `src/opu_task.c`, `RsTask`.

- Poll incoming UART bytes for telecommand lines.
- UART RX bytes are captured immediately in the USART1 interrupt and then consumed by high-priority `RsTask`.
- Send one telemetry packet immediately when `TMREQ` is received.
- Send `$iGRVT50,Ack\r\n` immediately after a valid `SVCON` packet is received, then apply solenoid states.
- Poll incoming UART bytes every 1 ms.
- No LPV or HPV channel is toggled automatically by `RsTask`.
- Boot can take longer because all four HPV DRV3946 nodes are configured before tasks start. USART1 RX byte capture is already active during this time, and HPV outputs remain OFF after initialization.

Important current-limit note:

- Existing DRV3946 current formula in this codebase is:
  `I = (N + 17) / 272 * 3 A` for the installed `R_IPROPI = 20k`.
- With that resistor value, 100 mA is below the register range.
- The code clamps a 100 mA request to register `0`, which is about 188 mA.

## PC Monitoring Tools

- Console monitor: `C:\PSC\psc_uart_monitor.py`
- GUI monitor: `C:\PSC\psc_uart_monitor_gui.py`
- Double-click launcher: `C:\PSC\run_psc_uart_monitor_gui.bat`
- Repository copies are stored under `tools/` for GitHub handoff.
- Default UART settings: 921600 bps, 8N1.
- The GUI parses `$iGRVT50` packets and displays PT1-PT9 in mV and TC1-TC4 in uV.
- The GUI reassembles received bytes until LF before parsing. Non-packet data is counted as `Ignored`; only real serial exceptions are counted as `Serial Err`.
- The GUI has LPV1-LPV12 and HPV1-HPV8 checkboxes plus `Request TM`, `Send SVCON`, and `All Off` buttons.
- The GUI also has `Request DIAG` for the firmware counters above.
- GUI layout: PT1-PT9 are shown in one horizontal row, TC1-TC4 are shown in one horizontal row below PT, Sol Valve checkboxes stay in the same layout, and the three operating buttons (`Request TM`, `Send SVCON`, `All Off`) are grouped in a separate `Commands` box.
- `Request TM` transmits `$iGRVT50,TMREQ\r\n` and expects telemetry only, with no Ack.
- `Send SVCON` transmits the SVCON packet defined above and expects `$iGRVT50,Ack\r\n`.
- GUI starts a 0.1 second response timeout after each `TMREQ` or `SVCON`.
- If no response arrives in 0.1 seconds, the GUI automatically retransmits the same command.
- Automatic retransmission is limited to 5 retries, so total attempts are initial send plus up to 5 retries.
- Missing response after all retries increments `Timeouts` and logs an error.
- GUI now waits 0.2 seconds after opening the serial port and enables command buttons only after the port-open event is received.
- GUI command timeouts now start after the serial worker reports actual TX completion, and log timestamps include milliseconds.

## Key Files

- `src/opu_task.c`
  - Main task wiring.
  - `RsTask` runs RS485 telemetry output and telecommand RX parsing only.
  - `RsTask_SendSensorPacket()` gets the current sensor scan and sends the `$iGRVT50` CSV packet through `UartComm_SendBlocking()`.
  - `RsTask_ProcessTelecommandLine()` parses `SVCON` and `TMREQ`.
  - RX line buffer is 512 bytes and the parser uses the last `$iGRVT50` header in a line, so it can recover if a partial first command and retry are glued together.
  - RX stream now discards leading non-`$` bytes instead of storing GUI preamble in the command line buffer.
  - Valid `SVCON` packets return `$iGRVT50,Ack\r\n` immediately after validation, then apply 12 LPV fields and 8 HPV fields.
  - Valid `TMREQ` packets send telemetry immediately and do not return Ack.
  - DRV3946 low-level SPI helpers are still in this file.

- `sam_ctl.X/iGRVT50/header/sensor.h`
- `sam_ctl.X/iGRVT50/source/sensor.c`
  - PT and TC scan/read access.

- `sam_ctl.X/iGRVT50/header/lpsolvalve.h`
- `sam_ctl.X/iGRVT50/source/lpsolvalve.c`
  - LPV1-LPV12 control API.
  - LPV1 keeps the dedicated 20 kHz PWM path (`LPSV1_PWM_PERIOD = 3750U`).
  - LPV2-LPV12 reuse the existing `MicroValve_SetDuty()` multi-channel mapping.

- `sam_ctl.X/iGRVT50/header/hpsolvalve.h`
- `sam_ctl.X/iGRVT50/source/hpsolvalve.c`
  - HPV1-HPV8 DRV3946 control API.
  - Mapping: HPV1/2=node0, HPV3/4=node1, HPV5/6=node2, HPV7/8=node3; odd valves use DRV ch1, even valves use DRV ch2.
  - Default ON command uses 800 mA peak and 100 mA requested hold.
  - `HpSolValve_Init()` configures all four DRV3946 nodes at boot and then commands both channels OFF.

- `sam_ctl.X/iGRVT50/header/uartcomm.h`
- `sam_ctl.X/iGRVT50/source/uartcomm.c`
  - USART1 / RS485-style communication interface.
  - Current port: USART1, 921600 bps, 8N1.
  - USART1 fractional baud divider is applied in `RS422_ApplyFractionalBaud()` after Harmony `USART1_SerialSetup()`.
  - Pins: PA21 RXD1, PB4 TXD1, PA22 DE, PA24 nRE.
  - Provides init, blocking transmit, RX queue read, loopback service, and the USART1 read callback.
  - `UartComm_Init()` returns immediately if already initialized, preserving any RX bytes captured before `RsTask` starts.
  - RX byte ring depth is 2048 bytes.

- `src/dbg_task.c`
  - Debug shell commands.
  - `hpv cycle` and `lpv cycle` commands have been removed.
  - `uart` with no argument prints USART1 TMTC RX counters; `uart 0`/`uart 1` still controls loopback.

## Hardware Notes Captured So Far

RS485 / RS422:

- USART1
- PA21 = RXD1
- PB4 = TXD1
- PA22 = DE GPIO
- PA24 = nRE GPIO
- ISOW1432 transceiver
- TX mode: `DE=1`, `nRE=1`
- RX/idle mode: `DE=0`, `nRE=0`

Solenoid notes:

- LSV0 / LPSV1 was confirmed working.
- HPV0 / HPSV1 has the real solenoid connected.
- Earlier connector guidance for HPV was pins 51 and 46, but confirm against schematic before rewiring.

Pressure / temperature:

- PT channels are scanned and printed.
- User observed PT1 input showing up on PT2 once due connector/channel mapping concerns.
- TC scan can be printed separately with line breaks if needed.

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

- Build succeeded.
- HEX: `C:\PSC\SAM_CTL_Control - IO\sam_ctl.X\dist\default\production\sam_ctl.X.production.hex`
- Last generated: 2026-08-03 16:10:21 KST

Known existing warnings:

- `dbg_task.c` command function pointer/type warnings.
- `dbg_task.c` implicit `vTaskDelay` declaration warning.
- These warnings predate the latest telemetry/telecommand changes.

## Current Git State When Memo Was Created

There are local uncommitted changes after the last push:

- `sam_ctl.X/iGRVT50/header/hpsolvalve.h`
- `sam_ctl.X/iGRVT50/source/hpsolvalve.c`
- `sam_ctl.X/iGRVT50/header/uartcomm.h`
- `sam_ctl.X/iGRVT50/source/uartcomm.c`
- `src/dbg_task.c`
- `src/opu_task.c`
- `src/rs422_func.c`
- `src/sam_ctl.h`
- `sam_ctl.X/nbproject/Makefile-default.mk`
- `sam_ctl.X/nbproject/configurations.xml`
- `PSC_PROGRESS_MEMO.md`

Next useful command:

```powershell
git status --short --branch
```

## Suggested Resume Steps

1. Read this memo.
2. Run `git status --short --branch`.
3. Review the latest diff if needed:

```powershell
git diff -- src/opu_task.c src/dbg_task.c src/rs422_func.c src/sam_ctl.h sam_ctl.X/iGRVT50/source/hpsolvalve.c sam_ctl.X/iGRVT50/header/hpsolvalve.h sam_ctl.X/iGRVT50/source/uartcomm.c sam_ctl.X/iGRVT50/header/uartcomm.h sam_ctl.X/nbproject/Makefile-default.mk sam_ctl.X/nbproject/configurations.xml PSC_PROGRESS_MEMO.md
```

4. If the user wants the latest HPV change saved remotely, commit and push these local changes.
