#
#  There exist several targets which are by default empty and which can be 
#  used for execution of your targets. These targets are usually executed 
#  before and after some main targets. They are: 
#
#     .build-pre:              called before 'build' target
#     .build-post:             called after 'build' target
#     .clean-pre:              called before 'clean' target
#     .clean-post:             called after 'clean' target
#     .clobber-pre:            called before 'clobber' target
#     .clobber-post:           called after 'clobber' target
#     .all-pre:                called before 'all' target
#     .all-post:               called after 'all' target
#     .help-pre:               called before 'help' target
#     .help-post:              called after 'help' target
#
#  Targets beginning with '.' are not intended to be called on their own.
#
#  Main targets can be executed directly, and they are:
#  
#     build                    build a specific configuration
#     clean                    remove built files from a configuration
#     clobber                  remove all built files
#     all                      build all configurations
#     help                     print help mesage
#  
#  Targets .build-impl, .clean-impl, .clobber-impl, .all-impl, and
#  .help-impl are implemented in nbproject/makefile-impl.mk.
#
#  Available make variables:
#
#     CND_BASEDIR                base directory for relative paths
#     CND_DISTDIR                default top distribution directory (build artifacts)
#     CND_BUILDDIR               default top build directory (object files, ...)
#     CONF                       name of current configuration
#     CND_ARTIFACT_DIR_${CONF}   directory of build artifact (current configuration)
#     CND_ARTIFACT_NAME_${CONF}  name of build artifact (current configuration)
#     CND_ARTIFACT_PATH_${CONF}  path to build artifact (current configuration)
#     CND_PACKAGE_DIR_${CONF}    directory of package (current configuration)
#     CND_PACKAGE_NAME_${CONF}   name of package (current configuration)
#     CND_PACKAGE_PATH_${CONF}   path to package (current configuration)
#
# NOCDDL


# Environment 
MKDIR=mkdir
CP=cp
CCADMIN=CCadmin
RANLIB=ranlib

MP_EXTRA_CC_PRE += -I"../config/libcsp/samv71/include"
MP_EXTRA_CC_PRE += -I"../third_party/libcsp/include"
MP_EXTRA_CC_PRE += -I"../third_party/libcsp/src"
MP_EXTRA_CC_PRE += -I"../third_party/csp-rs485/include"
MP_EXTRA_CC_PRE += -I"../third_party/csp-rs485/src"

CND_CONF?=default
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE?=debug
else
IMAGE_TYPE?=production
endif
OBJECTDIR?=build/${CND_CONF}/${IMAGE_TYPE}

SAM_CSP_BASE_OBJECTFILES= \
	${OBJECTDIR}/_ext/1865065685/plib_afec1.o \
	${OBJECTDIR}/_ext/1865065685/plib_afec0.o \
	${OBJECTDIR}/_ext/60165520/plib_clk.o \
	${OBJECTDIR}/_ext/60167248/plib_efc.o \
	${OBJECTDIR}/_ext/1865468468/plib_nvic.o \
	${OBJECTDIR}/_ext/60177924/plib_pio.o \
	${OBJECTDIR}/_ext/60178356/plib_pwm0.o \
	${OBJECTDIR}/_ext/60178356/plib_pwm1.o \
	${OBJECTDIR}/_ext/298189674/plib_spi0_master.o \
	${OBJECTDIR}/_ext/298189674/plib_spi1_master.o \
	${OBJECTDIR}/_ext/1827571544/plib_systick.o \
	${OBJECTDIR}/_ext/829342655/plib_tc1.o \
	${OBJECTDIR}/_ext/829342655/plib_tc0.o \
	${OBJECTDIR}/_ext/829342655/plib_tc3.o \
	${OBJECTDIR}/_ext/2001315827/plib_usart0.o \
	${OBJECTDIR}/_ext/2001315827/plib_usart1.o \
	${OBJECTDIR}/_ext/163028504/xc32_monitor.o \
	${OBJECTDIR}/_ext/1171490990/initialization.o \
	${OBJECTDIR}/_ext/1171490990/interrupts.o \
	${OBJECTDIR}/_ext/1171490990/exceptions.o \
	${OBJECTDIR}/_ext/1171490990/startup_xc32.o \
	${OBJECTDIR}/_ext/1171490990/libc_syscalls.o \
	${OBJECTDIR}/_ext/1171490990/freertos_hooks.o \
	${OBJECTDIR}/_ext/977623654/port.o \
	${OBJECTDIR}/_ext/1665200909/heap_1.o \
	${OBJECTDIR}/_ext/404212886/croutine.o \
	${OBJECTDIR}/_ext/404212886/list.o \
	${OBJECTDIR}/_ext/404212886/queue.o \
	${OBJECTDIR}/_ext/404212886/FreeRTOS_tasks.o \
	${OBJECTDIR}/_ext/404212886/timers.o \
	${OBJECTDIR}/_ext/404212886/event_groups.o \
	${OBJECTDIR}/_ext/404212886/stream_buffer.o \
	${OBJECTDIR}/iGRVT50/source/hpsolvalve.o \
	${OBJECTDIR}/iGRVT50/source/lpsolvalve.o \
	${OBJECTDIR}/iGRVT50/source/sensor.o \
	${OBJECTDIR}/iGRVT50/source/statemachine.o \
	${OBJECTDIR}/_ext/1360937237/main.o \
	${OBJECTDIR}/_ext/1360937237/adc_func.o \
	${OBJECTDIR}/_ext/1360937237/dbg_task.o \
	${OBJECTDIR}/_ext/1360937237/opu_task.o \
	${OBJECTDIR}/_ext/1360937237/tc_func.o \
	${OBJECTDIR}/_ext/1360937237/pwm_func.o

