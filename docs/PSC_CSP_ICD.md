# PSC-CSP Interface Control Document

Revision: Draft A  
Date: 2026-08-24  
Branch: `PSC_CSP`  
Code basis: commit `ffe00d6` plus current working-tree protocol changes  
Target reader: OBC CSP/RS485 software implementer

## 1. Scope

This ICD defines the CSP/RS485 interface between OBC and PSC.

PSC is a CSP responder on RS485. OBC is the bus master and initiates every transaction. PSC transmits only as a response to an OBC request, except the CSP reboot command where no application-level response shall be expected.

USART1 no longer uses the legacy ASCII `$iGRVT50` protocol.

## 2. Physical And Link Layer

| Item | Value |
|---|---|
| Physical link | RS485 UART |
| Baudrate | `921600 bps` |
| UART format | `8N1` |
| Wire byte size | 10 bits including start/stop |
| PSC behavior | Responder only |
| Recommended OBC transaction model | One outstanding request at a time |
| Suggested response timeout | `500 ms` |
| Suggested retry count | up to 5 retries |

## 3. CSP Address And Ports

| Node | CSP Address |
|---|---:|
| OBC | `0x0A` |
| PSC | `0x10` |

| Port | Name | Direction | Description |
|---:|---|---|---|
| `1` | `CSP_PING` | OBC -> PSC -> OBC | CSP standard ping echo |
| `4` | `CSP_REBOOT` | OBC -> PSC | CSP standard reboot |
| `10` | `COMMAND` | OBC -> PSC -> OBC | PSC telecommands |
| `11` | `TELEMETRY` | OBC -> PSC -> OBC | PSC telemetry requests |
| `12` | `DIAGNOSTICS` | OBC -> PSC -> OBC | PSC health request |

OBC shall use source ports in the dynamic range `14..63`. PSC responses use source address `0x10`, destination address `0x0A`, source port equal to the requested destination port, and destination port equal to the OBC request source port.

## 4. CSP Frame Encoding Over RS485

### 4.1 CSP ID

CSP ID is transmitted as a 32-bit big-endian value before CSP data.

| Bits | Field | Width | Current OBC Request Value |
|---:|---|---:|---|
| 31..30 | Priority | 2 | `2` |
| 29..25 | Source address | 5 | `0x0A` |
| 24..20 | Destination address | 5 | `0x10` |
| 19..14 | Destination port | 6 | target port |
| 13..8 | Source port | 6 | `14..63` |
| 7..0 | Flags | 8 | `0x00` |

Packing formula:

```c
uint32_t csp_id =
    ((priority & 0x03) << 30) |
    ((source & 0x1F) << 25) |
    ((destination & 0x1F) << 20) |
    ((destination_port & 0x3F) << 14) |
    ((source_port & 0x3F) << 8) |
    (flags & 0xFF);
```

### 4.2 CRC32C

Each RS485 CSP packet appends a 4-byte CRC32C to the CSP data. The CSP header is not included in CRC calculation.

| Item | Value |
|---|---|
| Algorithm | CRC32C |
| Initial value | `0xFFFFFFFF` |
| Reflected polynomial | `0x82F63B78` |
| Final XOR | `0xFFFFFFFF` |
| CRC coverage | CSP data/application payload only |
| CRC byte order | Big endian |

### 4.3 KISS Framing

Raw bytes before KISS escaping:

```text
CSP_ID_BE32 | CSP_DATA | CRC32C_BE32
```

KISS frame:

```text
0xC0 | 0x00 | escaped(CSP_ID_BE32 | CSP_DATA | CRC32C_BE32) | 0xC0
```

Escape rules:

| Raw byte | Escaped bytes |
|---:|---|
| `0xC0` | `0xDB 0xDC` |
| `0xDB` | `0xDB 0xDD` |

The second byte after the opening `0xC0` shall be KISS TNC data byte `0x00`.

## 5. Application Payload Common Header

Custom PSC ports `10`, `11`, and `12` use this application header.

| Offset | Size | Type | Name | Min | Max | Description |
|---:|---:|---|---|---:|---:|---|
| 0 | 1 | `u8` | `version` | 1 | 1 | PSC application protocol version |
| 1 | 1 | `u8` | `opcode` | command-specific | command-specific | Opcode |
| 2 | 2 | `u16` | `transaction_id` | 0 | 65535 | Big-endian transaction ID |

