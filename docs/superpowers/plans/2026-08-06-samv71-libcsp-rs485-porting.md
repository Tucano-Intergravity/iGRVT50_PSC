# SAMV71 libcsp + csp-rs485 Porting Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** ATSAMV71 firmware의 USART1 ASCII `uartcomm` 계층을 공식 libcsp v1.6, 검증된 csp-rs485 공통 코어, SAMV71 RS485 포트, Binary CSP v1 서비스로 완전히 교체한다.

**Architecture:** USART1 ISR은 수신 바이트와 fault만 csp-rs485 FreeRTOS stream에 전달하고, link worker가 KISS/CRC32/recovery를 담당한다. libcsp router와 단일 `CSP_ANY` service task가 packet을 전달하며, bytewise codec과 domain adapter가 포트 10/11/12 요청을 기존 valve/state/sensor API로 연결한다. USART1은 cutover 이후 이 스택만 소유한다.

**Tech Stack:** ATSAMV71Q21B, MPLAB X/Harmony 3, XC32 v5.10 C17, FreeRTOS 10.5.1, libcsp v1.6 (`87006959696c78f70535ab382b0bcd4cb5a6558d`), csp-rs485 `1.0.0` (`56addf6e936e78e3090b43ef5c3c8d60542f3b94`), Python 3.11, GCC 기반 Linux CI host tests.

## Global Constraints

- 승인 설계는 `docs/superpowers/specs/2026-08-06-samv71-libcsp-rs485-porting-design.md`이다. wire length, field offset, status/detail, address, task priority, memory limit을 바꾸지 않는다.
- `third_party/libcsp`는 공식 저장소의 정확한 v1.6 commit을 가리키는 submodule이어야 한다. csp-rs485 안의 nested libcsp는 사용하지 않는다.
- csp-rs485 공통 파일을 복사하기 전에 저장소 소유권 또는 사용·복사·재배포 권한의 서면 근거를 `UPSTREAM.md`에 기록한다. 근거가 없으면 Task 1에서 중단하며 파일을 복사하지 않는다.
- csp-rs485 공통 파일은 upstream과 byte-identical 상태로 유지한다. SAMV71 전용 변경은 `sam_ctl.X/iGRVT50/source/csp` 아래에만 둔다.
- csp-rs485 KISS interface가 packet마다 CRC32C를 직접 한 번 append/verify한다. libcsp connection에 `CSP_O_CRC32` 또는 `CSP_FCRC32`를 추가하면 CRC가 중복되므로 사용하지 않는다. 골든 벡터의 CSP header flags는 0이다.
- 신규 wire code에서는 `UInt32`/`SInt32`를 쓰지 않고 `<stdint.h>`의 `uint*_t`/`int*_t`만 사용한다. native struct image, float, enum image, `__attribute__((packed))` 전송은 금지한다.
- USART1 interrupt priority는 7로 유지하고 ISR에서는 blocking, allocation, KISS parsing, actuator 접근을 하지 않는다.
- DE/nRE는 모든 실패 경로와 idle에서 `DE=0`, `nRE=0`이어야 한다.
- production source 목록의 기준은 `sam_ctl.X/nbproject/configurations.xml`이다. `cmake/sam_ctl/default/.generated`는 MPLAB export 결과로만 갱신하고 직접 편집하지 않는다.
- 각 task의 test가 실패하는 상태를 먼저 확인하고 최소 구현으로 통과시킨다. hardware가 필요한 check는 host/target build가 모두 green인 뒤 수행한다.
- 기존 USART1 legacy 계층은 새 runtime이 target에서 초기화되고 host tests가 통과한 뒤에만 삭제한다.

## File and Responsibility Map

### New production files

- `.gitmodules`, `third_party/libcsp/`: 공식 libcsp v1.6 gitlink.
- `third_party/csp-rs485/UPSTREAM.md`: 권한, provenance, commit, 파일 manifest, SHA-256, sync 절차.
- `third_party/csp-rs485/include/{csp_rs485_link.h,csp_rs485_port.h,csp_rs485_profile.h}`: upstream public headers.
- `third_party/csp-rs485/src/{csp_rs485_internal.h,csp_rs485_freertos.c,csp_rs485_kiss.c,csp_rs485_link.c,csp_rs485_supervisor.c}`: upstream 공통 구현.
- `config/libcsp/samv71/include/csp/csp_autoconfig.h`: libcsp v1.6 SAMV71 compile profile.
- `sam_ctl.X/iGRVT50/header/csp/sam_csp_config.h`: node, port, size, task, timeout compile-time constants.
- `sam_ctl.X/iGRVT50/header/csp/sam_csp_protocol.h`: Binary v1 enums, typed request/response models, codec API.
- `sam_ctl.X/iGRVT50/header/csp/sam_csp_domain.h`: 기존 application과 protocol 사이의 fixed-width adapter API.
- `sam_ctl.X/iGRVT50/header/csp/sam_csp_service.h`: packet dispatcher와 service stats API.
- `sam_ctl.X/iGRVT50/header/csp/sam_csp_runtime.h`: startup result, ready state, debug health API.
- `sam_ctl.X/iGRVT50/header/csp/samv71_rs485_port.h`: csp-rs485 port ops/context getter.
- `sam_ctl.X/iGRVT50/source/csp/sam_csp_codec.c`: endian-safe decode/encode와 exact validation.
- `sam_ctl.X/iGRVT50/source/csp/sam_csp_domain.c`: outputs/mode/snapshot application adapter.
- `sam_ctl.X/iGRVT50/source/csp/sam_csp_service.c`: custom service dispatch와 libcsp packet lifecycle.
- `sam_ctl.X/iGRVT50/source/csp/sam_csp_runtime.c`: csp/link/route/socket 및 static router/service tasks.
- `sam_ctl.X/iGRVT50/source/csp/samv71_rs485_port_internal.h`: injectable low-level hardware operations.
- `sam_ctl.X/iGRVT50/source/csp/samv71_rs485_port.c`: USART1, GPIO, ISR hooks, TX deadline, recovery operations.
- `tools/verify_csp_vendor.py`: submodule commit, imported file manifest와 hashes 검증.
- `tools/verify_harmony_csp_contract.py`: USART1 weak hooks, NVIC priority, GPIO boot-safe generated contract 검증.
- `tools/sam_csp_peer.py`, `tools/requirements-csp-peer.txt`: Windows/Linux explicit-device CSP Binary peer.
- `tests/rs485_peer/golden_vectors.json`: application payload, raw CSP/KISS, CRC32C 골든 벡터.

