# PSC CSP 작업 진행 요약

- 작성 시각: 2026-08-21 15:06 KST
- 작업 브랜치: `PSC_CSP`
- 원격 저장소: `origin` / `https://github.com/Tucano-Intergravity/iGRVT50_PSC.git`
- 기준: 현재 `C:\PSC\SAM_CTL_Control - IO` 작업 트리

## 1. 통신 구조 변경

- 기존 ASCII/RS422 기반 TC/TM 흐름을 CSP/RS485 기반 바이너리 프로토콜로 전환했다.
- USART1을 CSP/RS485 물리계층으로 사용하도록 구성했다.
- PSC 보드에는 외부 DE/RE 제어 핀이 없으므로 `SAM_CSP_RESPONDER_ONLY=1` 구조로 반영했다.
- PSC는 OBC 요청에 대한 응답으로만 송신하는 responder 역할을 수행한다.
- 기존 `uartcomm`, `rs422_func` 기반 ASCII 송수신 경로는 프로젝트에서 제거했다.
- libcsp 및 csp-rs485 관련 소스/설정이 프로젝트에 포함되었다.

## 2. CSP 주소와 포트

- PSC local address: `0x10`
- OBC address: `0x0A`
- libcsp 표준 서비스:
  - Port 1: `CSP_PING`
  - Port 4: `CSP_REBOOT`
- PSC application ports:
  - Port 10: Command
  - Port 11: Telemetry
  - Port 12: Diagnostics / Health

## 3. TC/TM 프로토콜 정리

- Command opcode:
  - `0x01 SET_OUTPUTS`
  - `0x02 SET_MODE`
  - `0x03 THRUSTER_START`
  - `0x04 PAR_START`
  - `0x05 PAR_STOP`
  - `0x06 SET_LPV_OUTPUTS`
  - `0x07 SET_SIM_SENSOR_VALUES`
  - `0x08 SIM_START`
  - `0x09 SIM_STOP`
- Telemetry opcode:
  - `0x01 GET_SENSOR_SNAPSHOT`
  - `0x02 GET_SOLVALVE_STATE`
- Diagnostics opcode:
  - `0x01 GET_HEALTH`
- `SET_OUTPUTS`, `SET_LPV_OUTPUTS`, `SET_SIM_SENSOR_VALUES` 성공 응답은 센서 스냅샷 TM으로 통일했다.
- 센서 스냅샷에는 PT raw mV, PT 변환 압력 mbar, TC raw uV, TC 변환 온도 mK가 함께 포함된다.
- SV 상태 TM은 LPV/HPV/Heater/SP on mask를 반환한다.
- Health TM은 모드, link/error 상태, debug message queue, CSP/RS485 카운터를 반환한다.

## 4. 출력 제어와 물리 명칭 매핑

- `psc_io_map.h`를 추가하여 내부 코드에서 물리 명칭 기반 enum을 사용하도록 했다.
- HPV 매핑:
  - `SV-O1=HPV1`, `SV-O2=HPV2`, `SV-O3=HPV3`
  - `SV-F1=HPV5`, `SV-F2=HPV6`, `SV-F3=HPV7`
  - `HPV4`, `HPV8`은 spare로 유지
- LPV 매핑:
  - `SV-R01` ~ `SV-R12` = `LPV1` ~ `LPV12`
- Pressure sensor 매핑:
  - `PT-O1` ~ `PT-O4`
  - `PT-F1` ~ `PT-F4`
  - `PT-C1`
- Heater/SP:
  - Heater는 HTR1~4만 유지한다.
  - SP는 PC5 GPIO ON/OFF 제어로 유지하며 PWM을 사용하지 않는다.
- `SET_OUTPUTS`에서 HPV는 diagnostic mode에서만 직접 제어한다.
- LPV 단독 제어 TC(`SET_LPV_OUTPUTS`)를 추가했다.

## 5. Thruster Start 시퀀스

- `THRUSTER_START`는 Normal mode에서 수신 가능하다.
- 수신 후 pre-run check를 수행하고 성공 시 Run mode로 진입한다.
- Run mode에서 공통 시퀀스 코드를 사용하여 밸브와 SP를 타이밍 기반으로 제어한다.
- 타이밍은 TC 파라미터로 수신한다:
  - burn time
  - SV-O3 open delay
  - SV-F3 open delay
  - SP start delay
  - SP duration
  - SV-O3 close delay
  - SV-F3 close delay
- Thruster Start에서 작동하는 밸브는 `SV-O3(HPV3)`와 `SV-F3(HPV7)`로 변경했다.
- SP는 start sequence에서 켜지고, SP duration 경과 후 바로 꺼진다.
- 모든 stop 시퀀스가 종료된 뒤 Normal mode로 복귀한다.
- debug event는 Health TM에 포함되며, 밸브/SP 동작 및 check routine 호출 시점 확인이 가능하다.