## 6. Common Status Response

All status-only responses and all telemetry responses begin with this 6-byte status block.

| Offset | Size | Type | Name | Min | Max | Description |
|---:|---:|---|---|---:|---:|---|
| 0 | 1 | `u8` | `version` | 1 | 1 | Echo protocol version |
| 1 | 1 | `u8` | `opcode` | request opcode | request opcode | Echo opcode |
| 2 | 2 | `u16` | `transaction_id` | 0 | 65535 | Echo transaction ID |
| 4 | 1 | `u8` | `status` | 0 | 255 | Status code |
| 5 | 1 | `u8` | `detail` | 0 | 255 | Error-specific detail |

| Status | Name | Meaning |
|---:|---|---|
| 0 | `OK` | Request accepted/completed |
| 1 | `BAD_VERSION` | Unsupported app protocol version |
| 2 | `BAD_LENGTH` | Payload length mismatch |
| 3 | `BAD_OPCODE` | Opcode not valid for the port |
| 4 | `INVALID_ARGUMENT` | Parameter out of accepted range |
| 5 | `INVALID_STATE` | Current PSC state does not allow command |
| 6 | `APPLY_FAILED` | Command decoded but failed while applying |
| 7 | `INTERNAL_ERROR` | Internal snapshot/encoding failure |
| 8 | `BUSY` | Reserved/available status |
| 255 | `DROP` | Internal drop status; normally no response |

## 7. Modes And Command Permissions

| Value | Mode |
|---:|---|
| 0 | `init_mode` |
| 1 | `normal_mode` |
| 2 | `run_mode` |
| 3 | `diagnostic_mode` |

| TC | Accepted In | Notes |
|---|---|---|
| `SET_OUTPUTS` | all modes | LPV/Heater/SP are applied in all modes. HPV bits are applied only in `diagnostic_mode`. |
| `SET_LPV_OUTPUTS` | all modes | LPV only. |
| `SET_MODE` | all modes | Requests mode `0..3`. |
| `THRUSTER_START` | `normal_mode` only | If accepted, PSC enters `run_mode`; after sequence completes, PSC returns to `normal_mode`. |
| `PAR_START` | `normal_mode` only | PAR routine continues after start until `PAR_STOP`. |
| `PAR_STOP` | all modes | Stops PAR and closes PAR valves. |
| `SIM_START` | `normal_mode`, `diagnostic_mode` | Enables sensor override. SIM is not a state-machine mode. |
| `SET_SIM_SENSOR_VALUES` | after `SIM_START` | Updates override sensor values. |
| `SIM_STOP` | all modes | Disables sensor override. |
| `FAULT_CLEAR` | all modes | Clears latched thruster fault flags. |

## 8. Telecommand Definitions

### 8.1 SET_OUTPUTS

| Item | Value |
|---|---|
| Port | `10` |
| Opcode | `0x01` |
| Request length | 10 bytes |
| Success response | Sensor Snapshot TM, 126 bytes |

| Offset | Size | Type | Name | Min | Max | Unit/Encoding |
|---:|---:|---|---|---:|---:|---|
| 0 | 1 | `u8` | `version` | 1 | 1 | fixed |
| 1 | 1 | `u8` | `opcode` | `0x01` | `0x01` | fixed |
| 2 | 2 | `u16` | `transaction_id` | 0 | 65535 | BE |
| 4 | 2 | `u16` | `lpv_on_mask` | `0x0000` | `0x0FFF` | bit0=SV-R01/LPV1 ... bit11=SV-R12/LPV12 |
| 6 | 1 | `u8` | `hpv_on_mask` | `0x00` | `0xFF` | bit0=SV-O1/HPV1 ... bit7=SPARE/HPV8 |
| 7 | 1 | `u8` | `heater_on_mask` | `0x0` | `0xF` | bit0=HTR1 ... bit3=HTR4 |
| 8 | 1 | `u8` | `spark_on` | 0 | 1 | 0=OFF, 1=ON |
| 9 | 1 | `u8` | `reserved` | 0 | 0 | fixed |