### New test/CI files

- `tests/host/Makefile`: Linux GCC/ASan/UBSan host build.
- `tests/host/config/include/csp/csp_autoconfig.h`: libcsp POSIX host profile.
- `tests/host/support/{test.h,test.c,host_csp.h,host_csp.c}`: test runner and libcsp fixture.
- `tests/host/fakes/{fake_port.h,fake_port.c,fake_domain.h,fake_domain.c}`: link/service dependencies.
- `tests/host/fakes/freertos/`: upstream FreeRTOS fakes required by csp-rs485 core tests.
- `tests/host/fakes/samv71/{fake_samv71_hw.h,fake_samv71_hw.c}`: register/time/direction event recorder.
- `tests/host/test_{main,host_profile,kiss_encoder,link_rx,link_tx,supervisor,freertos_runtime}.c`: imported/adapted common-core regression tests.
- `tests/host/test_sam_csp_codec.c`: wire schema/length/endian/error vectors.
- `tests/host/test_sam_csp_domain.c`: actuator ordering, validity, coherent snapshot adapter.
- `tests/host/test_sam_csp_service.c`: port/opcode dispatch and packet ownership.
- `tests/host/test_samv71_rs485_port.c`: init/RX/fault/TX/deadline/cleanup sequencing.
- `tests/python/test_sam_csp_peer.py`: peer encoding/decoding and golden-vector parity.
- `.github/workflows/csp-host-tests.yml`: checkout with submodule, vendor verification, normal and sanitizer tests.

### Existing files to modify

- `src/config/default/FreeRTOSConfig.h`: static allocation, stack watermark, task lookup.
- `src/config/default/freertos_hooks.c`: idle task static memory provider.
- `src/config/default/pin_configurations.csv`: PA22/PA24 GPIO output low intent.
- `src/config/default/peripheral/pio/plib_pio.c`: PA22/PA24 boot-safe output-low generated result.
- `src/config/default/peripheral/pio/plib_pio.h`: named DE/nRE GPIO macros.
- `src/config/default/peripheral/usart/plib_usart1.c`: weak-hook ABI 보존 여부만 검증; custom logic 추가 금지.
- `sam_ctl.X/iGRVT50/header/sensor.h`, `sam_ctl.X/iGRVT50/source/sensor.c`: coherent PT/TC snapshot과 validity counters.
- `src/opu_task.c`: TC publish, RsTask/ASCII 제거, CSP runtime startup.
- `src/dbg_task.c`: legacy UART commands를 CSP health command로 교체.
- `src/sam_ctl.h`: legacy RS422/RS485 declarations와 legacy ring type 제거.
- `sam_ctl.X/nbproject/configurations.xml`: include/source graph에 libcsp, csp-rs485, SAM CSP files 반영.
- `cmake/sam_ctl/default/.generated/*`: MPLAB CMake export로만 재생성.
- `docs/evidence/samv71-csp-rs485-verification.md`: build, memory, scope, stress, recovery evidence.

### Files to delete after cutover

- `sam_ctl.X/iGRVT50/header/uartcomm.h`
- `sam_ctl.X/iGRVT50/source/uartcomm.c`
- `src/rs422_func.c`

---

## Task 1: Freeze the Baseline and Pass the Dependency Authorization Gate

**Files:**

- Create: `docs/evidence/samv71-csp-rs485-verification.md`

- [ ] Record the current revision, clean/dirty state, XC32 version, and baseline ELF/map sizes in the evidence document before changing production code.

Run from the repository root:

```powershell
git rev-parse HEAD
git status --short
& 'C:\Program Files\Microchip\MPLABX\v6.30\gnuBins\GnuWin32\bin\make.exe' -C sam_ctl.X -f Makefile CONF=default build
& 'C:\Program Files\Microchip\MPLABX\v6.30\gnuBins\GnuWin32\bin\make.exe' -C sam_ctl.X -f Makefile CONF=default TYPE_IMAGE=DEBUG_RUN build
Get-Item sam_ctl.X/dist/default/production/sam_ctl.X.production.hex
Get-Item sam_ctl.X/dist/default/production/sam_ctl.X.production.map
Get-Item sam_ctl.X/dist/default/debug/sam_ctl.X.debug.elf
```

Expected: both make invocations exit 0 and all three artifacts exist. If baseline compilation fails, record the exact existing failure and repair it in a separate commit before CSP work.

- [ ] Put the csp-rs485 authorization evidence in the evidence document: repository owner, authorization source, covered paths, and confirmation that copying and redistribution inside this firmware repository are allowed.
- [ ] Stop without copying csp-rs485 if that evidence cannot be supplied. This is a hard gate, not a deferred compliance item.
- [ ] Commit the evidence gate.

```powershell
git add docs/evidence/samv71-csp-rs485-verification.md
git commit -m "docs: approve CSP RS485 porting baseline"
```

## Task 2: Pin libcsp v1.6 and Import the Authorized csp-rs485 Core

**Files:**

- Create: `.gitmodules`, `third_party/libcsp/`
- Create: `third_party/csp-rs485/UPSTREAM.md`
- Create: the eight csp-rs485 files listed in the file map
- Create: `tools/verify_csp_vendor.py`

- [ ] Add the official libcsp submodule and detach it at the approved commit.

```powershell
git submodule add https://github.com/libcsp/libcsp.git third_party/libcsp
git -C third_party/libcsp checkout 87006959696c78f70535ab382b0bcd4cb5a6558d
git -C third_party/libcsp describe --tags --exact-match
git -C third_party/libcsp rev-parse HEAD
```

Expected:

```text
v1.6
87006959696c78f70535ab382b0bcd4cb5a6558d
```

- [ ] Copy only the approved common files; do not copy STM32 ports, CubeMX files, or nested libcsp.

