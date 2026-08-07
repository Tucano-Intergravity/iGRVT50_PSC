#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-default.mk)" "nbproject/Makefile-local-default.mk"
include nbproject/Makefile-local-default.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=default
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/sam_ctl.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/sam_ctl.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

ifeq ($(COMPARE_BUILD), true)
COMPARISON_BUILD=-mafrlcsj
else
COMPARISON_BUILD=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=../src/config/default/peripheral/afec/plib_afec1.c ../src/config/default/peripheral/afec/plib_afec0.c ../src/config/default/peripheral/clk/plib_clk.c ../src/config/default/peripheral/efc/plib_efc.c ../src/config/default/peripheral/nvic/plib_nvic.c ../src/config/default/peripheral/pio/plib_pio.c ../src/config/default/peripheral/pwm/plib_pwm0.c ../src/config/default/peripheral/pwm/plib_pwm1.c ../src/config/default/peripheral/spi/spi_master/plib_spi0_master.c ../src/config/default/peripheral/spi/spi_master/plib_spi1_master.c ../src/config/default/peripheral/systick/plib_systick.c ../src/config/default/peripheral/tc/plib_tc1.c ../src/config/default/peripheral/tc/plib_tc0.c ../src/config/default/peripheral/tc/plib_tc3.c ../src/config/default/peripheral/usart/plib_usart0.c ../src/config/default/peripheral/usart/plib_usart1.c ../src/config/default/stdio/xc32_monitor.c ../src/config/default/initialization.c ../src/config/default/interrupts.c ../src/config/default/exceptions.c ../src/config/default/startup_xc32.c ../src/config/default/libc_syscalls.c ../src/config/default/freertos_hooks.c ../third_party/csp-rs485/src/csp_rs485_freertos.c ../third_party/csp-rs485/src/csp_rs485_kiss.c ../third_party/csp-rs485/src/csp_rs485_link.c ../third_party/csp-rs485/src/csp_rs485_supervisor.c ../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7/port.c ../src/third_party/rtos/FreeRTOS/Source/portable/MemMang/heap_1.c ../src/third_party/rtos/FreeRTOS/Source/croutine.c ../src/third_party/rtos/FreeRTOS/Source/list.c ../src/third_party/rtos/FreeRTOS/Source/queue.c ../src/third_party/rtos/FreeRTOS/Source/FreeRTOS_tasks.c ../src/third_party/rtos/FreeRTOS/Source/timers.c ../src/third_party/rtos/FreeRTOS/Source/event_groups.c ../src/third_party/rtos/FreeRTOS/Source/stream_buffer.c iGRVT50/source/csp/sam_csp_codec.c iGRVT50/source/csp/sam_csp_domain.c iGRVT50/source/csp/sam_csp_runtime.c iGRVT50/source/csp/sam_csp_service.c iGRVT50/source/csp/samv71_rs485_port.c iGRVT50/source/hpsolvalve.c iGRVT50/source/lpsolvalve.c iGRVT50/source/sensor.c iGRVT50/source/statemachine.c ../third_party/libcsp/src/arch/freertos/csp_clock.c ../third_party/libcsp/src/arch/freertos/csp_malloc.c ../third_party/libcsp/src/arch/freertos/csp_queue.c ../third_party/libcsp/src/arch/freertos/csp_semaphore.c ../third_party/libcsp/src/arch/freertos/csp_system.c ../third_party/libcsp/src/arch/freertos/csp_thread.c ../third_party/libcsp/src/arch/freertos/csp_time.c ../third_party/libcsp/src/arch/csp_system.c ../third_party/libcsp/src/arch/csp_time.c ../third_party/libcsp/src/crypto/csp_hmac.c ../third_party/libcsp/src/crypto/csp_sha1.c ../third_party/libcsp/src/crypto/csp_xtea.c ../third_party/libcsp/src/interfaces/csp_if_can_pbuf.c ../third_party/libcsp/src/interfaces/csp_if_can.c ../third_party/libcsp/src/interfaces/csp_if_i2c.c ../third_party/libcsp/src/interfaces/csp_if_kiss.c ../third_party/libcsp/src/interfaces/csp_if_lo.c ../third_party/libcsp/src/interfaces/csp_if_zmqhub.c ../third_party/libcsp/src/rtable/csp_rtable.c ../third_party/libcsp/src/rtable/csp_rtable_static.c ../third_party/libcsp/src/transport/csp_rdp.c ../third_party/libcsp/src/transport/csp_udp.c ../third_party/libcsp/src/csp_bridge.c ../third_party/libcsp/src/csp_buffer.c ../third_party/libcsp/src/csp_conn.c ../third_party/libcsp/src/csp_crc32.c ../third_party/libcsp/src/csp_debug.c ../third_party/libcsp/src/csp_dedup.c ../third_party/libcsp/src/csp_endian.c ../third_party/libcsp/src/csp_hex_dump.c ../third_party/libcsp/src/csp_iflist.c ../third_party/libcsp/src/csp_init.c ../third_party/libcsp/src/csp_io.c ../third_party/libcsp/src/csp_port.c ../third_party/libcsp/src/csp_promisc.c ../third_party/libcsp/src/csp_qfifo.c ../third_party/libcsp/src/csp_route.c ../third_party/libcsp/src/csp_service_handler.c ../third_party/libcsp/src/csp_services.c ../third_party/libcsp/src/csp_sfp.c ../src/main.c ../src/adc_func.c ../src/dbg_task.c ../src/opu_task.c ../src/tc_func.c ../src/pwm_func.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/1865065685/plib_afec1.o ${OBJECTDIR}/_ext/1865065685/plib_afec0.o ${OBJECTDIR}/_ext/60165520/plib_clk.o ${OBJECTDIR}/_ext/60167248/plib_efc.o ${OBJECTDIR}/_ext/1865468468/plib_nvic.o ${OBJECTDIR}/_ext/60177924/plib_pio.o ${OBJECTDIR}/_ext/60178356/plib_pwm0.o ${OBJECTDIR}/_ext/60178356/plib_pwm1.o ${OBJECTDIR}/_ext/298189674/plib_spi0_master.o ${OBJECTDIR}/_ext/298189674/plib_spi1_master.o ${OBJECTDIR}/_ext/1827571544/plib_systick.o ${OBJECTDIR}/_ext/829342655/plib_tc1.o ${OBJECTDIR}/_ext/829342655/plib_tc0.o ${OBJECTDIR}/_ext/829342655/plib_tc3.o ${OBJECTDIR}/_ext/2001315827/plib_usart0.o ${OBJECTDIR}/_ext/2001315827/plib_usart1.o ${OBJECTDIR}/_ext/163028504/xc32_monitor.o ${OBJECTDIR}/_ext/1171490990/initialization.o ${OBJECTDIR}/_ext/1171490990/interrupts.o ${OBJECTDIR}/_ext/1171490990/exceptions.o ${OBJECTDIR}/_ext/1171490990/startup_xc32.o ${OBJECTDIR}/_ext/1171490990/libc_syscalls.o ${OBJECTDIR}/_ext/1171490990/freertos_hooks.o ${OBJECTDIR}/_ext/2036275640/csp_rs485_freertos.o ${OBJECTDIR}/_ext/2036275640/csp_rs485_kiss.o ${OBJECTDIR}/_ext/2036275640/csp_rs485_link.o ${OBJECTDIR}/_ext/2036275640/csp_rs485_supervisor.o ${OBJECTDIR}/_ext/977623654/port.o ${OBJECTDIR}/_ext/1665200909/heap_1.o ${OBJECTDIR}/_ext/404212886/croutine.o ${OBJECTDIR}/_ext/404212886/list.o ${OBJECTDIR}/_ext/404212886/queue.o ${OBJECTDIR}/_ext/404212886/FreeRTOS_tasks.o ${OBJECTDIR}/_ext/404212886/timers.o ${OBJECTDIR}/_ext/404212886/event_groups.o ${OBJECTDIR}/_ext/404212886/stream_buffer.o ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_codec.o ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_domain.o ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_runtime.o ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_service.o ${OBJECTDIR}/iGRVT50/source/csp/samv71_rs485_port.o ${OBJECTDIR}/iGRVT50/source/hpsolvalve.o ${OBJECTDIR}/iGRVT50/source/lpsolvalve.o ${OBJECTDIR}/iGRVT50/source/sensor.o ${OBJECTDIR}/iGRVT50/source/statemachine.o ${OBJECTDIR}/_ext/1455547172/csp_clock.o ${OBJECTDIR}/_ext/1455547172/csp_malloc.o ${OBJECTDIR}/_ext/1455547172/csp_queue.o ${OBJECTDIR}/_ext/1455547172/csp_semaphore.o ${OBJECTDIR}/_ext/1455547172/csp_system.o ${OBJECTDIR}/_ext/1455547172/csp_thread.o ${OBJECTDIR}/_ext/1455547172/csp_time.o ${OBJECTDIR}/_ext/3813883/csp_system.o ${OBJECTDIR}/_ext/3813883/csp_time.o ${OBJECTDIR}/_ext/687750832/csp_hmac.o ${OBJECTDIR}/_ext/687750832/csp_sha1.o ${OBJECTDIR}/_ext/687750832/csp_xtea.o ${OBJECTDIR}/_ext/83949769/csp_if_can_pbuf.o ${OBJECTDIR}/_ext/83949769/csp_if_can.o ${OBJECTDIR}/_ext/83949769/csp_if_i2c.o ${OBJECTDIR}/_ext/83949769/csp_if_kiss.o ${OBJECTDIR}/_ext/83949769/csp_if_lo.o ${OBJECTDIR}/_ext/83949769/csp_if_zmqhub.o ${OBJECTDIR}/_ext/1118306443/csp_rtable.o ${OBJECTDIR}/_ext/1118306443/csp_rtable_static.o ${OBJECTDIR}/_ext/1245785818/csp_rdp.o ${OBJECTDIR}/_ext/1245785818/csp_udp.o ${OBJECTDIR}/_ext/1239823104/csp_bridge.o ${OBJECTDIR}/_ext/1239823104/csp_buffer.o ${OBJECTDIR}/_ext/1239823104/csp_conn.o ${OBJECTDIR}/_ext/1239823104/csp_crc32.o ${OBJECTDIR}/_ext/1239823104/csp_debug.o ${OBJECTDIR}/_ext/1239823104/csp_dedup.o ${OBJECTDIR}/_ext/1239823104/csp_endian.o ${OBJECTDIR}/_ext/1239823104/csp_hex_dump.o ${OBJECTDIR}/_ext/1239823104/csp_iflist.o ${OBJECTDIR}/_ext/1239823104/csp_init.o ${OBJECTDIR}/_ext/1239823104/csp_io.o ${OBJECTDIR}/_ext/1239823104/csp_port.o ${OBJECTDIR}/_ext/1239823104/csp_promisc.o ${OBJECTDIR}/_ext/1239823104/csp_qfifo.o ${OBJECTDIR}/_ext/1239823104/csp_route.o ${OBJECTDIR}/_ext/1239823104/csp_service_handler.o ${OBJECTDIR}/_ext/1239823104/csp_services.o ${OBJECTDIR}/_ext/1239823104/csp_sfp.o ${OBJECTDIR}/_ext/1360937237/main.o ${OBJECTDIR}/_ext/1360937237/adc_func.o ${OBJECTDIR}/_ext/1360937237/dbg_task.o ${OBJECTDIR}/_ext/1360937237/opu_task.o ${OBJECTDIR}/_ext/1360937237/tc_func.o ${OBJECTDIR}/_ext/1360937237/pwm_func.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/1865065685/plib_afec1.o.d ${OBJECTDIR}/_ext/1865065685/plib_afec0.o.d ${OBJECTDIR}/_ext/60165520/plib_clk.o.d ${OBJECTDIR}/_ext/60167248/plib_efc.o.d ${OBJECTDIR}/_ext/1865468468/plib_nvic.o.d ${OBJECTDIR}/_ext/60177924/plib_pio.o.d ${OBJECTDIR}/_ext/60178356/plib_pwm0.o.d ${OBJECTDIR}/_ext/60178356/plib_pwm1.o.d ${OBJECTDIR}/_ext/298189674/plib_spi0_master.o.d ${OBJECTDIR}/_ext/298189674/plib_spi1_master.o.d ${OBJECTDIR}/_ext/1827571544/plib_systick.o.d ${OBJECTDIR}/_ext/829342655/plib_tc1.o.d ${OBJECTDIR}/_ext/829342655/plib_tc0.o.d ${OBJECTDIR}/_ext/829342655/plib_tc3.o.d ${OBJECTDIR}/_ext/2001315827/plib_usart0.o.d ${OBJECTDIR}/_ext/2001315827/plib_usart1.o.d ${OBJECTDIR}/_ext/163028504/xc32_monitor.o.d ${OBJECTDIR}/_ext/1171490990/initialization.o.d ${OBJECTDIR}/_ext/1171490990/interrupts.o.d ${OBJECTDIR}/_ext/1171490990/exceptions.o.d ${OBJECTDIR}/_ext/1171490990/startup_xc32.o.d ${OBJECTDIR}/_ext/1171490990/libc_syscalls.o.d ${OBJECTDIR}/_ext/1171490990/freertos_hooks.o.d ${OBJECTDIR}/_ext/2036275640/csp_rs485_freertos.o.d ${OBJECTDIR}/_ext/2036275640/csp_rs485_kiss.o.d ${OBJECTDIR}/_ext/2036275640/csp_rs485_link.o.d ${OBJECTDIR}/_ext/2036275640/csp_rs485_supervisor.o.d ${OBJECTDIR}/_ext/977623654/port.o.d ${OBJECTDIR}/_ext/1665200909/heap_1.o.d ${OBJECTDIR}/_ext/404212886/croutine.o.d ${OBJECTDIR}/_ext/404212886/list.o.d ${OBJECTDIR}/_ext/404212886/queue.o.d ${OBJECTDIR}/_ext/404212886/FreeRTOS_tasks.o.d ${OBJECTDIR}/_ext/404212886/timers.o.d ${OBJECTDIR}/_ext/404212886/event_groups.o.d ${OBJECTDIR}/_ext/404212886/stream_buffer.o.d ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_codec.o.d ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_domain.o.d ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_runtime.o.d ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_service.o.d ${OBJECTDIR}/iGRVT50/source/csp/samv71_rs485_port.o.d ${OBJECTDIR}/iGRVT50/source/hpsolvalve.o.d ${OBJECTDIR}/iGRVT50/source/lpsolvalve.o.d ${OBJECTDIR}/iGRVT50/source/sensor.o.d ${OBJECTDIR}/iGRVT50/source/statemachine.o.d ${OBJECTDIR}/_ext/1455547172/csp_clock.o.d ${OBJECTDIR}/_ext/1455547172/csp_malloc.o.d ${OBJECTDIR}/_ext/1455547172/csp_queue.o.d ${OBJECTDIR}/_ext/1455547172/csp_semaphore.o.d ${OBJECTDIR}/_ext/1455547172/csp_system.o.d ${OBJECTDIR}/_ext/1455547172/csp_thread.o.d ${OBJECTDIR}/_ext/1455547172/csp_time.o.d ${OBJECTDIR}/_ext/3813883/csp_system.o.d ${OBJECTDIR}/_ext/3813883/csp_time.o.d ${OBJECTDIR}/_ext/687750832/csp_hmac.o.d ${OBJECTDIR}/_ext/687750832/csp_sha1.o.d ${OBJECTDIR}/_ext/687750832/csp_xtea.o.d ${OBJECTDIR}/_ext/83949769/csp_if_can_pbuf.o.d ${OBJECTDIR}/_ext/83949769/csp_if_can.o.d ${OBJECTDIR}/_ext/83949769/csp_if_i2c.o.d ${OBJECTDIR}/_ext/83949769/csp_if_kiss.o.d ${OBJECTDIR}/_ext/83949769/csp_if_lo.o.d ${OBJECTDIR}/_ext/83949769/csp_if_zmqhub.o.d ${OBJECTDIR}/_ext/1118306443/csp_rtable.o.d ${OBJECTDIR}/_ext/1118306443/csp_rtable_static.o.d ${OBJECTDIR}/_ext/1245785818/csp_rdp.o.d ${OBJECTDIR}/_ext/1245785818/csp_udp.o.d ${OBJECTDIR}/_ext/1239823104/csp_bridge.o.d ${OBJECTDIR}/_ext/1239823104/csp_buffer.o.d ${OBJECTDIR}/_ext/1239823104/csp_conn.o.d ${OBJECTDIR}/_ext/1239823104/csp_crc32.o.d ${OBJECTDIR}/_ext/1239823104/csp_debug.o.d ${OBJECTDIR}/_ext/1239823104/csp_dedup.o.d ${OBJECTDIR}/_ext/1239823104/csp_endian.o.d ${OBJECTDIR}/_ext/1239823104/csp_hex_dump.o.d ${OBJECTDIR}/_ext/1239823104/csp_iflist.o.d ${OBJECTDIR}/_ext/1239823104/csp_init.o.d ${OBJECTDIR}/_ext/1239823104/csp_io.o.d ${OBJECTDIR}/_ext/1239823104/csp_port.o.d ${OBJECTDIR}/_ext/1239823104/csp_promisc.o.d ${OBJECTDIR}/_ext/1239823104/csp_qfifo.o.d ${OBJECTDIR}/_ext/1239823104/csp_route.o.d ${OBJECTDIR}/_ext/1239823104/csp_service_handler.o.d ${OBJECTDIR}/_ext/1239823104/csp_services.o.d ${OBJECTDIR}/_ext/1239823104/csp_sfp.o.d ${OBJECTDIR}/_ext/1360937237/main.o.d ${OBJECTDIR}/_ext/1360937237/adc_func.o.d ${OBJECTDIR}/_ext/1360937237/dbg_task.o.d ${OBJECTDIR}/_ext/1360937237/opu_task.o.d ${OBJECTDIR}/_ext/1360937237/tc_func.o.d ${OBJECTDIR}/_ext/1360937237/pwm_func.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/1865065685/plib_afec1.o ${OBJECTDIR}/_ext/1865065685/plib_afec0.o ${OBJECTDIR}/_ext/60165520/plib_clk.o ${OBJECTDIR}/_ext/60167248/plib_efc.o ${OBJECTDIR}/_ext/1865468468/plib_nvic.o ${OBJECTDIR}/_ext/60177924/plib_pio.o ${OBJECTDIR}/_ext/60178356/plib_pwm0.o ${OBJECTDIR}/_ext/60178356/plib_pwm1.o ${OBJECTDIR}/_ext/298189674/plib_spi0_master.o ${OBJECTDIR}/_ext/298189674/plib_spi1_master.o ${OBJECTDIR}/_ext/1827571544/plib_systick.o ${OBJECTDIR}/_ext/829342655/plib_tc1.o ${OBJECTDIR}/_ext/829342655/plib_tc0.o ${OBJECTDIR}/_ext/829342655/plib_tc3.o ${OBJECTDIR}/_ext/2001315827/plib_usart0.o ${OBJECTDIR}/_ext/2001315827/plib_usart1.o ${OBJECTDIR}/_ext/163028504/xc32_monitor.o ${OBJECTDIR}/_ext/1171490990/initialization.o ${OBJECTDIR}/_ext/1171490990/interrupts.o ${OBJECTDIR}/_ext/1171490990/exceptions.o ${OBJECTDIR}/_ext/1171490990/startup_xc32.o ${OBJECTDIR}/_ext/1171490990/libc_syscalls.o ${OBJECTDIR}/_ext/1171490990/freertos_hooks.o ${OBJECTDIR}/_ext/2036275640/csp_rs485_freertos.o ${OBJECTDIR}/_ext/2036275640/csp_rs485_kiss.o ${OBJECTDIR}/_ext/2036275640/csp_rs485_link.o ${OBJECTDIR}/_ext/2036275640/csp_rs485_supervisor.o ${OBJECTDIR}/_ext/977623654/port.o ${OBJECTDIR}/_ext/1665200909/heap_1.o ${OBJECTDIR}/_ext/404212886/croutine.o ${OBJECTDIR}/_ext/404212886/list.o ${OBJECTDIR}/_ext/404212886/queue.o ${OBJECTDIR}/_ext/404212886/FreeRTOS_tasks.o ${OBJECTDIR}/_ext/404212886/timers.o ${OBJECTDIR}/_ext/404212886/event_groups.o ${OBJECTDIR}/_ext/404212886/stream_buffer.o ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_codec.o ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_domain.o ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_runtime.o ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_service.o ${OBJECTDIR}/iGRVT50/source/csp/samv71_rs485_port.o ${OBJECTDIR}/iGRVT50/source/hpsolvalve.o ${OBJECTDIR}/iGRVT50/source/lpsolvalve.o ${OBJECTDIR}/iGRVT50/source/sensor.o ${OBJECTDIR}/iGRVT50/source/statemachine.o ${OBJECTDIR}/_ext/1455547172/csp_clock.o ${OBJECTDIR}/_ext/1455547172/csp_malloc.o ${OBJECTDIR}/_ext/1455547172/csp_queue.o ${OBJECTDIR}/_ext/1455547172/csp_semaphore.o ${OBJECTDIR}/_ext/1455547172/csp_system.o ${OBJECTDIR}/_ext/1455547172/csp_thread.o ${OBJECTDIR}/_ext/1455547172/csp_time.o ${OBJECTDIR}/_ext/3813883/csp_system.o ${OBJECTDIR}/_ext/3813883/csp_time.o ${OBJECTDIR}/_ext/687750832/csp_hmac.o ${OBJECTDIR}/_ext/687750832/csp_sha1.o ${OBJECTDIR}/_ext/687750832/csp_xtea.o ${OBJECTDIR}/_ext/83949769/csp_if_can_pbuf.o ${OBJECTDIR}/_ext/83949769/csp_if_can.o ${OBJECTDIR}/_ext/83949769/csp_if_i2c.o ${OBJECTDIR}/_ext/83949769/csp_if_kiss.o ${OBJECTDIR}/_ext/83949769/csp_if_lo.o ${OBJECTDIR}/_ext/83949769/csp_if_zmqhub.o ${OBJECTDIR}/_ext/1118306443/csp_rtable.o ${OBJECTDIR}/_ext/1118306443/csp_rtable_static.o ${OBJECTDIR}/_ext/1245785818/csp_rdp.o ${OBJECTDIR}/_ext/1245785818/csp_udp.o ${OBJECTDIR}/_ext/1239823104/csp_bridge.o ${OBJECTDIR}/_ext/1239823104/csp_buffer.o ${OBJECTDIR}/_ext/1239823104/csp_conn.o ${OBJECTDIR}/_ext/1239823104/csp_crc32.o ${OBJECTDIR}/_ext/1239823104/csp_debug.o ${OBJECTDIR}/_ext/1239823104/csp_dedup.o ${OBJECTDIR}/_ext/1239823104/csp_endian.o ${OBJECTDIR}/_ext/1239823104/csp_hex_dump.o ${OBJECTDIR}/_ext/1239823104/csp_iflist.o ${OBJECTDIR}/_ext/1239823104/csp_init.o ${OBJECTDIR}/_ext/1239823104/csp_io.o ${OBJECTDIR}/_ext/1239823104/csp_port.o ${OBJECTDIR}/_ext/1239823104/csp_promisc.o ${OBJECTDIR}/_ext/1239823104/csp_qfifo.o ${OBJECTDIR}/_ext/1239823104/csp_route.o ${OBJECTDIR}/_ext/1239823104/csp_service_handler.o ${OBJECTDIR}/_ext/1239823104/csp_services.o ${OBJECTDIR}/_ext/1239823104/csp_sfp.o ${OBJECTDIR}/_ext/1360937237/main.o ${OBJECTDIR}/_ext/1360937237/adc_func.o ${OBJECTDIR}/_ext/1360937237/dbg_task.o ${OBJECTDIR}/_ext/1360937237/opu_task.o ${OBJECTDIR}/_ext/1360937237/tc_func.o ${OBJECTDIR}/_ext/1360937237/pwm_func.o