HPV bits are ignored unless PSC is in `diagnostic_mode`.

### 8.2 SET_MODE

| Item | Value |
|---|---|
| Port | `10` |
| Opcode | `0x02` |
| Request length | 5 bytes |
| Success response | Status, 6 bytes |

| Offset | Size | Type | Name | Min | Max | Unit/Encoding |
|---:|---:|---|---|---:|---:|---|
| 0 | 1 | `u8` | `version` | 1 | 1 | fixed |
| 1 | 1 | `u8` | `opcode` | `0x02` | `0x02` | fixed |
| 2 | 2 | `u16` | `transaction_id` | 0 | 65535 | BE |
| 4 | 1 | `u8` | `mode` | 0 | 3 | see mode table |

### 8.3 THRUSTER_START

| Item | Value |
|---|---|
| Port | `10` |
| Opcode | `0x03` |
| Request length | 32 bytes |
| Success response | Status, 6 bytes |
| Accepted mode | `normal_mode` only |

| Offset | Size | Type | Name | Min | Max | Unit/Encoding |
|---:|---:|---|---|---:|---:|---|
| 0 | 1 | `u8` | `version` | 1 | 1 | fixed |
| 1 | 1 | `u8` | `opcode` | `0x03` | `0x03` | fixed |
| 2 | 2 | `u16` | `transaction_id` | 0 | 65535 | BE |
| 4 | 4 | `u32` | `burn_time_ms` | 1 | 4294967295 | ms |
| 8 | 4 | `u32` | `sv_o3_open_delay_ms` | 0 | 4294967295 | ms after command accept |
| 12 | 4 | `u32` | `sv_f3_open_delay_ms` | 0 | 4294967295 | ms after command accept |
| 16 | 4 | `u32` | `spark_on_delay_ms` | 0 | 4294967295 | ms after command accept |
| 20 | 4 | `u32` | `spark_on_duration_ms` | 0 | 4294967295 | ms |
| 24 | 4 | `u32` | `sv_o3_close_delay_ms` | 0 | 4294967295 | ms after burn end |
| 28 | 4 | `u32` | `sv_f3_close_delay_ms` | 0 | 4294967295 | ms after burn end |

Current sequence valves:

| Function | Physical valve |
|---|---|
| Oxidizer thruster valve | `SV-O3` / `HPV3` |
| Fuel thruster valve | `SV-F3` / `HPV7` |
| Ignition | Spark Plug / Heater05 |

Current firmware validates only `burn_time_ms != 0`. Other timing fields are accepted over the full `u32` range.

### 8.4 PAR_START

| Item | Value |
|---|---|
| Port | `10` |
| Opcode | `0x04` |
| Request length | 5 bytes |
| Success response | Status, 6 bytes |
| Accepted mode | `normal_mode` only |

| Offset | Size | Type | Name | Min | Max | Unit/Encoding |
|---:|---:|---|---|---:|---:|---|
| 0 | 1 | `u8` | `version` | 1 | 1 | fixed |
| 1 | 1 | `u8` | `opcode` | `0x04` | `0x04` | fixed |
| 2 | 2 | `u16` | `transaction_id` | 0 | 65535 | BE |
| 4 | 1 | `u8` | `selector` | 0 | 3 | bit field |

Selector:

| Bit | 0 | 1 |
|---:|---|---|
| bit0 | Ox pressure sensor `PT-O3` | Ox pressure sensor `PT-O4` |
| bit1 | Fuel pressure sensor `PT-F3` | Fuel pressure sensor `PT-F4` |

### 8.5 PAR_STOP

| Item | Value |
|---|---|
| Port | `10` |
| Opcode | `0x05` |
| Request length | 4 bytes |
| Success response | Status, 6 bytes |
| Accepted mode | all modes |

Payload is only the common request header.

### 8.6 SET_LPV_OUTPUTS

| Item | Value |
|---|---|
| Port | `10` |
| Opcode | `0x06` |
| Request length | 6 bytes |
| Success response | Sensor Snapshot TM, 126 bytes |