```powershell
New-Item -ItemType Directory -Force third_party/csp-rs485/include,third_party/csp-rs485/src | Out-Null
Copy-Item C:\PSC\csp-rs485\csp_rs485\include\csp_rs485_link.h third_party/csp-rs485/include/
Copy-Item C:\PSC\csp-rs485\csp_rs485\include\csp_rs485_port.h third_party/csp-rs485/include/
Copy-Item C:\PSC\csp-rs485\csp_rs485\include\csp_rs485_profile.h third_party/csp-rs485/include/
Copy-Item C:\PSC\csp-rs485\csp_rs485\src\csp_rs485_internal.h third_party/csp-rs485/src/
Copy-Item C:\PSC\csp-rs485\csp_rs485\src\csp_rs485_freertos.c third_party/csp-rs485/src/
Copy-Item C:\PSC\csp-rs485\csp_rs485\src\csp_rs485_kiss.c third_party/csp-rs485/src/
Copy-Item C:\PSC\csp-rs485\csp_rs485\src\csp_rs485_link.c third_party/csp-rs485/src/
Copy-Item C:\PSC\csp-rs485\csp_rs485\src\csp_rs485_supervisor.c third_party/csp-rs485/src/
```

- [ ] Write `UPSTREAM.md` with source path, tag `1.0.0`, commit `56addf6e936e78e3090b43ef5c3c8d60542f3b94`, nested libcsp observation, the eight-file manifest, authorization evidence reference, per-file SHA-256, upstream test command, and sync procedure.
- [ ] Implement `verify_csp_vendor.py` with Python standard library only. It must fail unless the libcsp gitlink is the exact commit, each manifest path exists, each copied file hash matches `UPSTREAM.md`, no extra `.c/.h` exists below `third_party/csp-rs485`, and `git diff --no-index` against `C:\PSC\csp-rs485` is empty when that checkout is present.
- [ ] Run the verifier.

```powershell
python tools/verify_csp_vendor.py
```

Expected: `PASS libcsp=87006959696c78f70535ab382b0bcd4cb5a6558d csp-rs485=8 files` and exit 0.

- [ ] Commit dependency intake.

```powershell
git add .gitmodules third_party tools/verify_csp_vendor.py
git commit -m "chore: pin libcsp and import csp-rs485 core"
```

## Task 3: Add the Host Regression Harness and CI

**Files:**

- Create: `tests/host/Makefile`
- Create: `tests/host/config/include/csp/csp_autoconfig.h`
- Create: common-core test/support/fake files listed in the file map
- Create: `.github/workflows/csp-host-tests.yml`

- [ ] Import the upstream host support, fake FreeRTOS, fake port, and these test suites with provenance comments: `host_profile`, `kiss_encoder`, `link_rx`, `link_tx`, `supervisor`, `freertos_runtime`. Exclude `dma_cursor` and `stm32_port` because their production sources are not imported.
- [ ] Adapt `tests/host/Makefile` so source roots are this repository's `third_party/libcsp` and `third_party/csp-rs485`. Compile project code with `-std=c11 -Wall -Wextra -Werror -Wconversion -Wshadow -Wstrict-prototypes`; compile libcsp with `-std=gnu11` and no `-Werror`.
- [ ] Make the default host test fail before the Makefile paths are fixed.

```sh
make -C tests/host test
```

Expected initial failure: compiler cannot find `csp_rs485_link.h` or a vendored source path.

- [ ] Fix the include/source graph and run normal plus sanitizer builds on Linux.

```sh
make -C tests/host clean test CC=gcc
make -C tests/host clean test CC=gcc SANITIZE=1
```

Expected: every imported test prints `PASS`; final line reports `0 failures`; ASan/UBSan reports no diagnostic.

- [ ] Add CI with `actions/checkout@v4` and `submodules: recursive`, Python vendor verification, the two make commands above, and Python unit-test discovery. Do not add a push/deploy step.
- [ ] Commit the harness.

```sh
git add tests/host .github/workflows/csp-host-tests.yml
git commit -m "test: add CSP RS485 host regression harness"
```

## Task 4: Define and Test the Binary v1 Codec

**Files:**

- Create: `sam_ctl.X/iGRVT50/header/csp/sam_csp_protocol.h`
- Create: `sam_ctl.X/iGRVT50/source/csp/sam_csp_codec.c`
- Create: `tests/host/test_sam_csp_codec.c`
- Modify: `tests/host/test_main.c`, `tests/host/Makefile`

- [ ] Define all constants with compile-time length assertions. The public API must use fixed-width models rather than packed wire structs:

```c
typedef enum {
    SAM_CSP_STATUS_OK = 0,
    SAM_CSP_STATUS_BAD_VERSION = 1,
    SAM_CSP_STATUS_BAD_LENGTH = 2,
    SAM_CSP_STATUS_BAD_OPCODE = 3,
    SAM_CSP_STATUS_INVALID_ARGUMENT = 4,
    SAM_CSP_STATUS_INVALID_STATE = 5,
    SAM_CSP_STATUS_APPLY_FAILED = 6,
    SAM_CSP_STATUS_INTERNAL_ERROR = 7,
    SAM_CSP_STATUS_BUSY = 8,
} sam_csp_status_t;

typedef struct {
    uint16_t transaction_id;
    uint16_t lpv_on_mask;
    uint8_t hpv_on_mask;
    uint8_t heater_on_mask;
    uint8_t spark_on;
} sam_csp_set_outputs_request_t;

typedef struct {
    uint32_t sample_time_ms;
    uint8_t current_mode;
    uint8_t requested_mode;
    uint16_t validity_mask;
    int32_t pt_millivolt[9];
    int32_t tc_microvolt[4];
} sam_csp_snapshot_t;

sam_csp_status_t sam_csp_decode_set_outputs(
    const uint8_t *data, size_t length,
    sam_csp_set_outputs_request_t *request, uint8_t *detail);
size_t sam_csp_encode_status(
    uint8_t opcode, uint16_t transaction_id,
    sam_csp_status_t status, uint8_t detail,
    uint8_t *output, size_t capacity);
size_t sam_csp_encode_snapshot(
    uint8_t opcode, uint16_t transaction_id,
    const sam_csp_snapshot_t *snapshot,
    uint8_t *output, size_t capacity);
```

- [ ] Write failing tests for these request vectors and exact six-byte responses:

```text
valid SET_OUTPUTS: 01 01 12 34 0A 55 A5 09 01 00
OK response:       01 01 12 34 00 00
bad version:       02 01 12 34 -> 01 01 12 34 01 00
bad length 9:      detail=0A
bad opcode 7F:     status=03 detail=7F
LPV upper bits:    status=04 detail=04
heater upper bits: status=04 detail=07
spark value 2:     status=04 detail=08
reserved value 1: status=04 detail=09
```

- [ ] Add GET_SNAPSHOT and GET_HEALTH golden tests that assert every byte, signed negative values, BE conversion, exact success lengths 66/58, and no output write when capacity is too small.
- [ ] Add a decoder test proving packets shorter than four bytes return a distinct `SAM_CSP_DECODE_DROP` result so the service cannot fabricate a transaction ID.
- [ ] Run the focused test and confirm the new suite fails before codec implementation.

```sh
make -C tests/host test TEST=sam_csp_codec
```

Expected initial result: at least one `FAIL sam_csp_codec` and nonzero exit.

- [ ] Implement bytewise `get_be16`, `put_be16`, `put_be32`, `put_be_i32` helpers. Validate in the order header length, version, opcode, exact length, field values; never cast the payload to a struct.
- [ ] Re-run focused and full host tests.

```sh
make -C tests/host test TEST=sam_csp_codec
make -C tests/host test
```

Expected: codec suite and full suite report `0 failures`.

- [ ] Commit the wire codec.

```sh
git add sam_ctl.X/iGRVT50/header/csp/sam_csp_protocol.h sam_ctl.X/iGRVT50/source/csp/sam_csp_codec.c tests/host
git commit -m "feat: add binary CSP v1 codec"
```

## Task 5: Add a Coherent Sensor/Actuator Domain Adapter

**Files:**

- Create: `sam_ctl.X/iGRVT50/header/csp/sam_csp_domain.h`
- Create: `sam_ctl.X/iGRVT50/source/csp/sam_csp_domain.c`
- Create: `tests/host/test_sam_csp_domain.c`
- Modify: `sam_ctl.X/iGRVT50/header/sensor.h`
- Modify: `sam_ctl.X/iGRVT50/source/sensor.c`
- Modify: `src/opu_task.c`
- Modify: `tests/host/test_main.c`, `tests/host/Makefile`

- [ ] Define a small domain contract:

```c
typedef enum {
    SAM_CSP_DOMAIN_OK = 0,
    SAM_CSP_DOMAIN_INVALID_STATE,
    SAM_CSP_DOMAIN_APPLY_FAILED,
    SAM_CSP_DOMAIN_SNAPSHOT_FAILED,
} sam_csp_domain_result_t;

sam_csp_domain_result_t sam_csp_domain_apply_outputs(
    const sam_csp_set_outputs_request_t *request);
sam_csp_domain_result_t sam_csp_domain_request_mode(uint8_t mode);
sam_csp_domain_result_t sam_csp_domain_get_snapshot(
    sam_csp_snapshot_t *snapshot);
```

- [ ] Write failing fake-backed tests asserting that invalid/null inputs produce no actuator calls, valid outputs call LPV 1..12 then HPV 1..8 then heater 1..4 then spark once, every bit maps to an absolute state, and a rejected `StateMachine_RequestMode()` maps to `APPLY_FAILED`.
- [ ] Replace live TC reads in `Sensor_GetScan()` with a published integer snapshot. Add `Sensor_UpdateTcRawScan(const int32_t *, uint8_t)`, `Sensor_GetTcScanCount()`, and copy PT/TC raw arrays and both counters under a short FreeRTOS critical section.
- [ ] In `TcTask`, collect `ADS1263_GetRawCode(1, 0..3)` after the four channel conversions and publish exactly once per completed scan. Until each counter is nonzero, the domain adapter must emit zero values and clear its corresponding validity bits.
- [ ] Implement the production mapping with `LpSolValve_Set`, `HpSolValve_Set`, `Heater_SetDuty`, `SparkPlug_Set`, `StateMachine_RequestMode`, `StateMachine_GetSnapshot`, `Sensor_GetScan`, and `xTaskGetTickCount`. Validate the complete outputs object before the first hardware call.
- [ ] Run focused then full host tests.

```sh
make -C tests/host test TEST=sam_csp_domain
make -C tests/host test
```

Expected: `0 failures`; the valid snapshot has PT validity bits 0..8 and TC bits 9..12 only after their respective first publishes.

- [ ] Commit the adapter and coherent sensor publication.

```sh
git add sam_ctl.X/iGRVT50/header/csp/sam_csp_domain.h sam_ctl.X/iGRVT50/source/csp/sam_csp_domain.c sam_ctl.X/iGRVT50/header/sensor.h sam_ctl.X/iGRVT50/source/sensor.c src/opu_task.c tests/host
git commit -m "feat: add CSP domain and coherent sensor snapshot"
```

## Task 6: Integrate libcsp and the FreeRTOS Build Profile

**Files:**

- Create: `config/libcsp/samv71/include/csp/csp_autoconfig.h`
- Create: `sam_ctl.X/iGRVT50/header/csp/sam_csp_config.h`
- Modify: `src/config/default/FreeRTOSConfig.h`
- Modify: `src/config/default/freertos_hooks.c`
- Modify: `sam_ctl.X/nbproject/configurations.xml`
- Regenerate: `sam_ctl.X/nbproject/Makefile-default.mk`, `cmake/sam_ctl/default/.generated/*`

- [ ] Add the approved autoconfig flags exactly: FreeRTOS on, POSIX/Windows/macOS off, CRC32 on, RDP/HMAC/XTEA/QOS/DEDUP off, little endian, only error logging enabled.
- [ ] Define node 1, peer 2, ports 10/11/12, connections 4, connection queue 10, FIFO 25, buffers 20, data 300, max bind 12, router/link/service priorities 4/3/2, each stack 512 words, TX margin 5 ms, recovery retry 25 ms.
- [ ] Add compile-time guards:

```c
_Static_assert(SAM_CSP_LOCAL_ADDRESS != SAM_CSP_PEER_ADDRESS,
    "CSP local and peer addresses must differ");
_Static_assert(SAM_CSP_DIAGNOSTIC_PORT == 12U,
    "CSP max bind must cover diagnostics");
_Static_assert(ATOMIC_BOOL_LOCK_FREE == 2,
    "CSP ISR atomics must be lock-free");
_Static_assert(ATOMIC_INT_LOCK_FREE == 2,
    "CSP 32-bit atomics must be lock-free");
_Static_assert(ATOMIC_LONG_LOCK_FREE == 2,
    "CSP long atomics must be lock-free");
_Static_assert(sizeof(uint_fast32_t) == 4U,
    "CSP health atomics require 32-bit uint_fast32_t");
```

- [ ] Set `configSUPPORT_STATIC_ALLOCATION=1`, `INCLUDE_uxTaskGetStackHighWaterMark=1`, and `INCLUDE_xTaskGetHandle=1`; leave dynamic allocation, 40,960-byte heap, and software timers unchanged.
- [ ] Add idle task storage to `freertos_hooks.c`:

```c
void vApplicationGetIdleTaskMemory(
    StaticTask_t **task_tcb,
    StackType_t **task_stack,
    uint32_t *stack_size)
{
    static StaticTask_t idle_tcb;
    static StackType_t idle_stack[configMINIMAL_STACK_SIZE];
    *task_tcb = &idle_tcb;
    *task_stack = idle_stack;
    *stack_size = configMINIMAL_STACK_SIZE;
}
```

- [ ] Add include roots `../config/libcsp/samv71/include`, `../third_party/libcsp/include`, `../third_party/csp-rs485/include`, `../third_party/csp-rs485/src`, and `iGRVT50/header/csp` to both compiler configurations in `configurations.xml`.
- [ ] Add the csp-rs485 eight sources and these libcsp groups to the production source graph: `src/*.c`, `src/transport/*.c`, `src/crypto/*.c`, `src/interfaces/*.c`, `src/arch/*.c`, `src/arch/freertos/*.c`, `src/rtable/csp_rtable.c`, and `src/rtable/csp_rtable_static.c`. Do not include POSIX/Windows/macOS, drivers, Python bindings, examples, or `csp_rtable_cidr.c`.
- [ ] Export the MPLAB project to CMake so the generated mirror uses XC32 v5.10 and contains the same source graph. Verify no tracked manual change was made under `.generated` before export.
- [ ] Build production and debug images.

```powershell
& 'C:\Program Files\Microchip\MPLABX\v6.30\gnuBins\GnuWin32\bin\make.exe' -C sam_ctl.X -f Makefile CONF=default build
& 'C:\Program Files\Microchip\MPLABX\v6.30\gnuBins\GnuWin32\bin\make.exe' -C sam_ctl.X -f Makefile CONF=default TYPE_IMAGE=DEBUG_RUN build
```

Expected: both exit 0; no undefined `csp_*`, `xTaskCreateStatic`, `vApplicationGetIdleTaskMemory`, or atomic helper symbol.

- [ ] Commit build integration.

```powershell
git add config/libcsp sam_ctl.X/iGRVT50/header/csp/sam_csp_config.h src/config/default/FreeRTOSConfig.h src/config/default/freertos_hooks.c sam_ctl.X/nbproject cmake/sam_ctl/default
git commit -m "build: integrate libcsp and static CSP runtime support"
```

## Task 7: Implement the SAMV71 RS485 Port with Host-Driven TDD

**Files:**

- Create: `sam_ctl.X/iGRVT50/header/csp/samv71_rs485_port.h`
- Create: `sam_ctl.X/iGRVT50/source/csp/samv71_rs485_port_internal.h`
- Create: `sam_ctl.X/iGRVT50/source/csp/samv71_rs485_port.c`
- Create: `tests/host/fakes/samv71/fake_samv71_hw.h`
- Create: `tests/host/fakes/samv71/fake_samv71_hw.c`
- Create: `tests/host/test_samv71_rs485_port.c`
- Modify: `tests/host/test_main.c`, `tests/host/Makefile`

- [ ] Define an injected hardware table used by the production singleton and host fake:

```c
typedef struct {
    uint32_t (*status)(void);
    uint8_t (*read_byte)(void);
    void (*write_byte)(uint8_t value);
    void (*set_direction_tx)(bool transmit);
    void (*set_rx_irq)(bool enabled);
    void (*clear_pending_irq)(void);
    void (*reset_status_and_flush)(void);
    void (*reset_rx)(void);
    void (*reset_tx)(void);
    uint32_t (*now_ms)(void);
    void (*delay_one_bit)(void);
} samv71_rs485_hw_ops_t;
```

- [ ] Write failing tests for: receive-safe initialization; arm/enable order; pre-init hooks return false without touching link state; RX hook forwards every byte; UART error marks discontinuity and reports `CSP_RS485_FAULT_UART`; whole-frame TX event order; timeout at TXRDY and TXEMPTY; null/zero arguments; all failures ending in receive mode; deinit/recovery idempotence.
- [ ] Assert the successful TX event trace is exactly:

```text
RX_IRQ_OFF, NRE_HIGH, DE_HIGH, GUARD_1BIT,
WRITE(each byte after TXRDY), WAIT_TXEMPTY,
DE_LOW, NRE_LOW, RX_IRQ_ON
```

- [ ] Implement all `csp_rs485_port_ops_t` operations. Configure USART1 to MCK, 8N1, OVER=0, BRGR `CD=10, FP=1`; clear RX errors and RHR before arming.
- [ ] Implement the guard with Cortex-M7 DWT cycle count using `ceil(SystemCoreClock / 921600)` cycles. The fake must verify one call, while hardware scope verification measures the actual duration.
- [ ] Use one absolute deadline for the entire frame. On TXRDY/TXEMPTY expiry reset TX, force receive, restore RX interrupt state, and return `CSP_RS485_PORT_TIMEOUT`; never retry a partial frame.
- [ ] Strong-define `USART1_UartCommRxReadyHook()` and `USART1_UartCommErrorHook(uint32_t)` in the port. The RX hook drains RHR and calls `csp_rs485_freertos_rx_from_isr`; the error hook clears status/RHR, marks a discontinuity, and calls `csp_rs485_link_report_fault_from_isr(CSP_RS485_FAULT_UART)`.
- [ ] Run focused, full, and sanitizer host tests.