SAM_CSP_OBJECTFILES= \
	${OBJECTDIR}/iGRVT50/source/csp/sam_csp_codec.o \
	${OBJECTDIR}/iGRVT50/source/csp/sam_csp_domain.o \
	${OBJECTDIR}/iGRVT50/source/csp/sam_csp_libc_compat.o \
	${OBJECTDIR}/iGRVT50/source/csp/sam_csp_runtime.o \
	${OBJECTDIR}/iGRVT50/source/csp/sam_csp_service.o \
	${OBJECTDIR}/iGRVT50/source/csp/samv71_rs485_port.o \
	${OBJECTDIR}/_ext/csp/libcsp/src/csp_buffer.o \
	${OBJECTDIR}/_ext/csp/libcsp/src/csp_conn.o \
	${OBJECTDIR}/_ext/csp/libcsp/src/csp_crc32.o \
	${OBJECTDIR}/_ext/csp/libcsp/src/csp_debug.o \
	${OBJECTDIR}/_ext/csp/libcsp/src/csp_endian.o \
	${OBJECTDIR}/_ext/csp/libcsp/src/csp_iflist.o \
	${OBJECTDIR}/_ext/csp/libcsp/src/csp_init.o \
	${OBJECTDIR}/_ext/csp/libcsp/src/csp_io.o \
	${OBJECTDIR}/_ext/csp/libcsp/src/csp_port.o \
	${OBJECTDIR}/_ext/csp/libcsp/src/csp_qfifo.o \
	${OBJECTDIR}/_ext/csp/libcsp/src/csp_route.o \
	${OBJECTDIR}/_ext/csp/libcsp/src/csp_service_handler.o \
	${OBJECTDIR}/_ext/csp/libcsp/src/arch/csp_system.o \
	${OBJECTDIR}/_ext/csp/libcsp/src/transport/csp_udp.o \
	${OBJECTDIR}/_ext/csp/libcsp/src/interfaces/csp_if_kiss.o \
	${OBJECTDIR}/_ext/csp/libcsp/src/interfaces/csp_if_lo.o \
	${OBJECTDIR}/_ext/csp/libcsp/src/arch/csp_time.o \
	${OBJECTDIR}/_ext/csp/libcsp/src/arch/freertos/csp_clock.o \
	${OBJECTDIR}/_ext/csp/libcsp/src/arch/freertos/csp_malloc.o \
	${OBJECTDIR}/_ext/csp/libcsp/src/arch/freertos/csp_queue.o \
	${OBJECTDIR}/_ext/csp/libcsp/src/arch/freertos/csp_semaphore.o \
	${OBJECTDIR}/_ext/csp/libcsp/src/arch/freertos/csp_thread.o \
	${OBJECTDIR}/_ext/csp/libcsp/src/arch/freertos/csp_time.o \
	${OBJECTDIR}/_ext/csp/libcsp/src/rtable/csp_rtable.o \
	${OBJECTDIR}/_ext/csp/libcsp/src/rtable/csp_rtable_static.o \
	${OBJECTDIR}/_ext/csp/csp-rs485/src/csp_rs485_freertos.o \
	${OBJECTDIR}/_ext/csp/csp-rs485/src/csp_rs485_kiss.o \
	${OBJECTDIR}/_ext/csp/csp-rs485/src/csp_rs485_link.o \
	${OBJECTDIR}/_ext/csp/csp-rs485/src/csp_rs485_supervisor.o

