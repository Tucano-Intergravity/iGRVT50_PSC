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
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/adc_func.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/exceptions.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/freertos_hooks.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/initialization.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/interrupts.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/libc_syscalls.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/afec/plib_afec1.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/clk/plib_clk.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/efc/plib_efc.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/nvic/plib_nvic.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/pio/plib_pio.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/pwm/plib_pwm0.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/spi/spi_master/plib_spi0_master.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/systick/plib_systick.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/uart/plib_uart1.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/usart/plib_usart0.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/startup_xc32.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/stdio/xc32_monitor.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/dbg_task.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/opu_task.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/pwm_func.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/rs422_func.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/tc_func.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/FreeRTOS_tasks.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/croutine.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/event_groups.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/list.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7/port.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/portable/MemMang/heap_1.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/queue.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/stream_buffer.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/timers.c")
set_source_files_properties(${sam_ctl_default_default_XC32_FILE_TYPE_compile} PROPERTIES LANGUAGE C)
set(sam_ctl_default_default_XC32_FILE_TYPE_compile_cpp)
set_source_files_properties(${sam_ctl_default_default_XC32_FILE_TYPE_compile_cpp} PROPERTIES LANGUAGE CXX)
set(sam_ctl_default_default_XC32_FILE_TYPE_link)
set(sam_ctl_default_default_XC32_FILE_TYPE_bin2hex)

# The linker script used for the build.
set(sam_ctl_default_LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/ATSAMV71Q21B.ld")
set(sam_ctl_default_image_name "default.elf")
set(sam_ctl_default_image_base_name "default")

# The output directory of the final image.
set(sam_ctl_default_output_dir "${CMAKE_CURRENT_SOURCE_DIR}/../../../out/sam_ctl")

# The full path to the final image.
set(sam_ctl_default_full_path_to_image ${sam_ctl_default_output_dir}/${sam_ctl_default_image_name})
