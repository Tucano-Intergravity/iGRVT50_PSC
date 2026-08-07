# The following variables contains the files used by the different stages of the build process.
set(sam_ctl_default_default_XC32_FILE_TYPE_assemble)
set_source_files_properties(${sam_ctl_default_default_XC32_FILE_TYPE_assemble} PROPERTIES LANGUAGE ASM)

# For assembly files, add "." to the include path for each file so that .include with a relative path works
foreach(source_file ${sam_ctl_default_default_XC32_FILE_TYPE_assemble})
        set_source_files_properties(${source_file} PROPERTIES INCLUDE_DIRECTORIES "$<PATH:NORMAL_PATH,$<PATH:REMOVE_FILENAME,${source_file}>>")
endforeach()

set(sam_ctl_default_default_XC32_FILE_TYPE_assembleWithPreprocess)
set_source_files_properties(${sam_ctl_default_default_XC32_FILE_TYPE_assembleWithPreprocess} PROPERTIES LANGUAGE ASM)

# For assembly files, add "." to the include path for each file so that .include with a relative path works
foreach(source_file ${sam_ctl_default_default_XC32_FILE_TYPE_assembleWithPreprocess})
        set_source_files_properties(${source_file} PROPERTIES INCLUDE_DIRECTORIES "$<PATH:NORMAL_PATH,$<PATH:REMOVE_FILENAME,${source_file}>>")
endforeach()

set(sam_ctl_default_default_XC32_FILE_TYPE_compile
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../sam_ctl.X/iGRVT50/source/csp/sam_csp_codec.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../sam_ctl.X/iGRVT50/source/csp/sam_csp_domain.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../sam_ctl.X/iGRVT50/source/csp/sam_csp_runtime.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../sam_ctl.X/iGRVT50/source/csp/sam_csp_service.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../sam_ctl.X/iGRVT50/source/csp/samv71_rs485_port.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../sam_ctl.X/iGRVT50/source/hpsolvalve.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../sam_ctl.X/iGRVT50/source/lpsolvalve.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../sam_ctl.X/iGRVT50/source/sensor.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../sam_ctl.X/iGRVT50/source/statemachine.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/adc_func.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/exceptions.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/freertos_hooks.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/initialization.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/interrupts.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/libc_syscalls.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/afec/plib_afec0.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/afec/plib_afec1.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/clk/plib_clk.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/efc/plib_efc.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/nvic/plib_nvic.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/pio/plib_pio.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/pwm/plib_pwm0.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/pwm/plib_pwm1.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/spi/spi_master/plib_spi0_master.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/spi/spi_master/plib_spi1_master.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/systick/plib_systick.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/tc/plib_tc0.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/tc/plib_tc1.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/tc/plib_tc3.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/usart/plib_usart0.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/usart/plib_usart1.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/startup_xc32.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/stdio/xc32_monitor.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/dbg_task.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/opu_task.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/pwm_func.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/tc_func.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/FreeRTOS_tasks.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/croutine.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/event_groups.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/list.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7/port.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/portable/MemMang/heap_1.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/queue.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/stream_buffer.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/timers.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/csp-rs485/src/csp_rs485_freertos.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/csp-rs485/src/csp_rs485_kiss.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/csp-rs485/src/csp_rs485_link.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/csp-rs485/src/csp_rs485_supervisor.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/arch/csp_system.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/arch/csp_time.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/arch/freertos/csp_clock.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/arch/freertos/csp_malloc.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/arch/freertos/csp_queue.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/arch/freertos/csp_semaphore.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/arch/freertos/csp_system.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/arch/freertos/csp_thread.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/arch/freertos/csp_time.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/crypto/csp_hmac.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/crypto/csp_sha1.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/crypto/csp_xtea.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/csp_bridge.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/csp_buffer.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/csp_conn.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/csp_crc32.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/csp_debug.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/csp_dedup.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/csp_endian.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/csp_hex_dump.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/csp_iflist.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/csp_init.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/csp_io.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/csp_port.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/csp_promisc.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/csp_qfifo.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/csp_route.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/csp_service_handler.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/csp_services.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/csp_sfp.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/interfaces/csp_if_can.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/interfaces/csp_if_can_pbuf.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/interfaces/csp_if_i2c.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/interfaces/csp_if_kiss.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/interfaces/csp_if_lo.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/interfaces/csp_if_zmqhub.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/rtable/csp_rtable.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/rtable/csp_rtable_static.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/transport/csp_rdp.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/src/transport/csp_udp.c")
set_source_files_properties(${sam_ctl_default_default_XC32_FILE_TYPE_compile} PROPERTIES LANGUAGE C)
set(sam_ctl_default_default_XC32_FILE_TYPE_compile_cpp)
set_source_files_properties(${sam_ctl_default_default_XC32_FILE_TYPE_compile_cpp} PROPERTIES LANGUAGE CXX)
set(sam_ctl_default_default_XC32_FILE_TYPE_link)

# The linker script used for the build.
set(sam_ctl_default_LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/ATSAMV71Q21B.ld")
set(sam_ctl_default_image_name "default.elf")
set(sam_ctl_default_image_base_name "default")

# The output directory of the final image.
set(sam_ctl_default_output_dir "${CMAKE_CURRENT_SOURCE_DIR}/../../../out/sam_ctl")

# The full path to the final image.
set(sam_ctl_default_full_path_to_image ${sam_ctl_default_output_dir}/${sam_ctl_default_image_name})