| Offset | Size | Type | Name | Min | Max | Unit/Encoding |
|---:|---:|---|---|---:|---:|---|
| 0 | 1 | `u8` | `version` | 1 | 1 | fixed |
| 1 | 1 | `u8` | `opcode` | `0x06` | `0x06` | fixed |
| 2 | 2 | `u16` | `transaction_id` | 0 | 65535 | BE |
| 4 | 2 | `u16` | `lpv_on_mask` | `0x0000` | `0x0FFF` | bit0=SV-R01/LPV1 ... bit11=SV-R12/LPV12 |

### 8.7 SET_SIM_SENSOR_VALUES

| Item | Value |
|---|---|
| Port | `10` |
| Opcode | `0x07` |
| Request length | 60 bytes |
| Success response | Sensor Snapshot TM, 126 bytes |
| Precondition | `SIM_START` accepted |

| Offset | Size | Type | Name | Min | Max | Unit/Encoding |
|---:|---:|---|---|---:|---:|---|
| 0 | 1 | `u8` | `version` | 1 | 1 | fixed |
| 1 | 1 | `u8` | `opcode` | `0x07` | `0x07` | fixed |
| 2 | 2 | `u16` | `transaction_id` | 0 | 65535 | BE |
| 4 | 36 | `i32[9]` | `pt_millibar` | -2147483648 | 2147483647 | mbar |
| 40 | 20 | `i32[5]` | `tc_millikelvin` | -2147483648 | 2147483647 | mK |

TC override validity is set only when each `tc_millikelvin` is in `1..2500000 mK`.

### 8.8 SIM_START

| Item | Value |
|---|---|
| Port | `10` |
| Opcode | `0x08` |
| Request length | 4 bytes |
| Success response | Status, 6 bytes |
| Accepted modes | `normal_mode`, `diagnostic_mode` |

Payload is only the common request header. On start, override values are initialized from the most recent real sensor values.

### 8.9 SIM_STOP

| Item | Value |
|---|---|
| Port | `10` |
| Opcode | `0x09` |
| Request length | 4 bytes |
| Success response | Status, 6 bytes |
| Accepted mode | all modes |

Payload is only the common request header.

### 8.10 FAULT_CLEAR

| Item | Value |
|---|---|
| Port | `10` |
| Opcode | `0x0A` |
| Request length | 4 bytes |
| Success response | Status, 6 bytes |
| Accepted mode | all modes |

Payload is only the common request header. This clears latched thruster fault flags.

## 9. Telemetry Definitions

### 9.1 GET_SENSOR_SNAPSHOT

| Item | Value |
|---|---|
| Port | `11` |
| Opcode | `0x01` |
| Request length | 4 bytes |
| Response length | 126 bytes |

Request payload is only the common request header.

Response:

| Offset | Size | Type | Name | Min | Max | Unit/Encoding |
|---:|---:|---|---|---:|---:|---|
| 0 | 6 | status | `status_block` | - | - | common status response |
| 6 | 4 | `u32` | `sample_time_ms` | 0 | 4294967295 | ms |
| 10 | 1 | `u8` | `current_mode` | 0 | 3 | mode |
| 11 | 1 | `u8` | `requested_mode` | 0 | 3 | mode |
| 12 | 2 | `u16` | `validity_mask` | 0 | 65535 | bit mask |
| 14 | 36 | `i32[9]` | `pt_millivolt` | -2147483648 | 2147483647 | mV, pressure sensor input-side voltage after PSC internal calibration |
| 50 | 36 | `i32[9]` | `pt_millibar` | -2147483648 | 2147483647 | mbar |
| 86 | 20 | `i32[5]` | `tc_microvolt` | -2147483648 | 2147483647 | uV |
| 106 | 20 | `i32[5]` | `tc_millikelvin` | -2147483648 | 2147483647 | mK |

Pressure sensor TM values are transmitted after PSC-side divider compensation and per-channel calibration. OBC shall treat `pt_millivolt` and `pt_millibar` as calibrated telemetry values; calibration coefficients are PSC internal implementation data and are not part of this ICD.

Current validity mask:

| Bits | Meaning |
|---|---|
| bit0..bit8 | PT channels valid |
| bit9..bit13 | TC temperature channels valid |

### 9.2 GET_SOLVALVE_STATE

| Item | Value |
|---|---|
| Port | `11` |
| Opcode | `0x02` |
| Request length | 4 bytes |
| Response length | 16 bytes |