# Source Files
SOURCEFILES=../src/config/default/peripheral/afec/plib_afec1.c ../src/config/default/peripheral/afec/plib_afec0.c ../src/config/default/peripheral/clk/plib_clk.c ../src/config/default/peripheral/efc/plib_efc.c ../src/config/default/peripheral/nvic/plib_nvic.c ../src/config/default/peripheral/pio/plib_pio.c ../src/config/default/peripheral/pwm/plib_pwm0.c ../src/config/default/peripheral/pwm/plib_pwm1.c ../src/config/default/peripheral/spi/spi_master/plib_spi0_master.c ../src/config/default/peripheral/spi/spi_master/plib_spi1_master.c ../src/config/default/peripheral/systick/plib_systick.c ../src/config/default/peripheral/tc/plib_tc1.c ../src/config/default/peripheral/tc/plib_tc0.c ../src/config/default/peripheral/tc/plib_tc3.c ../src/config/default/peripheral/usart/plib_usart0.c ../src/config/default/peripheral/usart/plib_usart1.c ../src/config/default/stdio/xc32_monitor.c ../src/config/default/initialization.c ../src/config/default/interrupts.c ../src/config/default/exceptions.c ../src/config/default/startup_xc32.c ../src/config/default/libc_syscalls.c ../src/config/default/freertos_hooks.c ../third_party/csp-rs485/src/csp_rs485_freertos.c ../third_party/csp-rs485/src/csp_rs485_kiss.c ../third_party/csp-rs485/src/csp_rs485_link.c ../third_party/csp-rs485/src/csp_rs485_supervisor.c ../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7/port.c ../src/third_party/rtos/FreeRTOS/Source/portable/MemMang/heap_1.c ../src/third_party/rtos/FreeRTOS/Source/croutine.c ../src/third_party/rtos/FreeRTOS/Source/list.c ../src/third_party/rtos/FreeRTOS/Source/queue.c ../src/third_party/rtos/FreeRTOS/Source/FreeRTOS_tasks.c ../src/third_party/rtos/FreeRTOS/Source/timers.c ../src/third_party/rtos/FreeRTOS/Source/event_groups.c ../src/third_party/rtos/FreeRTOS/Source/stream_buffer.c iGRVT50/source/csp/sam_csp_codec.c iGRVT50/source/csp/sam_csp_domain.c iGRVT50/source/csp/sam_csp_runtime.c iGRVT50/source/csp/sam_csp_service.c iGRVT50/source/csp/samv71_rs485_port.c iGRVT50/source/hpsolvalve.c iGRVT50/source/lpsolvalve.c iGRVT50/source/sensor.c iGRVT50/source/statemachine.c ../third_party/libcsp/src/arch/freertos/csp_clock.c ../third_party/libcsp/src/arch/freertos/csp_malloc.c ../third_party/libcsp/src/arch/freertos/csp_queue.c ../third_party/libcsp/src/arch/freertos/csp_semaphore.c ../third_party/libcsp/src/arch/freertos/csp_system.c ../third_party/libcsp/src/arch/freertos/csp_thread.c ../third_party/libcsp/src/arch/freertos/csp_time.c ../third_party/libcsp/src/arch/csp_system.c ../third_party/libcsp/src/arch/csp_time.c ../third_party/libcsp/src/crypto/csp_hmac.c ../third_party/libcsp/src/crypto/csp_sha1.c ../third_party/libcsp/src/crypto/csp_xtea.c ../third_party/libcsp/src/interfaces/csp_if_can_pbuf.c ../third_party/libcsp/src/interfaces/csp_if_can.c ../third_party/libcsp/src/interfaces/csp_if_i2c.c ../third_party/libcsp/src/interfaces/csp_if_kiss.c ../third_party/libcsp/src/interfaces/csp_if_lo.c ../third_party/libcsp/src/interfaces/csp_if_zmqhub.c ../third_party/libcsp/src/rtable/csp_rtable.c ../third_party/libcsp/src/rtable/csp_rtable_static.c ../third_party/libcsp/src/transport/csp_rdp.c ../third_party/libcsp/src/transport/csp_udp.c ../third_party/libcsp/src/csp_bridge.c ../third_party/libcsp/src/csp_buffer.c ../third_party/libcsp/src/csp_conn.c ../third_party/libcsp/src/csp_crc32.c ../third_party/libcsp/src/csp_debug.c ../third_party/libcsp/src/csp_dedup.c ../third_party/libcsp/src/csp_endian.c ../third_party/libcsp/src/csp_hex_dump.c ../third_party/libcsp/src/csp_iflist.c ../third_party/libcsp/src/csp_init.c ../third_party/libcsp/src/csp_io.c ../third_party/libcsp/src/csp_port.c ../third_party/libcsp/src/csp_promisc.c ../third_party/libcsp/src/csp_qfifo.c ../third_party/libcsp/src/csp_route.c ../third_party/libcsp/src/csp_service_handler.c ../third_party/libcsp/src/csp_services.c ../third_party/libcsp/src/csp_sfp.c ../src/main.c ../src/adc_func.c ../src/dbg_task.c ../src/opu_task.c ../src/tc_func.c ../src/pwm_func.c

# Pack Options 
PACK_COMMON_OPTIONS=-I "${CMSIS_DIR}/CMSIS/Core/Include"



CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-default.mk ${DISTDIR}/sam_ctl.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=ATSAMV71Q21B
MP_LINKER_FILE_OPTION=,--script="..\src\config\default\ATSAMV71Q21B.ld"
# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assembleWithPreprocess
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/1865065685/plib_afec1.o: ../src/config/default/peripheral/afec/plib_afec1.c  .generated_files/flags/default/5b7a1a570226f4fbf0533468cf2b8e4b5bb0a768 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1865065685" 
	@${RM} ${OBJECTDIR}/_ext/1865065685/plib_afec1.o.d 
	@${RM} ${OBJECTDIR}/_ext/1865065685/plib_afec1.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1865065685/plib_afec1.o.d" -o ${OBJECTDIR}/_ext/1865065685/plib_afec1.o ../src/config/default/peripheral/afec/plib_afec1.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1865065685/plib_afec0.o: ../src/config/default/peripheral/afec/plib_afec0.c  .generated_files/flags/default/ff5aa8cf081d54f1cb199e871cf0bd60ddfe167 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1865065685" 
	@${RM} ${OBJECTDIR}/_ext/1865065685/plib_afec0.o.d 
	@${RM} ${OBJECTDIR}/_ext/1865065685/plib_afec0.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1865065685/plib_afec0.o.d" -o ${OBJECTDIR}/_ext/1865065685/plib_afec0.o ../src/config/default/peripheral/afec/plib_afec0.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/60165520/plib_clk.o: ../src/config/default/peripheral/clk/plib_clk.c  .generated_files/flags/default/9297a65f7bafa7e82c6b458779812bc508a50d23 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/60165520" 
	@${RM} ${OBJECTDIR}/_ext/60165520/plib_clk.o.d 
	@${RM} ${OBJECTDIR}/_ext/60165520/plib_clk.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/60165520/plib_clk.o.d" -o ${OBJECTDIR}/_ext/60165520/plib_clk.o ../src/config/default/peripheral/clk/plib_clk.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/60167248/plib_efc.o: ../src/config/default/peripheral/efc/plib_efc.c  .generated_files/flags/default/8f86298ad4402f00aa30cc7f5cd37aa365883ce5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/60167248" 
	@${RM} ${OBJECTDIR}/_ext/60167248/plib_efc.o.d 
	@${RM} ${OBJECTDIR}/_ext/60167248/plib_efc.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/60167248/plib_efc.o.d" -o ${OBJECTDIR}/_ext/60167248/plib_efc.o ../src/config/default/peripheral/efc/plib_efc.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1865468468/plib_nvic.o: ../src/config/default/peripheral/nvic/plib_nvic.c  .generated_files/flags/default/6db67bd160853d5110b64786b62696211323b17e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1865468468" 
	@${RM} ${OBJECTDIR}/_ext/1865468468/plib_nvic.o.d 
	@${RM} ${OBJECTDIR}/_ext/1865468468/plib_nvic.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1865468468/plib_nvic.o.d" -o ${OBJECTDIR}/_ext/1865468468/plib_nvic.o ../src/config/default/peripheral/nvic/plib_nvic.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/60177924/plib_pio.o: ../src/config/default/peripheral/pio/plib_pio.c  .generated_files/flags/default/50191d58751f51ed31ee21702e3db17560c1865f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/60177924" 
	@${RM} ${OBJECTDIR}/_ext/60177924/plib_pio.o.d 
	@${RM} ${OBJECTDIR}/_ext/60177924/plib_pio.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/60177924/plib_pio.o.d" -o ${OBJECTDIR}/_ext/60177924/plib_pio.o ../src/config/default/peripheral/pio/plib_pio.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/60178356/plib_pwm0.o: ../src/config/default/peripheral/pwm/plib_pwm0.c  .generated_files/flags/default/b4d34d110d6219f01903e67689cf39b35c42c799 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/60178356" 
	@${RM} ${OBJECTDIR}/_ext/60178356/plib_pwm0.o.d 
	@${RM} ${OBJECTDIR}/_ext/60178356/plib_pwm0.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/60178356/plib_pwm0.o.d" -o ${OBJECTDIR}/_ext/60178356/plib_pwm0.o ../src/config/default/peripheral/pwm/plib_pwm0.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/60178356/plib_pwm1.o: ../src/config/default/peripheral/pwm/plib_pwm1.c  .generated_files/flags/default/38f9418add81f1618e91401e8ecd52f20be06dae .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/60178356" 
	@${RM} ${OBJECTDIR}/_ext/60178356/plib_pwm1.o.d 
	@${RM} ${OBJECTDIR}/_ext/60178356/plib_pwm1.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/60178356/plib_pwm1.o.d" -o ${OBJECTDIR}/_ext/60178356/plib_pwm1.o ../src/config/default/peripheral/pwm/plib_pwm1.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/298189674/plib_spi0_master.o: ../src/config/default/peripheral/spi/spi_master/plib_spi0_master.c  .generated_files/flags/default/57a8aa001c424b47cff2fd51fc01f7cee86e7787 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/298189674" 
	@${RM} ${OBJECTDIR}/_ext/298189674/plib_spi0_master.o.d 
	@${RM} ${OBJECTDIR}/_ext/298189674/plib_spi0_master.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/298189674/plib_spi0_master.o.d" -o ${OBJECTDIR}/_ext/298189674/plib_spi0_master.o ../src/config/default/peripheral/spi/spi_master/plib_spi0_master.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/298189674/plib_spi1_master.o: ../src/config/default/peripheral/spi/spi_master/plib_spi1_master.c  .generated_files/flags/default/3acad321c6ac165dd2a2514b1bc7780b7d361835 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/298189674" 
	@${RM} ${OBJECTDIR}/_ext/298189674/plib_spi1_master.o.d 
	@${RM} ${OBJECTDIR}/_ext/298189674/plib_spi1_master.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/298189674/plib_spi1_master.o.d" -o ${OBJECTDIR}/_ext/298189674/plib_spi1_master.o ../src/config/default/peripheral/spi/spi_master/plib_spi1_master.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1827571544/plib_systick.o: ../src/config/default/peripheral/systick/plib_systick.c  .generated_files/flags/default/ea8c5eae113ae0c54a447f0109d8384878e5c78c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1827571544" 
	@${RM} ${OBJECTDIR}/_ext/1827571544/plib_systick.o.d 
	@${RM} ${OBJECTDIR}/_ext/1827571544/plib_systick.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1827571544/plib_systick.o.d" -o ${OBJECTDIR}/_ext/1827571544/plib_systick.o ../src/config/default/peripheral/systick/plib_systick.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/829342655/plib_tc1.o: ../src/config/default/peripheral/tc/plib_tc1.c  .generated_files/flags/default/f327271a87ea247c5a810f576bf0a00aca699161 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/829342655" 
	@${RM} ${OBJECTDIR}/_ext/829342655/plib_tc1.o.d 
	@${RM} ${OBJECTDIR}/_ext/829342655/plib_tc1.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/829342655/plib_tc1.o.d" -o ${OBJECTDIR}/_ext/829342655/plib_tc1.o ../src/config/default/peripheral/tc/plib_tc1.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/829342655/plib_tc0.o: ../src/config/default/peripheral/tc/plib_tc0.c  .generated_files/flags/default/36579507c066d407abedae70756b2720af72e612 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/829342655" 
	@${RM} ${OBJECTDIR}/_ext/829342655/plib_tc0.o.d 
	@${RM} ${OBJECTDIR}/_ext/829342655/plib_tc0.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/829342655/plib_tc0.o.d" -o ${OBJECTDIR}/_ext/829342655/plib_tc0.o ../src/config/default/peripheral/tc/plib_tc0.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/829342655/plib_tc3.o: ../src/config/default/peripheral/tc/plib_tc3.c  .generated_files/flags/default/c9c7f40d3430ce1b3243ec4ccb5e9978cb05b085 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/829342655" 
	@${RM} ${OBJECTDIR}/_ext/829342655/plib_tc3.o.d 
	@${RM} ${OBJECTDIR}/_ext/829342655/plib_tc3.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/829342655/plib_tc3.o.d" -o ${OBJECTDIR}/_ext/829342655/plib_tc3.o ../src/config/default/peripheral/tc/plib_tc3.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/2001315827/plib_usart0.o: ../src/config/default/peripheral/usart/plib_usart0.c  .generated_files/flags/default/fc03374839a91f40dfd5f89e2f707ee8bfe57ed7 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/2001315827" 
	@${RM} ${OBJECTDIR}/_ext/2001315827/plib_usart0.o.d 
	@${RM} ${OBJECTDIR}/_ext/2001315827/plib_usart0.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/2001315827/plib_usart0.o.d" -o ${OBJECTDIR}/_ext/2001315827/plib_usart0.o ../src/config/default/peripheral/usart/plib_usart0.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/2001315827/plib_usart1.o: ../src/config/default/peripheral/usart/plib_usart1.c  .generated_files/flags/default/60964b40128292b806804013d60df3d8109c63a0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/2001315827" 
	@${RM} ${OBJECTDIR}/_ext/2001315827/plib_usart1.o.d 
	@${RM} ${OBJECTDIR}/_ext/2001315827/plib_usart1.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/2001315827/plib_usart1.o.d" -o ${OBJECTDIR}/_ext/2001315827/plib_usart1.o ../src/config/default/peripheral/usart/plib_usart1.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/163028504/xc32_monitor.o: ../src/config/default/stdio/xc32_monitor.c  .generated_files/flags/default/e54273a76378b6d4a559603bdf6d5dff8e52d3e4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/163028504" 
	@${RM} ${OBJECTDIR}/_ext/163028504/xc32_monitor.o.d 
	@${RM} ${OBJECTDIR}/_ext/163028504/xc32_monitor.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/163028504/xc32_monitor.o.d" -o ${OBJECTDIR}/_ext/163028504/xc32_monitor.o ../src/config/default/stdio/xc32_monitor.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1171490990/initialization.o: ../src/config/default/initialization.c  .generated_files/flags/default/ccb184217c0d0778bafecfa31841d4bca56be5f4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1171490990" 
	@${RM} ${OBJECTDIR}/_ext/1171490990/initialization.o.d 
	@${RM} ${OBJECTDIR}/_ext/1171490990/initialization.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1171490990/initialization.o.d" -o ${OBJECTDIR}/_ext/1171490990/initialization.o ../src/config/default/initialization.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1171490990/interrupts.o: ../src/config/default/interrupts.c  .generated_files/flags/default/9359b63b4bdf2265d74186d4219bc8bda4692469 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1171490990" 
	@${RM} ${OBJECTDIR}/_ext/1171490990/interrupts.o.d 
	@${RM} ${OBJECTDIR}/_ext/1171490990/interrupts.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1171490990/interrupts.o.d" -o ${OBJECTDIR}/_ext/1171490990/interrupts.o ../src/config/default/interrupts.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1171490990/exceptions.o: ../src/config/default/exceptions.c  .generated_files/flags/default/b8cde00a51d981cc20cbdb8bd85abda1d4ee4d72 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1171490990" 
	@${RM} ${OBJECTDIR}/_ext/1171490990/exceptions.o.d 
	@${RM} ${OBJECTDIR}/_ext/1171490990/exceptions.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1171490990/exceptions.o.d" -o ${OBJECTDIR}/_ext/1171490990/exceptions.o ../src/config/default/exceptions.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1171490990/startup_xc32.o: ../src/config/default/startup_xc32.c  .generated_files/flags/default/417e9efc9492abdf5d7e2cfdda63a2678016a4ad .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1171490990" 
	@${RM} ${OBJECTDIR}/_ext/1171490990/startup_xc32.o.d 
	@${RM} ${OBJECTDIR}/_ext/1171490990/startup_xc32.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1171490990/startup_xc32.o.d" -o ${OBJECTDIR}/_ext/1171490990/startup_xc32.o ../src/config/default/startup_xc32.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1171490990/libc_syscalls.o: ../src/config/default/libc_syscalls.c  .generated_files/flags/default/55cf8066a36ed98a09433010502d63514dec7363 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1171490990" 
	@${RM} ${OBJECTDIR}/_ext/1171490990/libc_syscalls.o.d 
	@${RM} ${OBJECTDIR}/_ext/1171490990/libc_syscalls.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1171490990/libc_syscalls.o.d" -o ${OBJECTDIR}/_ext/1171490990/libc_syscalls.o ../src/config/default/libc_syscalls.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1171490990/freertos_hooks.o: ../src/config/default/freertos_hooks.c  .generated_files/flags/default/655a57d3062939c68e8894d124051c0cfba2ed18 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1171490990" 
	@${RM} ${OBJECTDIR}/_ext/1171490990/freertos_hooks.o.d 
	@${RM} ${OBJECTDIR}/_ext/1171490990/freertos_hooks.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1171490990/freertos_hooks.o.d" -o ${OBJECTDIR}/_ext/1171490990/freertos_hooks.o ../src/config/default/freertos_hooks.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/2036275640/csp_rs485_freertos.o: ../third_party/csp-rs485/src/csp_rs485_freertos.c  .generated_files/flags/default/c7ff9afe8e8e048f654a4018feb1d77fd6016207 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/2036275640" 
	@${RM} ${OBJECTDIR}/_ext/2036275640/csp_rs485_freertos.o.d 
	@${RM} ${OBJECTDIR}/_ext/2036275640/csp_rs485_freertos.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/2036275640/csp_rs485_freertos.o.d" -o ${OBJECTDIR}/_ext/2036275640/csp_rs485_freertos.o ../third_party/csp-rs485/src/csp_rs485_freertos.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/2036275640/csp_rs485_kiss.o: ../third_party/csp-rs485/src/csp_rs485_kiss.c  .generated_files/flags/default/ef34d3ff3346b05003877cf9159ccde1c20ab670 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/2036275640" 
	@${RM} ${OBJECTDIR}/_ext/2036275640/csp_rs485_kiss.o.d 
	@${RM} ${OBJECTDIR}/_ext/2036275640/csp_rs485_kiss.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/2036275640/csp_rs485_kiss.o.d" -o ${OBJECTDIR}/_ext/2036275640/csp_rs485_kiss.o ../third_party/csp-rs485/src/csp_rs485_kiss.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/2036275640/csp_rs485_link.o: ../third_party/csp-rs485/src/csp_rs485_link.c  .generated_files/flags/default/9317981f15e6dd3326cf03f957bebfd43f336c84 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/2036275640" 
	@${RM} ${OBJECTDIR}/_ext/2036275640/csp_rs485_link.o.d 
	@${RM} ${OBJECTDIR}/_ext/2036275640/csp_rs485_link.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/2036275640/csp_rs485_link.o.d" -o ${OBJECTDIR}/_ext/2036275640/csp_rs485_link.o ../third_party/csp-rs485/src/csp_rs485_link.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/2036275640/csp_rs485_supervisor.o: ../third_party/csp-rs485/src/csp_rs485_supervisor.c  .generated_files/flags/default/bb240bd47e982821401d2f4532e6476b3743567d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/2036275640" 
	@${RM} ${OBJECTDIR}/_ext/2036275640/csp_rs485_supervisor.o.d 
	@${RM} ${OBJECTDIR}/_ext/2036275640/csp_rs485_supervisor.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/2036275640/csp_rs485_supervisor.o.d" -o ${OBJECTDIR}/_ext/2036275640/csp_rs485_supervisor.o ../third_party/csp-rs485/src/csp_rs485_supervisor.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/977623654/port.o: ../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7/port.c  .generated_files/flags/default/132b54968380ef951ef6f0ee2ba3b0825604fcfa .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/977623654" 
	@${RM} ${OBJECTDIR}/_ext/977623654/port.o.d 
	@${RM} ${OBJECTDIR}/_ext/977623654/port.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/977623654/port.o.d" -o ${OBJECTDIR}/_ext/977623654/port.o ../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7/port.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1665200909/heap_1.o: ../src/third_party/rtos/FreeRTOS/Source/portable/MemMang/heap_1.c  .generated_files/flags/default/e0d86d77a7edbf446ca7eec39fb3d931101c6197 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1665200909" 
	@${RM} ${OBJECTDIR}/_ext/1665200909/heap_1.o.d 
	@${RM} ${OBJECTDIR}/_ext/1665200909/heap_1.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1665200909/heap_1.o.d" -o ${OBJECTDIR}/_ext/1665200909/heap_1.o ../src/third_party/rtos/FreeRTOS/Source/portable/MemMang/heap_1.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/404212886/croutine.o: ../src/third_party/rtos/FreeRTOS/Source/croutine.c  .generated_files/flags/default/d509024ccd06c11cba5648d287d88a9716a73df4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/404212886" 
	@${RM} ${OBJECTDIR}/_ext/404212886/croutine.o.d 
	@${RM} ${OBJECTDIR}/_ext/404212886/croutine.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/404212886/croutine.o.d" -o ${OBJECTDIR}/_ext/404212886/croutine.o ../src/third_party/rtos/FreeRTOS/Source/croutine.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/404212886/list.o: ../src/third_party/rtos/FreeRTOS/Source/list.c  .generated_files/flags/default/9ddfdfc915a01272ae73b4e94121b801c0628a0e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/404212886" 
	@${RM} ${OBJECTDIR}/_ext/404212886/list.o.d 
	@${RM} ${OBJECTDIR}/_ext/404212886/list.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/404212886/list.o.d" -o ${OBJECTDIR}/_ext/404212886/list.o ../src/third_party/rtos/FreeRTOS/Source/list.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/404212886/queue.o: ../src/third_party/rtos/FreeRTOS/Source/queue.c  .generated_files/flags/default/91fdd08f62494bf734b54c510add85de8f956ba3 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/404212886" 
	@${RM} ${OBJECTDIR}/_ext/404212886/queue.o.d 
	@${RM} ${OBJECTDIR}/_ext/404212886/queue.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/404212886/queue.o.d" -o ${OBJECTDIR}/_ext/404212886/queue.o ../src/third_party/rtos/FreeRTOS/Source/queue.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/404212886/FreeRTOS_tasks.o: ../src/third_party/rtos/FreeRTOS/Source/FreeRTOS_tasks.c  .generated_files/flags/default/5b51c3b66adc4cdb5ab16b5294c002f2864727a5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/404212886" 
	@${RM} ${OBJECTDIR}/_ext/404212886/FreeRTOS_tasks.o.d 
	@${RM} ${OBJECTDIR}/_ext/404212886/FreeRTOS_tasks.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/404212886/FreeRTOS_tasks.o.d" -o ${OBJECTDIR}/_ext/404212886/FreeRTOS_tasks.o ../src/third_party/rtos/FreeRTOS/Source/FreeRTOS_tasks.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/404212886/timers.o: ../src/third_party/rtos/FreeRTOS/Source/timers.c  .generated_files/flags/default/c5b16115eddcb5566e0c1c86cf36824fc708d76d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/404212886" 
	@${RM} ${OBJECTDIR}/_ext/404212886/timers.o.d 
	@${RM} ${OBJECTDIR}/_ext/404212886/timers.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/404212886/timers.o.d" -o ${OBJECTDIR}/_ext/404212886/timers.o ../src/third_party/rtos/FreeRTOS/Source/timers.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/404212886/event_groups.o: ../src/third_party/rtos/FreeRTOS/Source/event_groups.c  .generated_files/flags/default/f35f6dd302d8ce23784bc81123e1d8ff712f47ce .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/404212886" 
	@${RM} ${OBJECTDIR}/_ext/404212886/event_groups.o.d 
	@${RM} ${OBJECTDIR}/_ext/404212886/event_groups.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/404212886/event_groups.o.d" -o ${OBJECTDIR}/_ext/404212886/event_groups.o ../src/third_party/rtos/FreeRTOS/Source/event_groups.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/404212886/stream_buffer.o: ../src/third_party/rtos/FreeRTOS/Source/stream_buffer.c  .generated_files/flags/default/f976ce429935441dc35812ad3d33d7cfbea9d11 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/404212886" 
	@${RM} ${OBJECTDIR}/_ext/404212886/stream_buffer.o.d 
	@${RM} ${OBJECTDIR}/_ext/404212886/stream_buffer.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/404212886/stream_buffer.o.d" -o ${OBJECTDIR}/_ext/404212886/stream_buffer.o ../src/third_party/rtos/FreeRTOS/Source/stream_buffer.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/iGRVT50/source/csp/sam_csp_codec.o: iGRVT50/source/csp/sam_csp_codec.c  .generated_files/flags/default/ca1f8f25ce81888503036cd4ec66d04a221912c9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/iGRVT50/source/csp" 
	@${RM} ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_codec.o.d 
	@${RM} ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_codec.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/iGRVT50/source/csp/sam_csp_codec.o.d" -o ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_codec.o iGRVT50/source/csp/sam_csp_codec.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/iGRVT50/source/csp/sam_csp_domain.o: iGRVT50/source/csp/sam_csp_domain.c  .generated_files/flags/default/9abd56180a653bf87faa3c5d21c956324f27ba0f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/iGRVT50/source/csp" 
	@${RM} ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_domain.o.d 
	@${RM} ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_domain.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/iGRVT50/source/csp/sam_csp_domain.o.d" -o ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_domain.o iGRVT50/source/csp/sam_csp_domain.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/iGRVT50/source/csp/sam_csp_runtime.o: iGRVT50/source/csp/sam_csp_runtime.c  .generated_files/flags/default/5518d07861b87671e73cd89d209df160f5a170e5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/iGRVT50/source/csp" 
	@${RM} ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_runtime.o.d 
	@${RM} ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_runtime.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/iGRVT50/source/csp/sam_csp_runtime.o.d" -o ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_runtime.o iGRVT50/source/csp/sam_csp_runtime.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/iGRVT50/source/csp/sam_csp_service.o: iGRVT50/source/csp/sam_csp_service.c  .generated_files/flags/default/3ab9361b05f84e805b13f2f657eac75fe15afc30 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/iGRVT50/source/csp" 
	@${RM} ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_service.o.d 
	@${RM} ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_service.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/iGRVT50/source/csp/sam_csp_service.o.d" -o ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_service.o iGRVT50/source/csp/sam_csp_service.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/iGRVT50/source/csp/samv71_rs485_port.o: iGRVT50/source/csp/samv71_rs485_port.c  .generated_files/flags/default/ccef6a0830cf20da948f676f649257b2fa350ed4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/iGRVT50/source/csp" 
	@${RM} ${OBJECTDIR}/iGRVT50/source/csp/samv71_rs485_port.o.d 
	@${RM} ${OBJECTDIR}/iGRVT50/source/csp/samv71_rs485_port.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/iGRVT50/source/csp/samv71_rs485_port.o.d" -o ${OBJECTDIR}/iGRVT50/source/csp/samv71_rs485_port.o iGRVT50/source/csp/samv71_rs485_port.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/iGRVT50/source/hpsolvalve.o: iGRVT50/source/hpsolvalve.c  .generated_files/flags/default/f6275830c5971fb341f76038c7629ce3b307c599 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/iGRVT50/source" 
	@${RM} ${OBJECTDIR}/iGRVT50/source/hpsolvalve.o.d 
	@${RM} ${OBJECTDIR}/iGRVT50/source/hpsolvalve.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/iGRVT50/source/hpsolvalve.o.d" -o ${OBJECTDIR}/iGRVT50/source/hpsolvalve.o iGRVT50/source/hpsolvalve.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/iGRVT50/source/lpsolvalve.o: iGRVT50/source/lpsolvalve.c  .generated_files/flags/default/4c12fd97b210d513c9de1d10dfc634f612bce0f1 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/iGRVT50/source" 
	@${RM} ${OBJECTDIR}/iGRVT50/source/lpsolvalve.o.d 
	@${RM} ${OBJECTDIR}/iGRVT50/source/lpsolvalve.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/iGRVT50/source/lpsolvalve.o.d" -o ${OBJECTDIR}/iGRVT50/source/lpsolvalve.o iGRVT50/source/lpsolvalve.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/iGRVT50/source/sensor.o: iGRVT50/source/sensor.c  .generated_files/flags/default/cfac6dfaf0049cd44d324e371cb17b4615cae960 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/iGRVT50/source" 
	@${RM} ${OBJECTDIR}/iGRVT50/source/sensor.o.d 
	@${RM} ${OBJECTDIR}/iGRVT50/source/sensor.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/iGRVT50/source/sensor.o.d" -o ${OBJECTDIR}/iGRVT50/source/sensor.o iGRVT50/source/sensor.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/iGRVT50/source/statemachine.o: iGRVT50/source/statemachine.c  .generated_files/flags/default/1f136038a9679d228c6b496024d50e5e30890a8e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/iGRVT50/source" 
	@${RM} ${OBJECTDIR}/iGRVT50/source/statemachine.o.d 
	@${RM} ${OBJECTDIR}/iGRVT50/source/statemachine.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/iGRVT50/source/statemachine.o.d" -o ${OBJECTDIR}/iGRVT50/source/statemachine.o iGRVT50/source/statemachine.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1455547172/csp_clock.o: ../third_party/libcsp/src/arch/freertos/csp_clock.c  .generated_files/flags/default/a4e0130ee725425c9551a9c709408748354c48d3 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1455547172" 
	@${RM} ${OBJECTDIR}/_ext/1455547172/csp_clock.o.d 
	@${RM} ${OBJECTDIR}/_ext/1455547172/csp_clock.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1455547172/csp_clock.o.d" -o ${OBJECTDIR}/_ext/1455547172/csp_clock.o ../third_party/libcsp/src/arch/freertos/csp_clock.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1455547172/csp_malloc.o: ../third_party/libcsp/src/arch/freertos/csp_malloc.c  .generated_files/flags/default/1ecfddb7aa9e5113f17acf07cf4268c255942534 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1455547172" 
	@${RM} ${OBJECTDIR}/_ext/1455547172/csp_malloc.o.d 
	@${RM} ${OBJECTDIR}/_ext/1455547172/csp_malloc.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1455547172/csp_malloc.o.d" -o ${OBJECTDIR}/_ext/1455547172/csp_malloc.o ../third_party/libcsp/src/arch/freertos/csp_malloc.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1455547172/csp_queue.o: ../third_party/libcsp/src/arch/freertos/csp_queue.c  .generated_files/flags/default/2d6bce8ab7b8647a4b9141cc714e063dc67968ae .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1455547172" 
	@${RM} ${OBJECTDIR}/_ext/1455547172/csp_queue.o.d 
	@${RM} ${OBJECTDIR}/_ext/1455547172/csp_queue.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1455547172/csp_queue.o.d" -o ${OBJECTDIR}/_ext/1455547172/csp_queue.o ../third_party/libcsp/src/arch/freertos/csp_queue.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1455547172/csp_semaphore.o: ../third_party/libcsp/src/arch/freertos/csp_semaphore.c  .generated_files/flags/default/7305190b5ba65d2451d36d68ebd2eb32f165cb4c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1455547172" 
	@${RM} ${OBJECTDIR}/_ext/1455547172/csp_semaphore.o.d 
	@${RM} ${OBJECTDIR}/_ext/1455547172/csp_semaphore.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1455547172/csp_semaphore.o.d" -o ${OBJECTDIR}/_ext/1455547172/csp_semaphore.o ../third_party/libcsp/src/arch/freertos/csp_semaphore.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1455547172/csp_system.o: ../third_party/libcsp/src/arch/freertos/csp_system.c  .generated_files/flags/default/526857d6cbea82b539906f7b6dd2b2e3e1d52e2f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1455547172" 
	@${RM} ${OBJECTDIR}/_ext/1455547172/csp_system.o.d 
	@${RM} ${OBJECTDIR}/_ext/1455547172/csp_system.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1455547172/csp_system.o.d" -o ${OBJECTDIR}/_ext/1455547172/csp_system.o ../third_party/libcsp/src/arch/freertos/csp_system.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1455547172/csp_thread.o: ../third_party/libcsp/src/arch/freertos/csp_thread.c  .generated_files/flags/default/1afca73de3c6501552bb9b709f1af230dda16cc4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1455547172" 
	@${RM} ${OBJECTDIR}/_ext/1455547172/csp_thread.o.d 
	@${RM} ${OBJECTDIR}/_ext/1455547172/csp_thread.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1455547172/csp_thread.o.d" -o ${OBJECTDIR}/_ext/1455547172/csp_thread.o ../third_party/libcsp/src/arch/freertos/csp_thread.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1455547172/csp_time.o: ../third_party/libcsp/src/arch/freertos/csp_time.c  .generated_files/flags/default/4aae192c7c368bf4fe94e89df908921d4a7e108e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1455547172" 
	@${RM} ${OBJECTDIR}/_ext/1455547172/csp_time.o.d 
	@${RM} ${OBJECTDIR}/_ext/1455547172/csp_time.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1455547172/csp_time.o.d" -o ${OBJECTDIR}/_ext/1455547172/csp_time.o ../third_party/libcsp/src/arch/freertos/csp_time.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/3813883/csp_system.o: ../third_party/libcsp/src/arch/csp_system.c  .generated_files/flags/default/ceb1f5636cff39dc03c8f02dae87df4861b3c7dc .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/3813883" 
	@${RM} ${OBJECTDIR}/_ext/3813883/csp_system.o.d 
	@${RM} ${OBJECTDIR}/_ext/3813883/csp_system.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/3813883/csp_system.o.d" -o ${OBJECTDIR}/_ext/3813883/csp_system.o ../third_party/libcsp/src/arch/csp_system.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/3813883/csp_time.o: ../third_party/libcsp/src/arch/csp_time.c  .generated_files/flags/default/638565ed8eebf469775614720f84cf74b99235fe .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/3813883" 
	@${RM} ${OBJECTDIR}/_ext/3813883/csp_time.o.d 
	@${RM} ${OBJECTDIR}/_ext/3813883/csp_time.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/3813883/csp_time.o.d" -o ${OBJECTDIR}/_ext/3813883/csp_time.o ../third_party/libcsp/src/arch/csp_time.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/687750832/csp_hmac.o: ../third_party/libcsp/src/crypto/csp_hmac.c  .generated_files/flags/default/25d838aeab2d932e56b0daddc1ef6577d1f038a2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/687750832" 
	@${RM} ${OBJECTDIR}/_ext/687750832/csp_hmac.o.d 
	@${RM} ${OBJECTDIR}/_ext/687750832/csp_hmac.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/687750832/csp_hmac.o.d" -o ${OBJECTDIR}/_ext/687750832/csp_hmac.o ../third_party/libcsp/src/crypto/csp_hmac.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/687750832/csp_sha1.o: ../third_party/libcsp/src/crypto/csp_sha1.c  .generated_files/flags/default/478c689527723821043dd5021f0ec47f6c8b8bf0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/687750832" 
	@${RM} ${OBJECTDIR}/_ext/687750832/csp_sha1.o.d 
	@${RM} ${OBJECTDIR}/_ext/687750832/csp_sha1.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/687750832/csp_sha1.o.d" -o ${OBJECTDIR}/_ext/687750832/csp_sha1.o ../third_party/libcsp/src/crypto/csp_sha1.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/687750832/csp_xtea.o: ../third_party/libcsp/src/crypto/csp_xtea.c  .generated_files/flags/default/4894e16dc7f550651ae35d3baf3e3b31d2d14ab3 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/687750832" 
	@${RM} ${OBJECTDIR}/_ext/687750832/csp_xtea.o.d 
	@${RM} ${OBJECTDIR}/_ext/687750832/csp_xtea.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/687750832/csp_xtea.o.d" -o ${OBJECTDIR}/_ext/687750832/csp_xtea.o ../third_party/libcsp/src/crypto/csp_xtea.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/83949769/csp_if_can_pbuf.o: ../third_party/libcsp/src/interfaces/csp_if_can_pbuf.c  .generated_files/flags/default/a270b7e60f6eef61a00b86830f5aa9b174c40e12 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/83949769" 
	@${RM} ${OBJECTDIR}/_ext/83949769/csp_if_can_pbuf.o.d 
	@${RM} ${OBJECTDIR}/_ext/83949769/csp_if_can_pbuf.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/83949769/csp_if_can_pbuf.o.d" -o ${OBJECTDIR}/_ext/83949769/csp_if_can_pbuf.o ../third_party/libcsp/src/interfaces/csp_if_can_pbuf.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/83949769/csp_if_can.o: ../third_party/libcsp/src/interfaces/csp_if_can.c  .generated_files/flags/default/8245963a68c62a8ede05e6af8b35663ef0dd0a9e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/83949769" 
	@${RM} ${OBJECTDIR}/_ext/83949769/csp_if_can.o.d 
	@${RM} ${OBJECTDIR}/_ext/83949769/csp_if_can.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/83949769/csp_if_can.o.d" -o ${OBJECTDIR}/_ext/83949769/csp_if_can.o ../third_party/libcsp/src/interfaces/csp_if_can.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/83949769/csp_if_i2c.o: ../third_party/libcsp/src/interfaces/csp_if_i2c.c  .generated_files/flags/default/f9faa9e5ae775384c31ad16d07b1df4294a8610d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/83949769" 
	@${RM} ${OBJECTDIR}/_ext/83949769/csp_if_i2c.o.d 
	@${RM} ${OBJECTDIR}/_ext/83949769/csp_if_i2c.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/83949769/csp_if_i2c.o.d" -o ${OBJECTDIR}/_ext/83949769/csp_if_i2c.o ../third_party/libcsp/src/interfaces/csp_if_i2c.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/83949769/csp_if_kiss.o: ../third_party/libcsp/src/interfaces/csp_if_kiss.c  .generated_files/flags/default/6401874a47ea4d1cdc4d23a4d38e321523cb7e04 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/83949769" 
	@${RM} ${OBJECTDIR}/_ext/83949769/csp_if_kiss.o.d 
	@${RM} ${OBJECTDIR}/_ext/83949769/csp_if_kiss.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/83949769/csp_if_kiss.o.d" -o ${OBJECTDIR}/_ext/83949769/csp_if_kiss.o ../third_party/libcsp/src/interfaces/csp_if_kiss.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/83949769/csp_if_lo.o: ../third_party/libcsp/src/interfaces/csp_if_lo.c  .generated_files/flags/default/f4dcb9bbf4a17ee540fb07faf4086ada3b5ee85 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/83949769" 
	@${RM} ${OBJECTDIR}/_ext/83949769/csp_if_lo.o.d 
	@${RM} ${OBJECTDIR}/_ext/83949769/csp_if_lo.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/83949769/csp_if_lo.o.d" -o ${OBJECTDIR}/_ext/83949769/csp_if_lo.o ../third_party/libcsp/src/interfaces/csp_if_lo.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/83949769/csp_if_zmqhub.o: ../third_party/libcsp/src/interfaces/csp_if_zmqhub.c  .generated_files/flags/default/bcadbe5d0cc8e8abc60749e0f6dce3894107d37a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/83949769" 
	@${RM} ${OBJECTDIR}/_ext/83949769/csp_if_zmqhub.o.d 
	@${RM} ${OBJECTDIR}/_ext/83949769/csp_if_zmqhub.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/83949769/csp_if_zmqhub.o.d" -o ${OBJECTDIR}/_ext/83949769/csp_if_zmqhub.o ../third_party/libcsp/src/interfaces/csp_if_zmqhub.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1118306443/csp_rtable.o: ../third_party/libcsp/src/rtable/csp_rtable.c  .generated_files/flags/default/c2be3ae8e1d27d3f8fdaddbb389db9c116f30c7e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1118306443" 
	@${RM} ${OBJECTDIR}/_ext/1118306443/csp_rtable.o.d 
	@${RM} ${OBJECTDIR}/_ext/1118306443/csp_rtable.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1118306443/csp_rtable.o.d" -o ${OBJECTDIR}/_ext/1118306443/csp_rtable.o ../third_party/libcsp/src/rtable/csp_rtable.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1118306443/csp_rtable_static.o: ../third_party/libcsp/src/rtable/csp_rtable_static.c  .generated_files/flags/default/5c9e19f2eca15c37f4709c8472e7479234e16e3a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1118306443" 
	@${RM} ${OBJECTDIR}/_ext/1118306443/csp_rtable_static.o.d 
	@${RM} ${OBJECTDIR}/_ext/1118306443/csp_rtable_static.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1118306443/csp_rtable_static.o.d" -o ${OBJECTDIR}/_ext/1118306443/csp_rtable_static.o ../third_party/libcsp/src/rtable/csp_rtable_static.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1245785818/csp_rdp.o: ../third_party/libcsp/src/transport/csp_rdp.c  .generated_files/flags/default/62e3d48bee2c76ebfa1ed84fac170eb7a8141f1b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1245785818" 
	@${RM} ${OBJECTDIR}/_ext/1245785818/csp_rdp.o.d 
	@${RM} ${OBJECTDIR}/_ext/1245785818/csp_rdp.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1245785818/csp_rdp.o.d" -o ${OBJECTDIR}/_ext/1245785818/csp_rdp.o ../third_party/libcsp/src/transport/csp_rdp.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1245785818/csp_udp.o: ../third_party/libcsp/src/transport/csp_udp.c  .generated_files/flags/default/3c787fdb5c72a8608424c74bcab6fcad4a4c5941 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1245785818" 
	@${RM} ${OBJECTDIR}/_ext/1245785818/csp_udp.o.d 
	@${RM} ${OBJECTDIR}/_ext/1245785818/csp_udp.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1245785818/csp_udp.o.d" -o ${OBJECTDIR}/_ext/1245785818/csp_udp.o ../third_party/libcsp/src/transport/csp_udp.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_bridge.o: ../third_party/libcsp/src/csp_bridge.c  .generated_files/flags/default/236192c3a140d6405b7157c4fd432c92848216ae .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_bridge.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_bridge.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_bridge.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_bridge.o ../third_party/libcsp/src/csp_bridge.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_buffer.o: ../third_party/libcsp/src/csp_buffer.c  .generated_files/flags/default/a1660ef51eb18d5e4ab20f4f6d3deae632e87e2b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_buffer.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_buffer.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_buffer.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_buffer.o ../third_party/libcsp/src/csp_buffer.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_conn.o: ../third_party/libcsp/src/csp_conn.c  .generated_files/flags/default/84493d656c0e0a9b43ebce4d12ad0e0a1fdcf8b6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_conn.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_conn.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_conn.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_conn.o ../third_party/libcsp/src/csp_conn.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_crc32.o: ../third_party/libcsp/src/csp_crc32.c  .generated_files/flags/default/fe71a7320726e9006d6d42ddae51808d18ff32e0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_crc32.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_crc32.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_crc32.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_crc32.o ../third_party/libcsp/src/csp_crc32.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_debug.o: ../third_party/libcsp/src/csp_debug.c  .generated_files/flags/default/621c0eb75d2fae8ad8851dbfd39bbabf81d42e5d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_debug.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_debug.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_debug.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_debug.o ../third_party/libcsp/src/csp_debug.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_dedup.o: ../third_party/libcsp/src/csp_dedup.c  .generated_files/flags/default/a33c892d32cbe1815c5a8d42918148fbe914e02b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_dedup.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_dedup.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_dedup.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_dedup.o ../third_party/libcsp/src/csp_dedup.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_endian.o: ../third_party/libcsp/src/csp_endian.c  .generated_files/flags/default/8ebad754cdcb6020cb0cd7e8305ab90b60eafc84 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_endian.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_endian.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_endian.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_endian.o ../third_party/libcsp/src/csp_endian.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_hex_dump.o: ../third_party/libcsp/src/csp_hex_dump.c  .generated_files/flags/default/32840d6374ae4edd72738f21cb3819ead75ba69d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_hex_dump.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_hex_dump.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_hex_dump.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_hex_dump.o ../third_party/libcsp/src/csp_hex_dump.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_iflist.o: ../third_party/libcsp/src/csp_iflist.c  .generated_files/flags/default/f704eae117072771cd0cc675d4396b513dc87f0e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_iflist.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_iflist.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_iflist.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_iflist.o ../third_party/libcsp/src/csp_iflist.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_init.o: ../third_party/libcsp/src/csp_init.c  .generated_files/flags/default/63be0913c37de5a1e30df9e918434439f02fddad .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_init.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_init.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_init.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_init.o ../third_party/libcsp/src/csp_init.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_io.o: ../third_party/libcsp/src/csp_io.c  .generated_files/flags/default/3497a6d7796427623ec8b04be2d3f0f74bd67838 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_io.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_io.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_io.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_io.o ../third_party/libcsp/src/csp_io.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_port.o: ../third_party/libcsp/src/csp_port.c  .generated_files/flags/default/f44ff58094a5890450a59456177e85c6b38dea9d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_port.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_port.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_port.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_port.o ../third_party/libcsp/src/csp_port.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_promisc.o: ../third_party/libcsp/src/csp_promisc.c  .generated_files/flags/default/36cd4448a2b36e1afc131e6ec744ed8cced6d504 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_promisc.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_promisc.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_promisc.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_promisc.o ../third_party/libcsp/src/csp_promisc.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_qfifo.o: ../third_party/libcsp/src/csp_qfifo.c  .generated_files/flags/default/98b62fa7960720e28181e315c0daae81773da9c2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_qfifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_qfifo.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_qfifo.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_qfifo.o ../third_party/libcsp/src/csp_qfifo.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_route.o: ../third_party/libcsp/src/csp_route.c  .generated_files/flags/default/903a3ed08a0ef66876e8e2356818424a71e9c020 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_route.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_route.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_route.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_route.o ../third_party/libcsp/src/csp_route.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_service_handler.o: ../third_party/libcsp/src/csp_service_handler.c  .generated_files/flags/default/ec6f4540f30f684192ad24f187c583a84e21bf00 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_service_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_service_handler.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_service_handler.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_service_handler.o ../third_party/libcsp/src/csp_service_handler.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_services.o: ../third_party/libcsp/src/csp_services.c  .generated_files/flags/default/584cc5a4e004ba0d938c1bae69df649630875694 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_services.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_services.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_services.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_services.o ../third_party/libcsp/src/csp_services.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_sfp.o: ../third_party/libcsp/src/csp_sfp.c  .generated_files/flags/default/954fbb8bbb67ee0caa5709122e7ea9271ba11072 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_sfp.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_sfp.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_sfp.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_sfp.o ../third_party/libcsp/src/csp_sfp.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1360937237/main.o: ../src/main.c  .generated_files/flags/default/47ab115a7ebfb0204b6dfd540907b4e0781919c2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1360937237" 
	@${RM} ${OBJECTDIR}/_ext/1360937237/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/main.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1360937237/main.o.d" -o ${OBJECTDIR}/_ext/1360937237/main.o ../src/main.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1360937237/adc_func.o: ../src/adc_func.c  .generated_files/flags/default/7816190f19c1acd284413b298cec4ec280fbb07a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1360937237" 
	@${RM} ${OBJECTDIR}/_ext/1360937237/adc_func.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/adc_func.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1360937237/adc_func.o.d" -o ${OBJECTDIR}/_ext/1360937237/adc_func.o ../src/adc_func.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1360937237/dbg_task.o: ../src/dbg_task.c  .generated_files/flags/default/9aef7ef0f3c1b154aa5a8162e8702015ecc15d81 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1360937237" 
	@${RM} ${OBJECTDIR}/_ext/1360937237/dbg_task.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/dbg_task.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1360937237/dbg_task.o.d" -o ${OBJECTDIR}/_ext/1360937237/dbg_task.o ../src/dbg_task.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1360937237/opu_task.o: ../src/opu_task.c  .generated_files/flags/default/9d7d4ffa5525ec6745c9a9e37562574912413015 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1360937237" 
	@${RM} ${OBJECTDIR}/_ext/1360937237/opu_task.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/opu_task.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1360937237/opu_task.o.d" -o ${OBJECTDIR}/_ext/1360937237/opu_task.o ../src/opu_task.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1360937237/tc_func.o: ../src/tc_func.c  .generated_files/flags/default/cf72cf13b80ea564e4bf91613eeef063b4baeb6c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1360937237" 
	@${RM} ${OBJECTDIR}/_ext/1360937237/tc_func.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/tc_func.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1360937237/tc_func.o.d" -o ${OBJECTDIR}/_ext/1360937237/tc_func.o ../src/tc_func.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1360937237/pwm_func.o: ../src/pwm_func.c  .generated_files/flags/default/fc4c7cf1b70b84e0403deb4ccfd3cfdad0a6d343 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1360937237" 
	@${RM} ${OBJECTDIR}/_ext/1360937237/pwm_func.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/pwm_func.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1360937237/pwm_func.o.d" -o ${OBJECTDIR}/_ext/1360937237/pwm_func.o ../src/pwm_func.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
