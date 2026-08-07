# The following functions contains all the flags passed to the different build stages.

set(PACK_REPO_PATH "C:/Users/USER/.mchp_packs" CACHE PATH "Path to the root of a pack repository.")

function(sam_ctl_default_default_XC32_assemble_rule target)
    set(options
        "-g"
        "${ASSEMBLER_PRE}"
        "-mprocessor=ATSAMV71Q21B"
        "-Wa,--defsym=__MPLAB_BUILD=1${MP_EXTRA_AS_POST},--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1"
        "-g,-I${CMAKE_CURRENT_SOURCE_DIR}/../../../sam_ctl.X"
        "-mdfp=${PACK_REPO_PATH}/Microchip/SAMV71_DFP/4.12.237/samv71b")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target} PRIVATE "__DEBUG=1")
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../sam_ctl.X")
endfunction()
function(sam_ctl_default_default_XC32_assembleWithPreprocess_rule target)
    set(options
        "-x"
        "assembler-with-cpp"
        "-g"
        "${MP_EXTRA_AS_PRE}"
        "${DEBUGGER_NAME_AS_MACRO}"
        "-mdfp=${PACK_REPO_PATH}/Microchip/SAMV71_DFP/4.12.237/samv71b"
        "-mprocessor=ATSAMV71Q21B"
        "-Wa,--defsym=__MPLAB_BUILD=1${MP_EXTRA_AS_POST},--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-I${CMAKE_CURRENT_SOURCE_DIR}/../../../sam_ctl.X")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "__DEBUG"
        PRIVATE "XPRJ_default=default")
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../sam_ctl.X")
endfunction()
function(sam_ctl_default_default_XC32_compile_rule target)
    set(options
        "-g"
        "${CC_PRE}"
        "-x"
        "c"
        "-c"
        "-mprocessor=ATSAMV71Q21B"
        "-ffunction-sections"
        "-fdata-sections"
        "-O1"
        "-mdfp=${PACK_REPO_PATH}/Microchip/SAMV71_DFP/4.12.237/samv71b")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "__DEBUG"
        PRIVATE "XPRJ_default=default")
    target_include_directories(${target}
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../sam_ctl.X/iGRVT50/header"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../sam_ctl.X/iGRVT50/header/csp"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/packs/ATSAMV71Q21B_DFP"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/packs/CMSIS"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/packs/CMSIS/CMSIS/Core/Include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../config/libcsp/samv71/include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/csp-rs485/include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/csp-rs485/src"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../sam_ctl.X"
        PRIVATE "${PACK_REPO_PATH}/ARM/CMSIS/5.4.0/CMSIS/Core/Include")
endfunction()
function(sam_ctl_default_default_XC32_compile_cpp_rule target)
    set(options
        "-g"
        "${CC_PRE}"
        "${DEBUGGER_NAME_AS_MACRO}"
        "-mprocessor=ATSAMV71Q21B"
        "-frtti"
        "-fexceptions"
        "-fno-check-new"
        "-fenforce-eh-specs"
        "-ffunction-sections"
        "-O1"
        "-fno-common"
        "-mdfp=${PACK_REPO_PATH}/Microchip/SAMV71_DFP/4.12.237/samv71b")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "__DEBUG"
        PRIVATE "XPRJ_default=default")
    target_include_directories(${target}
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../sam_ctl.X/iGRVT50/header"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../sam_ctl.X/iGRVT50/header/csp"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/packs/ATSAMV71Q21B_DFP"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/packs/CMSIS"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/packs/CMSIS/CMSIS/Core/Include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../config/libcsp/samv71/include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/libcsp/include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/csp-rs485/include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/csp-rs485/src"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../sam_ctl.X"
        PRIVATE "${PACK_REPO_PATH}/ARM/CMSIS/5.4.0/CMSIS/Core/Include")
endfunction()
function(sam_ctl_default_dependentObject_rule target)
    set(options
        "-mprocessor=ATSAMV71Q21B"
        "-mdfp=${PACK_REPO_PATH}/Microchip/SAMV71_DFP/4.12.237/samv71b")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
endfunction()
function(sam_ctl_default_link_rule target)
    set(options
        "-g"
        "${MP_EXTRA_LD_PRE}"
        "${DEBUGGER_OPTION_TO_LINKER}"
        "${DEBUGGER_NAME_AS_MACRO}"
        "-mprocessor=ATSAMV71Q21B"
        "-mno-device-startup-code"
        "-Wl,--defsym=__MPLAB_BUILD=1${MP_EXTRA_LD_POST},--script=${sam_ctl_default_LINKER_SCRIPT},--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=_min_heap_size=512,--gc-sections,-L${CMAKE_CURRENT_SOURCE_DIR}/../../../sam_ctl.X,-Map=mem.map,--memorysummary,memoryfile.xml"
        "-mdfp=${PACK_REPO_PATH}/Microchip/SAMV71_DFP/4.12.237/samv71b")
    list(REMOVE_ITEM options "")
    target_link_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target} PRIVATE "XPRJ_default=default")
endfunction()