```sh
make -C tests/host test TEST=samv71_rs485_port
make -C tests/host test
make -C tests/host clean test SANITIZE=1
```

Expected: all report `0 failures`; sanitizer output is clean.

- [ ] Keep the new port out of `sam_ctl.X/nbproject/configurations.xml` in this task. The legacy `uartcomm.c` still strong-defines the same hook ABI; production wiring happens atomically with legacy removal in Task 10.

- [ ] Commit the port.

```powershell
git add sam_ctl.X/iGRVT50/header/csp/samv71_rs485_port.h sam_ctl.X/iGRVT50/source/csp tests/host
git commit -m "feat: add SAMV71 interrupt RS485 port"
```

## Task 8: Make the RS485 Direction Pins Boot-Safe and Guard Generated Code

**Files:**

- Modify: `src/config/default/pin_configurations.csv`
- Modify: `src/config/default/peripheral/pio/plib_pio.c`
- Modify: `src/config/default/peripheral/pio/plib_pio.h`
- Verify only: `src/config/default/peripheral/usart/plib_usart1.c`
- Create: `tools/verify_harmony_csp_contract.py`

- [ ] Add a failing verifier test first. It must assert PA22 and PA24 are GPIO output low in the CSV, `PIO_Initialize()` enables both outputs before any CSP task, the USART1 weak hooks exist, and `NVIC_SetPriority(USART1_IRQn, 7)` remains present.

```powershell
python tools/verify_harmony_csp_contract.py
```

Expected initial failure: PA22 direction is `In` and PA24 function is `USART1_RTS1`.

- [ ] Change PA22 `UART1_DE` and PA24 `UART1_nRE` to GPIO, `Out`, latch `Low` in the pin intent CSV.
- [ ] Regenerate or reproduce the PIO result so PORTA `PIO_OER` contains bits 22 and 24 and `PIO_ODSR` clears them. Add `UART1_DE_*` and `UART1_nRE_*` macros in `plib_pio.h`.
- [ ] Do not alter the generated USART handler logic; preserve its weak-hook ABI. Run the verifier after a Harmony regeneration and reject any generated diff that removes those hooks or boot-safe GPIOs.
- [ ] Run the verifier and target builds.

```powershell
python tools/verify_harmony_csp_contract.py
& 'C:\Program Files\Microchip\MPLABX\v6.30\gnuBins\GnuWin32\bin\make.exe' -C sam_ctl.X -f Makefile CONF=default build
```

Expected: `PASS Harmony CSP contract` and build exit 0.

- [ ] With a logic analyzer attached, reset the board without sending traffic. Record that DE and nRE remain low from pin initialization through application startup. Repeat the CSP-specific timing capture after Task 10 in Task 12.
- [ ] Commit pin ownership.

```powershell
git add src/config/default/pin_configurations.csv src/config/default/peripheral/pio tools/verify_harmony_csp_contract.py docs/evidence/samv71-csp-rs485-verification.md
git commit -m "fix: make RS485 direction control boot safe"
```

## Task 9: Implement CSP Runtime and Binary Service Dispatch

**Files:**

- Create: `sam_ctl.X/iGRVT50/header/csp/sam_csp_service.h`
- Create: `sam_ctl.X/iGRVT50/source/csp/sam_csp_service.c`
- Create: `sam_ctl.X/iGRVT50/header/csp/sam_csp_runtime.h`
- Create: `sam_ctl.X/iGRVT50/source/csp/sam_csp_runtime.c`
- Create: `tests/host/fakes/fake_domain.h`, `tests/host/fakes/fake_domain.c`
- Create: `tests/host/test_sam_csp_service.c`
- Modify: `tests/host/test_main.c`, `tests/host/Makefile`

- [ ] Keep packet dispatch independent of the blocking CSP task loop:

```c
typedef enum {
    SAM_CSP_DISPATCH_DROP = 0,
    SAM_CSP_DISPATCH_RESPOND,
    SAM_CSP_DISPATCH_DELEGATE_PING,
} sam_csp_dispatch_action_t;

sam_csp_dispatch_action_t sam_csp_service_dispatch(
    uint8_t source_address,
    uint8_t destination_port,
    const uint8_t *request, size_t request_length,
    uint8_t *response, size_t response_capacity,
    size_t *response_length);
```

- [ ] Write failing service tests for every port/opcode, short-header drop, exact BAD_* detail, fake domain OK/INVALID_STATE/APPLY_FAILED mapping, output immutability on decode failure, snapshot/health length, response-capacity failure, peer mismatch drop, `CSP_PING` delegation, and silent drop of all other libcsp reserved/unknown ports.
- [ ] Implement the service task with `csp_socket(CSP_SO_NONE)`, `csp_bind(socket, CSP_ANY)`, and `csp_listen(socket, 10)`. Accept only node 2. Allocate a distinct response packet with `csp_buffer_get(0)`; free request and response on every unsent path; after successful `csp_send`, do not free the sent packet.
- [ ] When the destination port is exactly `CSP_PING`, pass the original request packet once to `csp_service_handler(connection, packet)` and treat ownership as transferred. For every other libcsp reserved port, free the request without invoking the default handler so remote reboot and identification services remain unavailable.
- [ ] Maintain local service counters for malformed packets, allocation failures, send failures, rejected peers, and dropped ports. These are visible through USART0 `csp` debug output but are not added to the fixed 58-byte GET_HEALTH schema.
- [ ] Implement runtime startup with the exact order below. Leave `conf.conn_dfl_so` at `CSP_O_NONE` because csp-rs485 KISS adds one CRC32C itself.

```c
csp_conf_get_defaults(&conf);
conf.address = 1U;
conf.conn_max = 4U;
conf.conn_queue_length = 10U;
conf.fifo_length = 25U;
conf.port_max_bind = 12U;
conf.buffers = 20U;
conf.buffer_data_size = 300U;
conf.conn_dfl_so = CSP_O_NONE;
```