else
${OBJECTDIR}/_ext/1865065685/plib_afec1.o: ../src/config/default/peripheral/afec/plib_afec1.c  .generated_files/flags/default/8a5d098f7020ed0e71241b89d4f277b5d6e13af4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1865065685" 
	@${RM} ${OBJECTDIR}/_ext/1865065685/plib_afec1.o.d 
	@${RM} ${OBJECTDIR}/_ext/1865065685/plib_afec1.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1865065685/plib_afec1.o.d" -o ${OBJECTDIR}/_ext/1865065685/plib_afec1.o ../src/config/default/peripheral/afec/plib_afec1.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1865065685/plib_afec0.o: ../src/config/default/peripheral/afec/plib_afec0.c  .generated_files/flags/default/610ec53e9dcd244a4103b54d6cde0b22748f0baa .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1865065685" 
	@${RM} ${OBJECTDIR}/_ext/1865065685/plib_afec0.o.d 
	@${RM} ${OBJECTDIR}/_ext/1865065685/plib_afec0.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1865065685/plib_afec0.o.d" -o ${OBJECTDIR}/_ext/1865065685/plib_afec0.o ../src/config/default/peripheral/afec/plib_afec0.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/60165520/plib_clk.o: ../src/config/default/peripheral/clk/plib_clk.c  .generated_files/flags/default/ef6ee2981c885bcbb8f6edf2be71e978d4b11f77 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/60165520" 
	@${RM} ${OBJECTDIR}/_ext/60165520/plib_clk.o.d 
	@${RM} ${OBJECTDIR}/_ext/60165520/plib_clk.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/60165520/plib_clk.o.d" -o ${OBJECTDIR}/_ext/60165520/plib_clk.o ../src/config/default/peripheral/clk/plib_clk.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/60167248/plib_efc.o: ../src/config/default/peripheral/efc/plib_efc.c  .generated_files/flags/default/92b591f453b7b26d6dd2b2ee6e9ea7c99f124623 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/60167248" 
	@${RM} ${OBJECTDIR}/_ext/60167248/plib_efc.o.d 
	@${RM} ${OBJECTDIR}/_ext/60167248/plib_efc.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/60167248/plib_efc.o.d" -o ${OBJECTDIR}/_ext/60167248/plib_efc.o ../src/config/default/peripheral/efc/plib_efc.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1865468468/plib_nvic.o: ../src/config/default/peripheral/nvic/plib_nvic.c  .generated_files/flags/default/7b64d59a792f40388ed3fc7236a8cd9873dc3399 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1865468468" 
	@${RM} ${OBJECTDIR}/_ext/1865468468/plib_nvic.o.d 
	@${RM} ${OBJECTDIR}/_ext/1865468468/plib_nvic.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1865468468/plib_nvic.o.d" -o ${OBJECTDIR}/_ext/1865468468/plib_nvic.o ../src/config/default/peripheral/nvic/plib_nvic.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/60177924/plib_pio.o: ../src/config/default/peripheral/pio/plib_pio.c  .generated_files/flags/default/8ce1978fd9c2c63188b48ca1c932c1e3ed631710 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/60177924" 
	@${RM} ${OBJECTDIR}/_ext/60177924/plib_pio.o.d 
	@${RM} ${OBJECTDIR}/_ext/60177924/plib_pio.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/60177924/plib_pio.o.d" -o ${OBJECTDIR}/_ext/60177924/plib_pio.o ../src/config/default/peripheral/pio/plib_pio.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/60178356/plib_pwm0.o: ../src/config/default/peripheral/pwm/plib_pwm0.c  .generated_files/flags/default/abd520edfb15de36d17c67c4407923f76ae8789a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/60178356" 
	@${RM} ${OBJECTDIR}/_ext/60178356/plib_pwm0.o.d 
	@${RM} ${OBJECTDIR}/_ext/60178356/plib_pwm0.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/60178356/plib_pwm0.o.d" -o ${OBJECTDIR}/_ext/60178356/plib_pwm0.o ../src/config/default/peripheral/pwm/plib_pwm0.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/60178356/plib_pwm1.o: ../src/config/default/peripheral/pwm/plib_pwm1.c  .generated_files/flags/default/cd1ebebec686ccaed3784c88a089b65c12faaeb4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/60178356" 
	@${RM} ${OBJECTDIR}/_ext/60178356/plib_pwm1.o.d 
	@${RM} ${OBJECTDIR}/_ext/60178356/plib_pwm1.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/60178356/plib_pwm1.o.d" -o ${OBJECTDIR}/_ext/60178356/plib_pwm1.o ../src/config/default/peripheral/pwm/plib_pwm1.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/298189674/plib_spi0_master.o: ../src/config/default/peripheral/spi/spi_master/plib_spi0_master.c  .generated_files/flags/default/dd59a87f5d009059a04faa58b17310f978b16a4c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/298189674" 
	@${RM} ${OBJECTDIR}/_ext/298189674/plib_spi0_master.o.d 
	@${RM} ${OBJECTDIR}/_ext/298189674/plib_spi0_master.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/298189674/plib_spi0_master.o.d" -o ${OBJECTDIR}/_ext/298189674/plib_spi0_master.o ../src/config/default/peripheral/spi/spi_master/plib_spi0_master.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/298189674/plib_spi1_master.o: ../src/config/default/peripheral/spi/spi_master/plib_spi1_master.c  .generated_files/flags/default/50626870308e8391bd324772d6fb613dd8de8b62 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/298189674" 
	@${RM} ${OBJECTDIR}/_ext/298189674/plib_spi1_master.o.d 
	@${RM} ${OBJECTDIR}/_ext/298189674/plib_spi1_master.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/298189674/plib_spi1_master.o.d" -o ${OBJECTDIR}/_ext/298189674/plib_spi1_master.o ../src/config/default/peripheral/spi/spi_master/plib_spi1_master.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1827571544/plib_systick.o: ../src/config/default/peripheral/systick/plib_systick.c  .generated_files/flags/default/46559b8b35ff81077f5cb12b3f74d44c3dd6a13f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1827571544" 
	@${RM} ${OBJECTDIR}/_ext/1827571544/plib_systick.o.d 
	@${RM} ${OBJECTDIR}/_ext/1827571544/plib_systick.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1827571544/plib_systick.o.d" -o ${OBJECTDIR}/_ext/1827571544/plib_systick.o ../src/config/default/peripheral/systick/plib_systick.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/829342655/plib_tc1.o: ../src/config/default/peripheral/tc/plib_tc1.c  .generated_files/flags/default/7df7ffdc0a49f9e865ad6fdc2010d2d926355fbd .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/829342655" 
	@${RM} ${OBJECTDIR}/_ext/829342655/plib_tc1.o.d 
	@${RM} ${OBJECTDIR}/_ext/829342655/plib_tc1.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/829342655/plib_tc1.o.d" -o ${OBJECTDIR}/_ext/829342655/plib_tc1.o ../src/config/default/peripheral/tc/plib_tc1.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/829342655/plib_tc0.o: ../src/config/default/peripheral/tc/plib_tc0.c  .generated_files/flags/default/a8e7d3c3fcfb32e2aaa329c84b60dbb74b6ea5be .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/829342655" 
	@${RM} ${OBJECTDIR}/_ext/829342655/plib_tc0.o.d 
	@${RM} ${OBJECTDIR}/_ext/829342655/plib_tc0.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/829342655/plib_tc0.o.d" -o ${OBJECTDIR}/_ext/829342655/plib_tc0.o ../src/config/default/peripheral/tc/plib_tc0.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/829342655/plib_tc3.o: ../src/config/default/peripheral/tc/plib_tc3.c  .generated_files/flags/default/7835684e7215cd942b5373f0cf6f904c85db6594 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/829342655" 
	@${RM} ${OBJECTDIR}/_ext/829342655/plib_tc3.o.d 
	@${RM} ${OBJECTDIR}/_ext/829342655/plib_tc3.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/829342655/plib_tc3.o.d" -o ${OBJECTDIR}/_ext/829342655/plib_tc3.o ../src/config/default/peripheral/tc/plib_tc3.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/2001315827/plib_usart0.o: ../src/config/default/peripheral/usart/plib_usart0.c  .generated_files/flags/default/522b57a429813f1886833829e6ac3132035d7579 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/2001315827" 
	@${RM} ${OBJECTDIR}/_ext/2001315827/plib_usart0.o.d 
	@${RM} ${OBJECTDIR}/_ext/2001315827/plib_usart0.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/2001315827/plib_usart0.o.d" -o ${OBJECTDIR}/_ext/2001315827/plib_usart0.o ../src/config/default/peripheral/usart/plib_usart0.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/2001315827/plib_usart1.o: ../src/config/default/peripheral/usart/plib_usart1.c  .generated_files/flags/default/38cc617f31673c276f2ff065a113ff6daaba803d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/2001315827" 
	@${RM} ${OBJECTDIR}/_ext/2001315827/plib_usart1.o.d 
	@${RM} ${OBJECTDIR}/_ext/2001315827/plib_usart1.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/2001315827/plib_usart1.o.d" -o ${OBJECTDIR}/_ext/2001315827/plib_usart1.o ../src/config/default/peripheral/usart/plib_usart1.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/163028504/xc32_monitor.o: ../src/config/default/stdio/xc32_monitor.c  .generated_files/flags/default/65c660dda46a2156ab14b31b0576fd21cff4fb0a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/163028504" 
	@${RM} ${OBJECTDIR}/_ext/163028504/xc32_monitor.o.d 
	@${RM} ${OBJECTDIR}/_ext/163028504/xc32_monitor.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/163028504/xc32_monitor.o.d" -o ${OBJECTDIR}/_ext/163028504/xc32_monitor.o ../src/config/default/stdio/xc32_monitor.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1171490990/initialization.o: ../src/config/default/initialization.c  .generated_files/flags/default/6f9d2441897a6e37eaf3ee91fcd287761bb40cb9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1171490990" 
	@${RM} ${OBJECTDIR}/_ext/1171490990/initialization.o.d 
	@${RM} ${OBJECTDIR}/_ext/1171490990/initialization.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1171490990/initialization.o.d" -o ${OBJECTDIR}/_ext/1171490990/initialization.o ../src/config/default/initialization.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1171490990/interrupts.o: ../src/config/default/interrupts.c  .generated_files/flags/default/d3c5217ce75372a3e6e3ce28e7fae483b6af21e5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1171490990" 
	@${RM} ${OBJECTDIR}/_ext/1171490990/interrupts.o.d 
	@${RM} ${OBJECTDIR}/_ext/1171490990/interrupts.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1171490990/interrupts.o.d" -o ${OBJECTDIR}/_ext/1171490990/interrupts.o ../src/config/default/interrupts.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1171490990/exceptions.o: ../src/config/default/exceptions.c  .generated_files/flags/default/3a6d14e7abe7a517c45275d2fef6fc918a1f8a01 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1171490990" 
	@${RM} ${OBJECTDIR}/_ext/1171490990/exceptions.o.d 
	@${RM} ${OBJECTDIR}/_ext/1171490990/exceptions.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1171490990/exceptions.o.d" -o ${OBJECTDIR}/_ext/1171490990/exceptions.o ../src/config/default/exceptions.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1171490990/startup_xc32.o: ../src/config/default/startup_xc32.c  .generated_files/flags/default/485848d2de45bbc2c37a8233119fd5edb9ca23d3 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1171490990" 
	@${RM} ${OBJECTDIR}/_ext/1171490990/startup_xc32.o.d 
	@${RM} ${OBJECTDIR}/_ext/1171490990/startup_xc32.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1171490990/startup_xc32.o.d" -o ${OBJECTDIR}/_ext/1171490990/startup_xc32.o ../src/config/default/startup_xc32.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1171490990/libc_syscalls.o: ../src/config/default/libc_syscalls.c  .generated_files/flags/default/b42e1dc79de87b02abce84228637432cac9f5ac5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1171490990" 
	@${RM} ${OBJECTDIR}/_ext/1171490990/libc_syscalls.o.d 
	@${RM} ${OBJECTDIR}/_ext/1171490990/libc_syscalls.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1171490990/libc_syscalls.o.d" -o ${OBJECTDIR}/_ext/1171490990/libc_syscalls.o ../src/config/default/libc_syscalls.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1171490990/freertos_hooks.o: ../src/config/default/freertos_hooks.c  .generated_files/flags/default/2b9af9fb3d96b53232bb94bf467648b669600709 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1171490990" 
	@${RM} ${OBJECTDIR}/_ext/1171490990/freertos_hooks.o.d 
	@${RM} ${OBJECTDIR}/_ext/1171490990/freertos_hooks.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1171490990/freertos_hooks.o.d" -o ${OBJECTDIR}/_ext/1171490990/freertos_hooks.o ../src/config/default/freertos_hooks.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/2036275640/csp_rs485_freertos.o: ../third_party/csp-rs485/src/csp_rs485_freertos.c  .generated_files/flags/default/7434ce02bce8bc503f0266d5cad12ada1421d5e0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/2036275640" 
	@${RM} ${OBJECTDIR}/_ext/2036275640/csp_rs485_freertos.o.d 
	@${RM} ${OBJECTDIR}/_ext/2036275640/csp_rs485_freertos.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/2036275640/csp_rs485_freertos.o.d" -o ${OBJECTDIR}/_ext/2036275640/csp_rs485_freertos.o ../third_party/csp-rs485/src/csp_rs485_freertos.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/2036275640/csp_rs485_kiss.o: ../third_party/csp-rs485/src/csp_rs485_kiss.c  .generated_files/flags/default/847830f8f532896abad4b565cac9439fd80a8083 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/2036275640" 
	@${RM} ${OBJECTDIR}/_ext/2036275640/csp_rs485_kiss.o.d 
	@${RM} ${OBJECTDIR}/_ext/2036275640/csp_rs485_kiss.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/2036275640/csp_rs485_kiss.o.d" -o ${OBJECTDIR}/_ext/2036275640/csp_rs485_kiss.o ../third_party/csp-rs485/src/csp_rs485_kiss.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/2036275640/csp_rs485_link.o: ../third_party/csp-rs485/src/csp_rs485_link.c  .generated_files/flags/default/48f3ab2ae0075d4552c3f2057e7654aeff3084e9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/2036275640" 
	@${RM} ${OBJECTDIR}/_ext/2036275640/csp_rs485_link.o.d 
	@${RM} ${OBJECTDIR}/_ext/2036275640/csp_rs485_link.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/2036275640/csp_rs485_link.o.d" -o ${OBJECTDIR}/_ext/2036275640/csp_rs485_link.o ../third_party/csp-rs485/src/csp_rs485_link.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/2036275640/csp_rs485_supervisor.o: ../third_party/csp-rs485/src/csp_rs485_supervisor.c  .generated_files/flags/default/cb9f6605cba894b6ada213e02954c9c57c86dfcd .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/2036275640" 
	@${RM} ${OBJECTDIR}/_ext/2036275640/csp_rs485_supervisor.o.d 
	@${RM} ${OBJECTDIR}/_ext/2036275640/csp_rs485_supervisor.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/2036275640/csp_rs485_supervisor.o.d" -o ${OBJECTDIR}/_ext/2036275640/csp_rs485_supervisor.o ../third_party/csp-rs485/src/csp_rs485_supervisor.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/977623654/port.o: ../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7/port.c  .generated_files/flags/default/56b15c6ee0035f0e7c433e5ec76c5cd545563612 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/977623654" 
	@${RM} ${OBJECTDIR}/_ext/977623654/port.o.d 
	@${RM} ${OBJECTDIR}/_ext/977623654/port.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/977623654/port.o.d" -o ${OBJECTDIR}/_ext/977623654/port.o ../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7/port.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1665200909/heap_1.o: ../src/third_party/rtos/FreeRTOS/Source/portable/MemMang/heap_1.c  .generated_files/flags/default/81a17a4a3afc461d6afeadfe2ce28128179330e4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1665200909" 
	@${RM} ${OBJECTDIR}/_ext/1665200909/heap_1.o.d 
	@${RM} ${OBJECTDIR}/_ext/1665200909/heap_1.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1665200909/heap_1.o.d" -o ${OBJECTDIR}/_ext/1665200909/heap_1.o ../src/third_party/rtos/FreeRTOS/Source/portable/MemMang/heap_1.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/404212886/croutine.o: ../src/third_party/rtos/FreeRTOS/Source/croutine.c  .generated_files/flags/default/862cb3e98e65e26f1a288d5ec9d735d9f31c26a2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/404212886" 
	@${RM} ${OBJECTDIR}/_ext/404212886/croutine.o.d 
	@${RM} ${OBJECTDIR}/_ext/404212886/croutine.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/404212886/croutine.o.d" -o ${OBJECTDIR}/_ext/404212886/croutine.o ../src/third_party/rtos/FreeRTOS/Source/croutine.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/404212886/list.o: ../src/third_party/rtos/FreeRTOS/Source/list.c  .generated_files/flags/default/40f46e94c26a5c73b7a6fb889a9efb42821c7928 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/404212886" 
	@${RM} ${OBJECTDIR}/_ext/404212886/list.o.d 
	@${RM} ${OBJECTDIR}/_ext/404212886/list.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/404212886/list.o.d" -o ${OBJECTDIR}/_ext/404212886/list.o ../src/third_party/rtos/FreeRTOS/Source/list.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/404212886/queue.o: ../src/third_party/rtos/FreeRTOS/Source/queue.c  .generated_files/flags/default/f58f36c5e386d06478edb247862a690f79f67be2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/404212886" 
	@${RM} ${OBJECTDIR}/_ext/404212886/queue.o.d 
	@${RM} ${OBJECTDIR}/_ext/404212886/queue.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/404212886/queue.o.d" -o ${OBJECTDIR}/_ext/404212886/queue.o ../src/third_party/rtos/FreeRTOS/Source/queue.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/404212886/FreeRTOS_tasks.o: ../src/third_party/rtos/FreeRTOS/Source/FreeRTOS_tasks.c  .generated_files/flags/default/bf7548c89b161dd7b44e69341efd3d828d7b94c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/404212886" 
	@${RM} ${OBJECTDIR}/_ext/404212886/FreeRTOS_tasks.o.d 
	@${RM} ${OBJECTDIR}/_ext/404212886/FreeRTOS_tasks.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/404212886/FreeRTOS_tasks.o.d" -o ${OBJECTDIR}/_ext/404212886/FreeRTOS_tasks.o ../src/third_party/rtos/FreeRTOS/Source/FreeRTOS_tasks.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/404212886/timers.o: ../src/third_party/rtos/FreeRTOS/Source/timers.c  .generated_files/flags/default/ab6564827c70fe74f9ed2852c43f0d8e281d616f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/404212886" 
	@${RM} ${OBJECTDIR}/_ext/404212886/timers.o.d 
	@${RM} ${OBJECTDIR}/_ext/404212886/timers.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/404212886/timers.o.d" -o ${OBJECTDIR}/_ext/404212886/timers.o ../src/third_party/rtos/FreeRTOS/Source/timers.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/404212886/event_groups.o: ../src/third_party/rtos/FreeRTOS/Source/event_groups.c  .generated_files/flags/default/914f9cfc7d008548ad538a8366fccbd49e456310 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/404212886" 
	@${RM} ${OBJECTDIR}/_ext/404212886/event_groups.o.d 
	@${RM} ${OBJECTDIR}/_ext/404212886/event_groups.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/404212886/event_groups.o.d" -o ${OBJECTDIR}/_ext/404212886/event_groups.o ../src/third_party/rtos/FreeRTOS/Source/event_groups.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/404212886/stream_buffer.o: ../src/third_party/rtos/FreeRTOS/Source/stream_buffer.c  .generated_files/flags/default/b46178c63b425547bd98f7b8606fe1620ff54071 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/404212886" 
	@${RM} ${OBJECTDIR}/_ext/404212886/stream_buffer.o.d 
	@${RM} ${OBJECTDIR}/_ext/404212886/stream_buffer.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/404212886/stream_buffer.o.d" -o ${OBJECTDIR}/_ext/404212886/stream_buffer.o ../src/third_party/rtos/FreeRTOS/Source/stream_buffer.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/iGRVT50/source/csp/sam_csp_codec.o: iGRVT50/source/csp/sam_csp_codec.c  .generated_files/flags/default/22cfccaa52efdcc668bd6051f7581cc62ca2ff17 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/iGRVT50/source/csp" 
	@${RM} ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_codec.o.d 
	@${RM} ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_codec.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/iGRVT50/source/csp/sam_csp_codec.o.d" -o ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_codec.o iGRVT50/source/csp/sam_csp_codec.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/iGRVT50/source/csp/sam_csp_domain.o: iGRVT50/source/csp/sam_csp_domain.c  .generated_files/flags/default/45c18b77a805e242ef5106055157b1bd3550d1b5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/iGRVT50/source/csp" 
	@${RM} ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_domain.o.d 
	@${RM} ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_domain.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/iGRVT50/source/csp/sam_csp_domain.o.d" -o ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_domain.o iGRVT50/source/csp/sam_csp_domain.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/iGRVT50/source/csp/sam_csp_runtime.o: iGRVT50/source/csp/sam_csp_runtime.c  .generated_files/flags/default/c10445dd2b5ff378f6623427f5d8b43a7f6528dd .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/iGRVT50/source/csp" 
	@${RM} ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_runtime.o.d 
	@${RM} ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_runtime.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/iGRVT50/source/csp/sam_csp_runtime.o.d" -o ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_runtime.o iGRVT50/source/csp/sam_csp_runtime.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/iGRVT50/source/csp/sam_csp_service.o: iGRVT50/source/csp/sam_csp_service.c  .generated_files/flags/default/c20676e1f937736c576155e2f1229dd514e9625f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/iGRVT50/source/csp" 
	@${RM} ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_service.o.d 
	@${RM} ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_service.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/iGRVT50/source/csp/sam_csp_service.o.d" -o ${OBJECTDIR}/iGRVT50/source/csp/sam_csp_service.o iGRVT50/source/csp/sam_csp_service.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/iGRVT50/source/csp/samv71_rs485_port.o: iGRVT50/source/csp/samv71_rs485_port.c  .generated_files/flags/default/73de9785a5cf83157f669d4e6c9c4dd639a9942 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/iGRVT50/source/csp" 
	@${RM} ${OBJECTDIR}/iGRVT50/source/csp/samv71_rs485_port.o.d 
	@${RM} ${OBJECTDIR}/iGRVT50/source/csp/samv71_rs485_port.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/iGRVT50/source/csp/samv71_rs485_port.o.d" -o ${OBJECTDIR}/iGRVT50/source/csp/samv71_rs485_port.o iGRVT50/source/csp/samv71_rs485_port.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/iGRVT50/source/hpsolvalve.o: iGRVT50/source/hpsolvalve.c  .generated_files/flags/default/1ab72b7bc73a43c6b59fba40501acb56b47fbaf4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/iGRVT50/source" 
	@${RM} ${OBJECTDIR}/iGRVT50/source/hpsolvalve.o.d 
	@${RM} ${OBJECTDIR}/iGRVT50/source/hpsolvalve.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/iGRVT50/source/hpsolvalve.o.d" -o ${OBJECTDIR}/iGRVT50/source/hpsolvalve.o iGRVT50/source/hpsolvalve.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/iGRVT50/source/lpsolvalve.o: iGRVT50/source/lpsolvalve.c  .generated_files/flags/default/ffc74cf9add9b1273808831b22e20a4546cbe7c1 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/iGRVT50/source" 
	@${RM} ${OBJECTDIR}/iGRVT50/source/lpsolvalve.o.d 
	@${RM} ${OBJECTDIR}/iGRVT50/source/lpsolvalve.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/iGRVT50/source/lpsolvalve.o.d" -o ${OBJECTDIR}/iGRVT50/source/lpsolvalve.o iGRVT50/source/lpsolvalve.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/iGRVT50/source/sensor.o: iGRVT50/source/sensor.c  .generated_files/flags/default/87bc86e0d4b66dbc57f3a601de60a42e4f996c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/iGRVT50/source" 
	@${RM} ${OBJECTDIR}/iGRVT50/source/sensor.o.d 
	@${RM} ${OBJECTDIR}/iGRVT50/source/sensor.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/iGRVT50/source/sensor.o.d" -o ${OBJECTDIR}/iGRVT50/source/sensor.o iGRVT50/source/sensor.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/iGRVT50/source/statemachine.o: iGRVT50/source/statemachine.c  .generated_files/flags/default/197616b51af4ce49de7da44a051dd5e64dc6cc2d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/iGRVT50/source" 
	@${RM} ${OBJECTDIR}/iGRVT50/source/statemachine.o.d 
	@${RM} ${OBJECTDIR}/iGRVT50/source/statemachine.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/iGRVT50/source/statemachine.o.d" -o ${OBJECTDIR}/iGRVT50/source/statemachine.o iGRVT50/source/statemachine.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1455547172/csp_clock.o: ../third_party/libcsp/src/arch/freertos/csp_clock.c  .generated_files/flags/default/92bc54923a1c04326633c736b684f03ebf0b5a2b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1455547172" 
	@${RM} ${OBJECTDIR}/_ext/1455547172/csp_clock.o.d 
	@${RM} ${OBJECTDIR}/_ext/1455547172/csp_clock.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1455547172/csp_clock.o.d" -o ${OBJECTDIR}/_ext/1455547172/csp_clock.o ../third_party/libcsp/src/arch/freertos/csp_clock.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1455547172/csp_malloc.o: ../third_party/libcsp/src/arch/freertos/csp_malloc.c  .generated_files/flags/default/e7854f5965844caff821db6df7b48ef8df8575b9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1455547172" 
	@${RM} ${OBJECTDIR}/_ext/1455547172/csp_malloc.o.d 
	@${RM} ${OBJECTDIR}/_ext/1455547172/csp_malloc.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1455547172/csp_malloc.o.d" -o ${OBJECTDIR}/_ext/1455547172/csp_malloc.o ../third_party/libcsp/src/arch/freertos/csp_malloc.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1455547172/csp_queue.o: ../third_party/libcsp/src/arch/freertos/csp_queue.c  .generated_files/flags/default/64b49f39a7a203983b5e98b30e8aad0611223adb .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1455547172" 
	@${RM} ${OBJECTDIR}/_ext/1455547172/csp_queue.o.d 
	@${RM} ${OBJECTDIR}/_ext/1455547172/csp_queue.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1455547172/csp_queue.o.d" -o ${OBJECTDIR}/_ext/1455547172/csp_queue.o ../third_party/libcsp/src/arch/freertos/csp_queue.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1455547172/csp_semaphore.o: ../third_party/libcsp/src/arch/freertos/csp_semaphore.c  .generated_files/flags/default/2228a39b9aaf5a33a8d6605e7c4332ab2eeb9faa .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1455547172" 
	@${RM} ${OBJECTDIR}/_ext/1455547172/csp_semaphore.o.d 
	@${RM} ${OBJECTDIR}/_ext/1455547172/csp_semaphore.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1455547172/csp_semaphore.o.d" -o ${OBJECTDIR}/_ext/1455547172/csp_semaphore.o ../third_party/libcsp/src/arch/freertos/csp_semaphore.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1455547172/csp_system.o: ../third_party/libcsp/src/arch/freertos/csp_system.c  .generated_files/flags/default/a6915e48f3205960c0f68beb39103e726981e30 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1455547172" 
	@${RM} ${OBJECTDIR}/_ext/1455547172/csp_system.o.d 
	@${RM} ${OBJECTDIR}/_ext/1455547172/csp_system.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1455547172/csp_system.o.d" -o ${OBJECTDIR}/_ext/1455547172/csp_system.o ../third_party/libcsp/src/arch/freertos/csp_system.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1455547172/csp_thread.o: ../third_party/libcsp/src/arch/freertos/csp_thread.c  .generated_files/flags/default/41f4db24f805797d2628f0b07ca0640e1a2ded1d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1455547172" 
	@${RM} ${OBJECTDIR}/_ext/1455547172/csp_thread.o.d 
	@${RM} ${OBJECTDIR}/_ext/1455547172/csp_thread.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1455547172/csp_thread.o.d" -o ${OBJECTDIR}/_ext/1455547172/csp_thread.o ../third_party/libcsp/src/arch/freertos/csp_thread.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1455547172/csp_time.o: ../third_party/libcsp/src/arch/freertos/csp_time.c  .generated_files/flags/default/128c2df7b0f2ab1fba307244cc2e3b07143ef145 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1455547172" 
	@${RM} ${OBJECTDIR}/_ext/1455547172/csp_time.o.d 
	@${RM} ${OBJECTDIR}/_ext/1455547172/csp_time.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1455547172/csp_time.o.d" -o ${OBJECTDIR}/_ext/1455547172/csp_time.o ../third_party/libcsp/src/arch/freertos/csp_time.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/3813883/csp_system.o: ../third_party/libcsp/src/arch/csp_system.c  .generated_files/flags/default/50746ca2c5af18eac935ca5bd588511c88d2efa .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/3813883" 
	@${RM} ${OBJECTDIR}/_ext/3813883/csp_system.o.d 
	@${RM} ${OBJECTDIR}/_ext/3813883/csp_system.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/3813883/csp_system.o.d" -o ${OBJECTDIR}/_ext/3813883/csp_system.o ../third_party/libcsp/src/arch/csp_system.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/3813883/csp_time.o: ../third_party/libcsp/src/arch/csp_time.c  .generated_files/flags/default/a956bcd251713ff02e5ca56c6d9a8b46cd37f88f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/3813883" 
	@${RM} ${OBJECTDIR}/_ext/3813883/csp_time.o.d 
	@${RM} ${OBJECTDIR}/_ext/3813883/csp_time.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/3813883/csp_time.o.d" -o ${OBJECTDIR}/_ext/3813883/csp_time.o ../third_party/libcsp/src/arch/csp_time.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/687750832/csp_hmac.o: ../third_party/libcsp/src/crypto/csp_hmac.c  .generated_files/flags/default/263fa0d208a0164ea27ef7665fa1c783ea3535b0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/687750832" 
	@${RM} ${OBJECTDIR}/_ext/687750832/csp_hmac.o.d 
	@${RM} ${OBJECTDIR}/_ext/687750832/csp_hmac.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/687750832/csp_hmac.o.d" -o ${OBJECTDIR}/_ext/687750832/csp_hmac.o ../third_party/libcsp/src/crypto/csp_hmac.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/687750832/csp_sha1.o: ../third_party/libcsp/src/crypto/csp_sha1.c  .generated_files/flags/default/701fd2d8b7551e168336f7619d85bd7bc5c3ce5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/687750832" 
	@${RM} ${OBJECTDIR}/_ext/687750832/csp_sha1.o.d 
	@${RM} ${OBJECTDIR}/_ext/687750832/csp_sha1.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/687750832/csp_sha1.o.d" -o ${OBJECTDIR}/_ext/687750832/csp_sha1.o ../third_party/libcsp/src/crypto/csp_sha1.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/687750832/csp_xtea.o: ../third_party/libcsp/src/crypto/csp_xtea.c  .generated_files/flags/default/c767f3c401b3ceab7b9c64aa76cdca48b0a1d923 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/687750832" 
	@${RM} ${OBJECTDIR}/_ext/687750832/csp_xtea.o.d 
	@${RM} ${OBJECTDIR}/_ext/687750832/csp_xtea.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/687750832/csp_xtea.o.d" -o ${OBJECTDIR}/_ext/687750832/csp_xtea.o ../third_party/libcsp/src/crypto/csp_xtea.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/83949769/csp_if_can_pbuf.o: ../third_party/libcsp/src/interfaces/csp_if_can_pbuf.c  .generated_files/flags/default/91ba03f7965696fed160ace4f4b82d154e9c9b94 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/83949769" 
	@${RM} ${OBJECTDIR}/_ext/83949769/csp_if_can_pbuf.o.d 
	@${RM} ${OBJECTDIR}/_ext/83949769/csp_if_can_pbuf.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/83949769/csp_if_can_pbuf.o.d" -o ${OBJECTDIR}/_ext/83949769/csp_if_can_pbuf.o ../third_party/libcsp/src/interfaces/csp_if_can_pbuf.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/83949769/csp_if_can.o: ../third_party/libcsp/src/interfaces/csp_if_can.c  .generated_files/flags/default/a140c6805d2e7ebebebd3dd3e35dd4151e08cf04 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/83949769" 
	@${RM} ${OBJECTDIR}/_ext/83949769/csp_if_can.o.d 
	@${RM} ${OBJECTDIR}/_ext/83949769/csp_if_can.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/83949769/csp_if_can.o.d" -o ${OBJECTDIR}/_ext/83949769/csp_if_can.o ../third_party/libcsp/src/interfaces/csp_if_can.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/83949769/csp_if_i2c.o: ../third_party/libcsp/src/interfaces/csp_if_i2c.c  .generated_files/flags/default/cefb1d32a09ecf22cbd47287a8143c11d0fd6b4f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/83949769" 
	@${RM} ${OBJECTDIR}/_ext/83949769/csp_if_i2c.o.d 
	@${RM} ${OBJECTDIR}/_ext/83949769/csp_if_i2c.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/83949769/csp_if_i2c.o.d" -o ${OBJECTDIR}/_ext/83949769/csp_if_i2c.o ../third_party/libcsp/src/interfaces/csp_if_i2c.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/83949769/csp_if_kiss.o: ../third_party/libcsp/src/interfaces/csp_if_kiss.c  .generated_files/flags/default/9267d877e6f96df5cb218d1d7b64fd6476a41e18 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/83949769" 
	@${RM} ${OBJECTDIR}/_ext/83949769/csp_if_kiss.o.d 
	@${RM} ${OBJECTDIR}/_ext/83949769/csp_if_kiss.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/83949769/csp_if_kiss.o.d" -o ${OBJECTDIR}/_ext/83949769/csp_if_kiss.o ../third_party/libcsp/src/interfaces/csp_if_kiss.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/83949769/csp_if_lo.o: ../third_party/libcsp/src/interfaces/csp_if_lo.c  .generated_files/flags/default/c97086a3a3458f1d6c876244750290791c1b0d6e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/83949769" 
	@${RM} ${OBJECTDIR}/_ext/83949769/csp_if_lo.o.d 
	@${RM} ${OBJECTDIR}/_ext/83949769/csp_if_lo.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/83949769/csp_if_lo.o.d" -o ${OBJECTDIR}/_ext/83949769/csp_if_lo.o ../third_party/libcsp/src/interfaces/csp_if_lo.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/83949769/csp_if_zmqhub.o: ../third_party/libcsp/src/interfaces/csp_if_zmqhub.c  .generated_files/flags/default/5faac8cd2dd59295392234fa25cfb348118fbd5b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/83949769" 
	@${RM} ${OBJECTDIR}/_ext/83949769/csp_if_zmqhub.o.d 
	@${RM} ${OBJECTDIR}/_ext/83949769/csp_if_zmqhub.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/83949769/csp_if_zmqhub.o.d" -o ${OBJECTDIR}/_ext/83949769/csp_if_zmqhub.o ../third_party/libcsp/src/interfaces/csp_if_zmqhub.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1118306443/csp_rtable.o: ../third_party/libcsp/src/rtable/csp_rtable.c  .generated_files/flags/default/699c779153936a903bb58e7e6e5d8c919ec73f4f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1118306443" 
	@${RM} ${OBJECTDIR}/_ext/1118306443/csp_rtable.o.d 
	@${RM} ${OBJECTDIR}/_ext/1118306443/csp_rtable.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1118306443/csp_rtable.o.d" -o ${OBJECTDIR}/_ext/1118306443/csp_rtable.o ../third_party/libcsp/src/rtable/csp_rtable.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1118306443/csp_rtable_static.o: ../third_party/libcsp/src/rtable/csp_rtable_static.c  .generated_files/flags/default/bc2bb5b99a4ca8a26b9759481ffaee255455ea06 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1118306443" 
	@${RM} ${OBJECTDIR}/_ext/1118306443/csp_rtable_static.o.d 
	@${RM} ${OBJECTDIR}/_ext/1118306443/csp_rtable_static.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1118306443/csp_rtable_static.o.d" -o ${OBJECTDIR}/_ext/1118306443/csp_rtable_static.o ../third_party/libcsp/src/rtable/csp_rtable_static.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1245785818/csp_rdp.o: ../third_party/libcsp/src/transport/csp_rdp.c  .generated_files/flags/default/9975095dbcf44f13657689a592b2043789f7fa97 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1245785818" 
	@${RM} ${OBJECTDIR}/_ext/1245785818/csp_rdp.o.d 
	@${RM} ${OBJECTDIR}/_ext/1245785818/csp_rdp.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1245785818/csp_rdp.o.d" -o ${OBJECTDIR}/_ext/1245785818/csp_rdp.o ../third_party/libcsp/src/transport/csp_rdp.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1245785818/csp_udp.o: ../third_party/libcsp/src/transport/csp_udp.c  .generated_files/flags/default/e3942b0c5a2a184b6e896915eebc5bb81f1c2a62 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1245785818" 
	@${RM} ${OBJECTDIR}/_ext/1245785818/csp_udp.o.d 
	@${RM} ${OBJECTDIR}/_ext/1245785818/csp_udp.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1245785818/csp_udp.o.d" -o ${OBJECTDIR}/_ext/1245785818/csp_udp.o ../third_party/libcsp/src/transport/csp_udp.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_bridge.o: ../third_party/libcsp/src/csp_bridge.c  .generated_files/flags/default/d2a39834505279a75fa6c150fa4d8b38f5813a66 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_bridge.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_bridge.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_bridge.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_bridge.o ../third_party/libcsp/src/csp_bridge.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_buffer.o: ../third_party/libcsp/src/csp_buffer.c  .generated_files/flags/default/27aeaf0634d5234ba9cfb7d5ac7804fba37d5e5b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_buffer.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_buffer.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_buffer.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_buffer.o ../third_party/libcsp/src/csp_buffer.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_conn.o: ../third_party/libcsp/src/csp_conn.c  .generated_files/flags/default/9cd18cfc8f127676c4d5159dbd853ad052ecf8d4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_conn.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_conn.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_conn.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_conn.o ../third_party/libcsp/src/csp_conn.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_crc32.o: ../third_party/libcsp/src/csp_crc32.c  .generated_files/flags/default/656efaf46dc4449d5804bda6785abf360c86e708 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_crc32.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_crc32.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_crc32.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_crc32.o ../third_party/libcsp/src/csp_crc32.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_debug.o: ../third_party/libcsp/src/csp_debug.c  .generated_files/flags/default/f8ab524055915f4e1b91784c73fb0b37f24d4666 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_debug.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_debug.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_debug.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_debug.o ../third_party/libcsp/src/csp_debug.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_dedup.o: ../third_party/libcsp/src/csp_dedup.c  .generated_files/flags/default/32e739b70f95aca69d38a8956d3db0fd84fcc5b7 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_dedup.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_dedup.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_dedup.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_dedup.o ../third_party/libcsp/src/csp_dedup.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_endian.o: ../third_party/libcsp/src/csp_endian.c  .generated_files/flags/default/561042065237f09ee7a38eb93351ad0a31b80291 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_endian.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_endian.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_endian.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_endian.o ../third_party/libcsp/src/csp_endian.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_hex_dump.o: ../third_party/libcsp/src/csp_hex_dump.c  .generated_files/flags/default/2d470492eb1fdea995ee49b813737426ca399164 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_hex_dump.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_hex_dump.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_hex_dump.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_hex_dump.o ../third_party/libcsp/src/csp_hex_dump.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_iflist.o: ../third_party/libcsp/src/csp_iflist.c  .generated_files/flags/default/9d2c80192cb55dc68601d7fdb4c8abff2412c25a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_iflist.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_iflist.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_iflist.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_iflist.o ../third_party/libcsp/src/csp_iflist.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_init.o: ../third_party/libcsp/src/csp_init.c  .generated_files/flags/default/8804a10031b34abcaf337713696e86e20e1f559b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_init.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_init.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_init.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_init.o ../third_party/libcsp/src/csp_init.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_io.o: ../third_party/libcsp/src/csp_io.c  .generated_files/flags/default/575fb00c1346bad34cfff9c700d6eb30bb81c0fb .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_io.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_io.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_io.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_io.o ../third_party/libcsp/src/csp_io.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_port.o: ../third_party/libcsp/src/csp_port.c  .generated_files/flags/default/1dffb62d3f75dddb2d1670481ae3fe22f6ca2aad .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_port.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_port.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_port.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_port.o ../third_party/libcsp/src/csp_port.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_promisc.o: ../third_party/libcsp/src/csp_promisc.c  .generated_files/flags/default/5576ab2deaedec14b4e3a5a86a357810519f0dfd .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_promisc.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_promisc.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_promisc.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_promisc.o ../third_party/libcsp/src/csp_promisc.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_qfifo.o: ../third_party/libcsp/src/csp_qfifo.c  .generated_files/flags/default/ebfe4b53c6c91215fdf7accba87d05cd9fe2845f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_qfifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_qfifo.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_qfifo.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_qfifo.o ../third_party/libcsp/src/csp_qfifo.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_route.o: ../third_party/libcsp/src/csp_route.c  .generated_files/flags/default/f695eb6565f79855084f5d1233d72acfcd6f5064 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_route.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_route.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_route.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_route.o ../third_party/libcsp/src/csp_route.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_service_handler.o: ../third_party/libcsp/src/csp_service_handler.c  .generated_files/flags/default/dfffabb282bfcf2929f86c43a8ec34c18327175a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_service_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_service_handler.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_service_handler.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_service_handler.o ../third_party/libcsp/src/csp_service_handler.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_services.o: ../third_party/libcsp/src/csp_services.c  .generated_files/flags/default/13918cb7dfac55b3cabe4d6b9dca18dcd54ccef7 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_services.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_services.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_services.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_services.o ../third_party/libcsp/src/csp_services.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1239823104/csp_sfp.o: ../third_party/libcsp/src/csp_sfp.c  .generated_files/flags/default/8e635b33541dd8b39dd4fe6cd84499e931e15ddf .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1239823104" 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_sfp.o.d 
	@${RM} ${OBJECTDIR}/_ext/1239823104/csp_sfp.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1239823104/csp_sfp.o.d" -o ${OBJECTDIR}/_ext/1239823104/csp_sfp.o ../third_party/libcsp/src/csp_sfp.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1360937237/main.o: ../src/main.c  .generated_files/flags/default/796fb5e247aa05f4d3c105dbe6212ffa650fcc22 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1360937237" 
	@${RM} ${OBJECTDIR}/_ext/1360937237/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/main.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1360937237/main.o.d" -o ${OBJECTDIR}/_ext/1360937237/main.o ../src/main.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1360937237/adc_func.o: ../src/adc_func.c  .generated_files/flags/default/609bb509abf1603af155404c362bc816dc8daced .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1360937237" 
	@${RM} ${OBJECTDIR}/_ext/1360937237/adc_func.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/adc_func.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1360937237/adc_func.o.d" -o ${OBJECTDIR}/_ext/1360937237/adc_func.o ../src/adc_func.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1360937237/dbg_task.o: ../src/dbg_task.c  .generated_files/flags/default/ac9361367fcc7bfe55382854f974ae68959e9eaf .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1360937237" 
	@${RM} ${OBJECTDIR}/_ext/1360937237/dbg_task.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/dbg_task.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1360937237/dbg_task.o.d" -o ${OBJECTDIR}/_ext/1360937237/dbg_task.o ../src/dbg_task.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1360937237/opu_task.o: ../src/opu_task.c  .generated_files/flags/default/6dace42fad6c9148d90e8abd821f5174e7400aa9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1360937237" 
	@${RM} ${OBJECTDIR}/_ext/1360937237/opu_task.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/opu_task.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1360937237/opu_task.o.d" -o ${OBJECTDIR}/_ext/1360937237/opu_task.o ../src/opu_task.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1360937237/tc_func.o: ../src/tc_func.c  .generated_files/flags/default/b756d8ccda506d06b2a6ed9315ae7e15970336a5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1360937237" 
	@${RM} ${OBJECTDIR}/_ext/1360937237/tc_func.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/tc_func.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1360937237/tc_func.o.d" -o ${OBJECTDIR}/_ext/1360937237/tc_func.o ../src/tc_func.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