Request payload is only the common request header.

Response:

| Offset | Size | Type | Name | Min | Max | Unit/Encoding |
|---:|---:|---|---|---:|---:|---|
| 0 | 6 | status | `status_block` | - | - | common status response |
| 6 | 4 | `u32` | `sample_time_ms` | 0 | 4294967295 | ms |
| 10 | 1 | `u8` | `current_mode` | 0 | 3 | mode |
| 11 | 2 | `u16` | `lpv_on_mask` | `0x0000` | `0x0FFF` | LPV state |
| 13 | 1 | `u8` | `hpv_on_mask` | `0x00` | `0xFF` | HPV state |
| 14 | 1 | `u8` | `heater_on_mask` | `0x0` | `0xF` | Heater state |
| 15 | 1 | `u8` | `spark_on` | 0 | 1 | Spark Plug state |

### 9.3 GET_HEALTH

| Item | Value |
|---|---|
| Port | `12` |
| Opcode | `0x01` |
| Request length | 4 bytes |
| Response length | 17 bytes |

Request payload is only the common request header.

Response:

| Offset | Size | Type | Name | Min | Max | Unit/Encoding |
|---:|---:|---|---|---:|---:|---|
| 0 | 6 | status | `status_block` | - | - | common status response |
| 6 | 4 | `u32` | `uptime_ms` | 0 | 4294967295 | RTOS tick count in ms-equivalent |
| 10 | 1 | `u8` | `current_mode` | 0 | 3 | mode |
| 11 | 1 | `u8` | `link_state` | 0 | 255 | RS485 link state enum |
| 12 | 1 | `u8` | `last_error` | 0 | 255 | RS485 link last error enum |
| 13 | 4 | `u32` | `thruster_fault_flags` | 0 | 4294967295 | bit flags |

`GET_HEALTH` intentionally excludes debug message queue and link counters.

## 10. CSP Standard Services

### 10.1 CSP_PING

| Item | Value |
|---|---|
| Port | `1` |
| App header | Not used |
| Suggested payload | ASCII `PSC-PING`, hex `50 53 43 2D 50 49 4E 47` |
| Response | Echo of request payload |

### 10.2 CSP_REBOOT

| Item | Value |
|---|---|
| Port | `4` |
| App header | Not used |
| Payload | `u32` magic `0x80078007`, big endian |
| Response | Do not expect an application response |

Accepted reboot request causes PSC to call `NVIC_SystemReset()`.

## 11. Channel Mapping

### 11.1 Pressure Sensors

| Index | TM Label | Physical |
|---:|---|---|
| 0 | `PT-O1` | PT1 |
| 1 | `PT-O2` | PT2 |
| 2 | `PT-O3` | PT3 |
| 3 | `PT-O4` | PT4 |
| 4 | `PT-F1` | PT5 |
| 5 | `PT-F2` | PT6 |
| 6 | `PT-F3` | PT7 |
| 7 | `PT-F4` | PT8 |
| 8 | `PT-C1` | PT9 |

Pressure conversion:

| Sensor | 0 bar | Full scale | Full-scale pressure |
|---|---:|---:|---:|
| PT-O1..O4, PT-F1..F4 | 0.5 V | 4.5 V | 100 bar |
| PT-C1 | 0.5 V | 4.5 V | 16 bar |

The firmware compensates the pressure ADC divider using:

```text
gain = (20000 + 78700/2) / (78700/2)
```

### 11.2 Temperature Sensors

| Index | TM Label | Electrical |
|---:|---|---|
| 0 | `TC-O1` | TC1 AIN0/1 |
| 1 | `TC-O2` | TC2 AIN2/3 |
| 2 | `TC-F1` | TC3 AIN4/5 |
| 3 | `TC-C1` | TC4 AIN6/7 |
| 4 | `CJC1 10k NTC` | AIN8/9 |

TC valid converted range is `0 K < T <= 2500 K`.

### 11.3 Solenoid Valves

| Bit | LPV Mask |
|---:|---|
| 0..11 | `SV-R01`..`SV-R12` / `LPV1`..`LPV12` |