- [ ] Call `csp_init`, `csp_rs485_link_init`, `csp_route_set(2, iface, CSP_NO_VIA_ADDRESS)`, create a static router task that loops on `csp_route_work(CSP_MAX_TIMEOUT)`, create/bind/listen the socket, then create the static service task. Publish ready only after all succeed.
- [ ] On startup failure, store a stable negative init code, call `csp_rs485_link_deinit` only if the link was established, force receive mode, print the code on USART0, and leave ready false. Do not call `EnterSafeState()` because communication loss is outside the v1 safety policy.
- [ ] Keep runtime/service/port application sources out of the MPLAB source graph until Task 10, so the legacy USART1 hook owner remains buildable during this intermediate commit.
- [ ] Run focused and full host tests.

```sh
make -C tests/host test TEST=sam_csp_service
make -C tests/host test
```

Expected: host `0 failures`.

- [ ] Commit runtime/service.

```powershell
git add sam_ctl.X/iGRVT50/header/csp sam_ctl.X/iGRVT50/source/csp tests/host
git commit -m "feat: add libcsp runtime and binary services"
```

## Task 10: Cut Over OpuTask and Remove the Legacy USART1 Stack

**Files:**

- Modify: `src/opu_task.c`
- Modify: `src/dbg_task.c`
- Modify: `src/sam_ctl.h`
- Modify: `sam_ctl.X/nbproject/configurations.xml`
- Delete: `sam_ctl.X/iGRVT50/header/uartcomm.h`
- Delete: `sam_ctl.X/iGRVT50/source/uartcomm.c`
- Delete: `src/rs422_func.c`

- [ ] In `OpuTask`, finish actuator/sensor boot-safe initialization, call `StateMachine_Init`, create only `TcTask` and `AdcTask`, and then call `SamCspRuntime_Init`. Remove `xRsTask`, all `RSTASK_NOTIFY_*`, ASCII buffers/counters/parsers, telemetry formatters, `RsTask`, `UartComm_Init`, and `UartComm_SetRxNotifyTask`.
- [ ] Replace the USART0 debug command `uart` with `csp`. It must print runtime ready/init code, link state/error/counters, service counters, `xPortGetMinimumEverFreeHeapSize()`, and stack high-watermarks obtained from the router/service handles and `xTaskGetHandle("csp-rs485")`.
- [ ] Remove the raw `rs485 [count] [hexbyte]` debug command because arbitrary USART1 writes violate single ownership. Keep all unrelated USART0 debug commands.
- [ ] Remove `usRs422Loop`, RS422 declarations, `sRbData`, and all `uartcomm.h` includes. Delete the three legacy files and their project entries.
- [ ] In the same `configurations.xml` edit, add every source under `sam_ctl.X/iGRVT50/source/csp` and every public CSP header. This is the first target configuration that links `samv71_rs485_port.c`, so legacy hooks and new hooks never coexist in one image.
- [ ] Prove there is no legacy reference.

```powershell
rg -n 'UartComm_|uartcomm\.h|RsTask|RSTASK_NOTIFY|RS422_Init|RS485_SetTransmit|usRs422Loop|\$iGRVT50' src sam_ctl.X/iGRVT50 sam_ctl.X/nbproject/configurations.xml
```

Expected: no matches and `rg` exit 1.

- [ ] Run vendor/Harmony verification, full host tests, and both target builds.

```powershell
python tools/verify_csp_vendor.py
python tools/verify_harmony_csp_contract.py
& 'C:\Program Files\Microchip\MPLABX\v6.30\gnuBins\GnuWin32\bin\make.exe' -C sam_ctl.X -f Makefile CONF=default build
& 'C:\Program Files\Microchip\MPLABX\v6.30\gnuBins\GnuWin32\bin\make.exe' -C sam_ctl.X -f Makefile CONF=default TYPE_IMAGE=DEBUG_RUN build
```

Expected: both verifiers pass and both builds exit 0.

On Linux:

```sh
make -C tests/host clean test CC=gcc
make -C tests/host clean test CC=gcc SANITIZE=1
```

Expected: normal and sanitizer runs report `0 failures`.

- [ ] Inspect final hook ownership.

```powershell
& 'C:\Program Files\Microchip\xc32\v5.10\bin\xc32-nm.exe' sam_ctl.X/dist/default/production/sam_ctl.X.production.elf | Select-String 'USART1_UartComm(RxReady|Error)Hook'
```

Expected: exactly one strong `T` symbol for each hook and no legacy `UartComm_*` symbol.

- [ ] Commit the irreversible cutover only after all checks pass.

```powershell
git add -A
git commit -m "refactor: replace legacy USART1 command transport with CSP"
```

## Task 11: Add the Binary CSP Peer and Golden Vectors

**Files:**

- Create: `tools/sam_csp_peer.py`
- Create: `tools/requirements-csp-peer.txt`
- Create: `tests/rs485_peer/golden_vectors.json`
- Create: `tests/python/test_sam_csp_peer.py`

- [ ] Implement Python 3.11 bytewise CSP v1 header, CRC32C, KISS escape/unescape, application request/response codecs. Pin only `pyserial==3.5`; `selftest` and unit tests must not import or open serial.
- [ ] Add vectors for SET_OUTPUTS, SET_MODE, GET_SNAPSHOT, GET_HEALTH, bad version, bad length, invalid mask, `0xC0/0xDB` escaping, signed negative sensor values, and CRC corruption. Each vector stores application payload, CSP header with flags 0, CRC, raw frame, and KISS frame.
- [ ] Add explicit commands `set-outputs`, `set-mode`, `snapshot`, `health`, `smoke`, `sequence`, `rx-burst`, and `recovery`. Every transaction uses one write/flush, one outstanding request, a transaction-ID check, no automatic retry, and an explicit device passed by `--device`.
- [ ] Write tests first and observe failures before implementing peer functions.

```powershell
python -m unittest discover -s tests/python -p 'test_*.py' -v
```

Expected initial result: import or assertion failure for `sam_csp_peer`.

- [ ] Implement until unit tests and selftest pass.

```powershell
python -m unittest discover -s tests/python -p 'test_*.py' -v
python tools/sam_csp_peer.py selftest
```

Expected: all tests `ok`; selftest prints `PASS golden vectors`.

- [ ] Commit peer tooling.

```powershell
git add tools/sam_csp_peer.py tools/requirements-csp-peer.txt tests/rs485_peer tests/python
git commit -m "test: add binary CSP RS485 peer and vectors"
```