${OBJECTDIR}/_ext/1360937237/pwm_func.o: ../src/pwm_func.c  .generated_files/flags/default/c44b711ff2e68577b485da93a77efd353ce4d21e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1360937237" 
	@${RM} ${OBJECTDIR}/_ext/1360937237/pwm_func.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/pwm_func.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -I"../src" -I"iGRVT50/header" -I"iGRVT50/header/csp" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -MP -MMD -MF "${OBJECTDIR}/_ext/1360937237/pwm_func.o.d" -o ${OBJECTDIR}/_ext/1360937237/pwm_func.o ../src/pwm_func.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS} 
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compileCPP
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${DISTDIR}/sam_ctl.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    ../src/config/default/ATSAMV71Q21B.ld
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE) -g  -D__MPLAB_DEBUGGER_PK5=1 -mprocessor=$(MP_PROCESSOR_OPTION)  -mno-device-startup-code -o ${DISTDIR}/sam_ctl.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}          -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=__ICD2RAM=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D=__DEBUG_D,--defsym=__MPLAB_DEBUGGER_PK5=1,--defsym=_min_heap_size=512,--gc-sections,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--memorysummary,${DISTDIR}/memoryfile.xml -mdfp="${DFP_DIR}/samv71b"
	
else
${DISTDIR}/sam_ctl.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   ../src/config/default/ATSAMV71Q21B.ld
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -mprocessor=$(MP_PROCESSOR_OPTION)  -mno-device-startup-code -o ${DISTDIR}/sam_ctl.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}          -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=_min_heap_size=512,--gc-sections,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--memorysummary,${DISTDIR}/memoryfile.xml -mdfp="${DFP_DIR}/samv71b"
	${MP_CC_DIR}\\xc32-bin2hex ${DISTDIR}/sam_ctl.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} 
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${OBJECTDIR}
	${RM} -r ${DISTDIR}

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(wildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