## 6. Emergency / Check Hook

- Thruster 진입 전 조건 확인 함수:
  - `ThrusterEmergency_PreRunCheck()`
- Thruster Run 중 감시 함수:
  - `ThrusterEmergency_RunMonitor()`
- Pre-run check 호출 시 debug message를 남긴다.
- Run monitor debug message는 1 Hz로 제한했다.

## 7. PAR 제어

- `PAR_START`는 Normal mode에서 허용한다.
- `PAR_STOP`은 안전 목적상 모든 모드에서 허용한다.
- PAR routine은 start 이후 stop 전까지 mode와 무관하게 주기적으로 호출된다.
- PAR stop 시 관련 밸브 `SV-F1`, `SV-F2`, `SV-O1`, `SV-O2`를 모두 닫도록 했다.
- PAR 알고리즘은 산화제와 연료 양쪽에 반영했다.
- PAR 기준 PT는 start TC 파라미터로 선택한다:
  - 산화제: `PT-O3` 또는 `PT-O4`
  - 연료: `PT-F3` 또는 `PT-F4`
- PAR 동작 확인용 debug message를 1 Hz로 Health TM에 포함한다.

## 8. 센서 변환

- PT 변환식을 적용했다:
  - 일반 PT: `0.5 V -> 0 bar`, `4.5 V -> 100 bar`
  - `PT-C1`: `0.5 V -> 0 bar`, `4.5 V -> 16 bar`
- TC 측정은 K 타입 thermocouple 변환을 사용한다.
- CJC는 10 kOhm NTC, `B=3936 K` 조건으로 수정했다.
- TC 결과는 Kelvin 기준으로 firmware TM에 포함하고, GUI에서는 `K (C)` 형태로 표시한다.
- GUI 표시 예: `278.136 K (4.986 C)`

## 9. SIM Sensor Override

- 기존 `sim_mode` 방식은 제거했다.
- 별도 mode를 추가하지 않고 sensor override 방식으로 변경했다.
- `SIM_START`는 Normal mode와 Diagnostic mode에서만 허용한다.
- `SIM_STOP`은 모든 mode에서 허용한다.
- `SIM_START` 직후 override 초기값은 가장 최근 실제 센서 측정값으로 설정한다.
- `SIM_START`와 `SIM_STOP` 사이에서는 실제 센서 업데이트를 무시한다.
- `SET_SIM_SENSOR_VALUES` TC로 입력된 PT bar 및 TC K 값이 즉시 센서 스냅샷에 반영된다.
- override 중에도 Thruster/PAR의 기존 제어 루틴은 동일한 센서 getter를 사용하므로, 제어 코드 경로를 분리하지 않고 시험할 수 있다.

## 10. GUI 변경

- GUI를 CSP/RS485 바이너리 프로토콜 기준으로 전환했다.
- Connection/status/health/sensor/output/control 영역을 재배치했다.
- Sensor raw와 변환값을 같은 박스에서 함께 표시하도록 정리했다.
- TC 온도는 Kelvin과 Celsius를 함께 표시한다.
- TM request 버튼은 sensor/health/status 영역 아래에 배치했다.
- SV state 버튼은 Set Outputs 왼쪽에 배치했다.
- `Set LPV` 별도 버튼은 제거했다.
- Thruster Start 파라미터 입력 UI를 추가했다.
- PAR pressure sensor selector UI를 추가했다.
- SIM START / SET SIM SENSORS / SIM STOP UI를 추가했다.
- CSP Ping / Reset 버튼을 추가했다.

## 11. 최근 검증 상태

- GUI 문법 검사:
  - `python -m py_compile tools\psc_uart_monitor_gui.py` 성공
- whitespace 검사:
  - `git diff --check` 성공
- MPLAB make build:
  - `make -f Makefile CONF=default build` 성공
- 빌드 중 `dbg_task.c`의 기존 타입 관련 warning은 남아 있으나, 현재 변경으로 인한 build fail은 없다.

## 12. 남은 주의 사항

- TMTC 정의 문서는 현재 코드의 port/opcode/field 의미와 맞춰 계속 갱신해야 한다.
- Thruster Start TC의 타이밍 필드 의미는 현재 `SV-O3/SV-F3` 기준이다.
- Diagnostic mode 직접 HPV 제어와 Thruster/PAR 내부 HPV 제어의 권한 차이를 시험 절차에 명확히 반영해야 한다.
- SIM sensor override 사용 후에는 반드시 `SIM_STOP`으로 실제 센서 업데이트 경로를 복구해야 한다.