## Task 12: Perform Target, Electrical, Memory, Stress, and Recovery Acceptance

**Files:**

- Modify: `docs/evidence/samv71-csp-rs485-verification.md`

- [ ] Create an isolated peer environment and install the one pinned serial dependency.

```powershell
python -m venv .superpowers/csp-peer-venv
$samCspPython = (Resolve-Path '.superpowers/csp-peer-venv/Scripts/python.exe').Path
& $samCspPython -m pip install -r tools/requirements-csp-peer.txt
```

Expected: `Successfully installed pyserial-3.5`.

- [ ] Flash the exact debug ELF recorded in the evidence file. Set `SAM_CSP_SERIAL` to the explicitly identified USB-RS485 adapter port before transmitting; do not probe every serial port.
- [ ] Verify one transaction of each service and record request/response hex, transaction ID, latency, and final actuator/state/sensor observation.

```powershell
& $samCspPython tools/sam_csp_peer.py smoke --device $env:SAM_CSP_SERIAL --timeout-seconds 1.0
& $samCspPython tools/sam_csp_peer.py set-outputs --device $env:SAM_CSP_SERIAL --lpv-mask 0x0000 --hpv-mask 0x00 --heater-mask 0x00 --spark 0
& $samCspPython tools/sam_csp_peer.py set-mode --device $env:SAM_CSP_SERIAL --mode 1
& $samCspPython tools/sam_csp_peer.py snapshot --device $env:SAM_CSP_SERIAL
& $samCspPython tools/sam_csp_peer.py health --device $env:SAM_CSP_SERIAL
```

Expected: every response is version 1, matching opcode/transaction ID, status/detail `0/0`, exact payload length, and valid CRC32C. The all-off command leaves every output off.

- [ ] Capture DE/nRE/TX/RX with a logic analyzer. Verify idle and all cleanup have DE=0/nRE=0, pre-TX guard is at least one 921600-baud bit (1.085 microseconds), DE remains asserted through TXEMPTY, and receive mode resumes without a transmitted tail truncation.
- [ ] Run maximum request/reply traffic for 10 minutes. Record total frames, maximum latency, link/service counter deltas, OPU callback deltas, and debug responsiveness.

```powershell
& $samCspPython tools/sam_csp_peer.py sequence --device $env:SAM_CSP_SERIAL --duration-seconds 600 --interval-ms 0 --timeout-seconds 1.0
```

Expected: zero timeout, malformed response, CRC error, stream drop, and missed OPU 10/100/1000 ms callback count. If this fails because interrupt load is the cause, stop acceptance and open a separate XDMAC RX design; do not silently change this implementation.

- [ ] Run the 1-hour 10 Hz soak.

```powershell
& $samCspPython tools/sam_csp_peer.py sequence --device $env:SAM_CSP_SERIAL --duration-seconds 3600 --interval-ms 100 --timeout-seconds 1.0
```

Expected: zero lost/corrupt reply, no unexpected recovery, no stream drop, controls/debug remain responsive.

- [ ] Inject 100 UART error/recovery trials using the peer's fault-baud action and capture pre/post health each time.

```powershell
& $samCspPython tools/sam_csp_peer.py recovery --device $env:SAM_CSP_SERIAL --iterations 100 --fault-baud 115200 --fault-bytes 64 --settle-seconds 0.20 --timeout-seconds 1.0
```

Expected per conclusive trial: UART error and recovery attempt/success counters increase, final state is RUNNING, a new valid request succeeds within 250 ms, and outputs do not change. Trials that induce no observable UART error are recorded as inconclusive rather than pass.

- [ ] Read the production map and USART0 `csp` command after worst-case load. Record csp-rs485 static RAM, libcsp allocations, minimum-ever free heap, and all three task high-watermarks.

Acceptance limits:

```text
csp-rs485 static RAM: approximately 13–14 KiB, explained by symbols
minimum-ever free heap: at least 8192 bytes
router/link/service remaining stack: at least 25% of 512 words each
```

- [ ] Re-run the complete release gate from a clean build directory.

```powershell
python tools/verify_csp_vendor.py
python tools/verify_harmony_csp_contract.py
python -m unittest discover -s tests/python -p 'test_*.py' -v
& 'C:\Program Files\Microchip\MPLABX\v6.30\gnuBins\GnuWin32\bin\make.exe' -C sam_ctl.X -f Makefile CONF=default clean
& 'C:\Program Files\Microchip\MPLABX\v6.30\gnuBins\GnuWin32\bin\make.exe' -C sam_ctl.X -f Makefile CONF=default build
& 'C:\Program Files\Microchip\MPLABX\v6.30\gnuBins\GnuWin32\bin\make.exe' -C sam_ctl.X -f Makefile CONF=default TYPE_IMAGE=DEBUG_RUN build
git status --short
```

On Linux CI:

```sh
make -C tests/host clean test CC=gcc
make -C tests/host clean test CC=gcc SANITIZE=1
```

Expected: verifiers and all tests pass, both target images build, sanitizer is clean, and `git status --short` contains only the evidence update intended for the final commit.

- [ ] Commit acceptance evidence.

```powershell
git add docs/evidence/samv71-csp-rs485-verification.md
git commit -m "docs: record SAMV71 CSP RS485 acceptance"
```

## Final Review Checklist

- [ ] Compare every wire offset, length, status, and detail rule against the approved spec.
- [ ] Confirm exactly one USART1 owner and exactly one strong definition of each generated hook ABI.
- [ ] Confirm CSP addresses are 1/2, route is peer 2, custom ports are 10/11/12, and only `CSP_PING` is delegated.
- [ ] Confirm one CRC32C is present per KISS frame and CSP header flags remain 0.
- [ ] Confirm no remote reboot, RDP, HMAC, XTEA, multidrop, automatic safe-state, or periodic telemetry was introduced.
- [ ] Confirm csp-rs485 imported hashes still match `UPSTREAM.md` and libcsp still points to the exact v1.6 commit.
- [ ] Confirm production/debug target builds, normal/sanitized host tests, Python vectors, electrical checks, stress, recovery, heap, and stack gates all have recorded evidence.
- [ ] Confirm a repository search has no unfinished CSP implementation marker or legacy `uartcomm`, `RsTask`, or `$iGRVT50` transport reference.