override OBJECTFILES=$(SAM_CSP_BASE_OBJECTFILES) $(SAM_CSP_OBJECTFILES)
override OBJECTFILES_QUOTED_IF_SPACED=$(OBJECTFILES)
override POSSIBLE_DEPFILES=$(addsuffix .d,$(OBJECTFILES))

SAM_CSP_DEBUG_CC_FLAGS=
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
SAM_CSP_DEBUG_CC_FLAGS=-D__DEBUG -D__MPLAB_DEBUGGER_PK5=1
endif
SAM_CSP_COMMON_CC_INCLUDES=-I"../src" -I"iGRVT50/header" -I"../config/libcsp/samv71/include" -I"../third_party/libcsp/include" -I"../third_party/libcsp/src" -I"../third_party/csp-rs485/include" -I"../third_party/csp-rs485/src" -I"../src/config/default" -I"../src/packs/ATSAMV71Q21B_DFP" -I"../src/packs/CMSIS/" -I"../src/packs/CMSIS/CMSIS/Core/Include" -I"../src/third_party/rtos/FreeRTOS/Source/include" -I"../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/CM7"
SAM_CSP_COMMON_CC_FLAGS=-g $(SAM_CSP_DEBUG_CC_FLAGS) -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -ffunction-sections -fdata-sections -O1 -fno-common $(SAM_CSP_COMMON_CC_INCLUDES)

${OBJECTDIR}/iGRVT50/source/csp/%.o: iGRVT50/source/csp/%.c
	@${MKDIR} "$(dir $@)"
	@${RM} $@.d
	@${RM} $@
	${MP_CC} $(MP_EXTRA_CC_PRE) $(SAM_CSP_COMMON_CC_FLAGS) -MP -MMD -MF "$@.d" -o $@ $< -DXPRJ_default=$(CND_CONF) $(COMPARISON_BUILD) -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS}

${OBJECTDIR}/_ext/csp/libcsp/src/%.o: ../third_party/libcsp/src/%.c
	@${MKDIR} "$(dir $@)"
	@${RM} $@.d
	@${RM} $@
	${MP_CC} $(MP_EXTRA_CC_PRE) $(SAM_CSP_COMMON_CC_FLAGS) -MP -MMD -MF "$@.d" -o $@ $< -DXPRJ_default=$(CND_CONF) $(COMPARISON_BUILD) -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS}

${OBJECTDIR}/_ext/csp/csp-rs485/src/%.o: ../third_party/csp-rs485/src/%.c
	@${MKDIR} "$(dir $@)"
	@${RM} $@.d
	@${RM} $@
	${MP_CC} $(MP_EXTRA_CC_PRE) $(SAM_CSP_COMMON_CC_FLAGS) -MP -MMD -MF "$@.d" -o $@ $< -DXPRJ_default=$(CND_CONF) $(COMPARISON_BUILD) -mdfp="${DFP_DIR}/samv71b" ${PACK_COMMON_OPTIONS}


# build
build: .build-post

.build-pre:
# Add your pre 'build' code here...

.build-post: .build-impl
# Add your post 'build' code here...


# clean
clean: .clean-post

.clean-pre:
# Add your pre 'clean' code here...
# WARNING: the IDE does not call this target since it takes a long time to
# simply run make. Instead, the IDE removes the configuration directories
# under build and dist directly without calling make.
# This target is left here so people can do a clean when running a clean
# outside the IDE.

.clean-post: .clean-impl
# Add your post 'clean' code here...


# clobber
clobber: .clobber-post

.clobber-pre:
# Add your pre 'clobber' code here...

.clobber-post: .clobber-impl
# Add your post 'clobber' code here...


# all
all: .all-post

.all-pre:
# Add your pre 'all' code here...

.all-post: .all-impl
# Add your post 'all' code here...


# help
help: .help-post

.help-pre:
# Add your pre 'help' code here...

.help-post: .help-impl
# Add your post 'help' code here...



# include project implementation makefile
include nbproject/Makefile-impl.mk

# include project make variables
include nbproject/Makefile-variables.mk