| Bit | HPV Mask |
|---:|---|
| 0 | `SV-O1` / `HPV1` |
| 1 | `SV-O2` / `HPV2` |
| 2 | `SV-O3` / `HPV3` |
| 3 | `SPARE` / `HPV4` |
| 4 | `SV-F1` / `HPV5` |
| 5 | `SV-F2` / `HPV6` |
| 6 | `SV-F3` / `HPV7` |
| 7 | `SPARE` / `HPV8` |

| Bit | Heater Mask |
|---:|---|
| 0 | `HTR1` |
| 1 | `HTR2` |
| 2 | `HTR3` |
| 3 | `HTR4` |

Spark Plug is a separate `u8` field: `0=OFF`, `1=ON`.

## 12. Fault Flags

| Bit Mask | Name | Meaning |
|---:|---|---|
| `0x00000000` | `THRUSTER_FAULT_NONE` | No latched thruster fault |
| `0x00000001` | `THRUSTER_FAULT_PT_C1_HH` | PT-C1 above HH threshold |
| `0x00000002` | `THRUSTER_FAULT_PT_C1_LL` | PT-C1 below LL threshold |
| `0x00000004` | `THRUSTER_FAULT_TT_C1_HH` | TT-C1 above HH threshold |
| `0x00000008` | `THRUSTER_FAULT_PT_C1_INVALID` | PT-C1 invalid/out of plausible range |
| `0x00000010` | `THRUSTER_FAULT_TT_C1_INVALID` | TT-C1 invalid/out of plausible range |

Current run-monitor thresholds:

| Tag | Start | End | HH | LL | Valid range |
|---|---:|---:|---:|---:|---|
| `PT-C1` | 3250 ms | 20250 ms | 7800 mbar | 3000 mbar | -500..17000 mbar |
| `TT-C1` | 3250 ms | 20250 ms | 1200000 mK | none | 1..2500000 mK |

Fault flags are latched until `FAULT_CLEAR` is accepted.

## 13. Example Frames

Examples use:

| Field | Value |
|---|---|
| Priority | `2` |
| Source address | OBC `0x0A` |
| Destination address | PSC `0x10` |
| Flags | `0x00` |

### 13.1 GET_HEALTH

Request app payload:

```text
01 01 00 01
```

Frame with source port `14`, destination port `12`:

```text
c0 00 95 03 0e 00 01 01 00 01 c2 08 f0 02 c0
```

### 13.2 GET_SENSOR_SNAPSHOT

Request app payload:

```text
01 01 00 02
```

Frame with source port `15`, destination port `11`:

```text
c0 00 95 02 cf 00 01 01 00 02 d1 58 03 f6 c0
```

### 13.3 SET_LPV_OUTPUTS All Off

Request app payload:

```text
01 06 00 03 00 00
```

Frame with source port `16`, destination port `10`:

```text
c0 00 95 02 90 00 01 06 00 03 00 00 c2 53 b6 08 c0
```

### 13.4 CSP_PING

Payload:

```text
50 53 43 2d 50 49 4e 47
```

Frame with source port `17`, destination port `1`:

```text
c0 00 95 00 51 00 50 53 43 2d 50 49 4e 47 f6 5e e8 02 c0
```

### 13.5 CSP_REBOOT

Payload:

```text
80 07 80 07
```

Frame with source port `18`, destination port `4`:

```text
c0 00 95 01 12 00 80 07 80 07 41 3e 78 83 c0
```

## 14. OBC Bring-Up Checklist

1. Open RS485 UART at `921600 8N1`.
2. Send `CSP_PING` to address `0x10`, port `1`; verify echo payload.
3. Send `GET_HEALTH`; verify 17-byte app response and matching transaction ID.
4. Send `GET_SENSOR_SNAPSHOT`; verify 126-byte app response.
5. Send `GET_SOLVALVE_STATE`; verify 16-byte app response.
6. In `normal_mode`, test `SET_LPV_OUTPUTS`.
7. In `diagnostic_mode`, test HPV bits through `SET_OUTPUTS`.
8. Verify rejected mode cases return `INVALID_STATE`.
9. Verify `THRUSTER_START` enters `run_mode` and later returns to `normal_mode`.
10. Verify fault flags remain latched until `FAULT_CLEAR`.
