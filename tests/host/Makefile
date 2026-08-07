# Imported and adapted from C:\PSC\csp-rs485\tests\host\Makefile.
# This import makes no assertion about upstream licensing.
REPO_ROOT := $(abspath ../..)
LIBCSP_DIR := $(REPO_ROOT)/third_party/libcsp
CSP_RS485_DIR := $(REPO_ROOT)/third_party/csp-rs485
BUILD_SUFFIX := $(if $(filter 1,$(SANITIZE)),-sanitize,)
override BUILD_DIR := /tmp/csp-rs485-host-tests$(BUILD_SUFFIX)
TEST_BINARY := $(BUILD_DIR)/csp-rs485-host-tests

CC ?= cc

COMMON_CPPFLAGS := \
	-I$(REPO_ROOT)/tests/host/fakes/domain/include \
	-I$(REPO_ROOT)/tests/host/fakes/samv71 \
	-I$(REPO_ROOT)/sam_ctl.X/iGRVT50/source \
	-DCSP_RS485_HOST_TEST \
	-I$(REPO_ROOT)/tests/host/fakes/freertos/include \
	-I$(REPO_ROOT)/tests/host/config/include \
	-I$(REPO_ROOT)/sam_ctl.X/iGRVT50/header \
	-I$(REPO_ROOT)/src \
	-I$(CSP_RS485_DIR)/include \
	-I$(CSP_RS485_DIR)/src \
	-isystem $(LIBCSP_DIR)/include

PROJECT_CFLAGS := \
	-std=c11 \
	-g \
	-O1 \
	-Wall \
	-Wextra \
	-Werror \
	-Wconversion \
	-Wshadow \
	-Wstrict-prototypes

THIRD_PARTY_CFLAGS := \
	-std=gnu11 \
	-g \
	-O1 \
	-Wall \
	-Wextra \
	-Wshadow \
	-Wno-unused-parameter

LDLIBS := -pthread -lrt -lutil
LDFLAGS += \
	-Wl,--wrap=csp_qfifo_write \
	-Wl,--wrap=csp_rs485_freertos_rx_from_isr \
	-Wl,--wrap=csp_rs485_freertos_mark_rx_discontinuity_from_isr \
	-Wl,--wrap=csp_rs485_link_report_fault_from_isr

ifeq ($(SANITIZE),1)
SANITIZER_FLAGS := -fsanitize=address,undefined -fno-omit-frame-pointer
PROJECT_CFLAGS += $(SANITIZER_FLAGS)
THIRD_PARTY_CFLAGS += $(SANITIZER_FLAGS)
LDFLAGS += $(SANITIZER_FLAGS)
LEAK_SANITIZE ?= 0
TEST_ENV := \
	ASAN_OPTIONS=detect_leaks=$(LEAK_SANITIZE):halt_on_error=1 \
	UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1
endif

PROJECT_SOURCES := \
	third_party/csp-rs485/src/csp_rs485_kiss.c \
	third_party/csp-rs485/src/csp_rs485_link.c \
	third_party/csp-rs485/src/csp_rs485_supervisor.c \
	third_party/csp-rs485/src/csp_rs485_freertos.c \
	tests/host/fakes/fake_port.c \
	tests/host/fakes/fake_domain.c \
	tests/host/fakes/domain/fake_sam_csp_domain.c \
	tests/host/fakes/fake_opu_tc_scan.c \
	tests/host/fakes/samv71/fake_samv71_hw.c \
	tests/host/fakes/freertos/fake_freertos.c \
	tests/host/test_main.c \
	tests/host/test_host_profile.c \
	tests/host/test_kiss_encoder.c \
	tests/host/test_link_rx.c \
	tests/host/test_link_tx.c \
	tests/host/test_supervisor.c \
	tests/host/test_freertos_runtime.c \
	tests/host/test_sam_csp_codec.c \
	tests/host/test_sam_csp_domain.c \
	tests/host/test_sensor_publication.c \
	tests/host/test_opu_tc_scan.c \
	tests/host/test_samv71_rs485_port.c \
	tests/host/test_sam_csp_service.c \
	tests/host/test_sam_csp_runtime.c \
	tests/host/support/test_sensor_under_test.c \
	tests/host/support/host_csp.c

SAM_CSP_CODEC_SOURCE := sam_ctl.X/iGRVT50/source/csp/sam_csp_codec.c
ifneq ($(wildcard $(REPO_ROOT)/$(SAM_CSP_CODEC_SOURCE)),)
COMMON_CPPFLAGS += -DSAM_CSP_CODEC_HAVE_IMPLEMENTATION=1
PROJECT_SOURCES += $(SAM_CSP_CODEC_SOURCE)
endif

SAM_CSP_DOMAIN_SOURCE := sam_ctl.X/iGRVT50/source/csp/sam_csp_domain.c
ifneq ($(wildcard $(REPO_ROOT)/$(SAM_CSP_DOMAIN_SOURCE)),)
COMMON_CPPFLAGS += -DSAM_CSP_DOMAIN_HAVE_IMPLEMENTATION=1
PROJECT_SOURCES += $(SAM_CSP_DOMAIN_SOURCE)
endif

SAMV71_RS485_PORT_SOURCE := sam_ctl.X/iGRVT50/source/csp/samv71_rs485_port.c
ifneq ($(wildcard $(REPO_ROOT)/$(SAMV71_RS485_PORT_SOURCE)),)
PROJECT_SOURCES += $(SAMV71_RS485_PORT_SOURCE)
endif

SAM_CSP_SERVICE_SOURCE := sam_ctl.X/iGRVT50/source/csp/sam_csp_service.c
ifneq ($(wildcard $(REPO_ROOT)/$(SAM_CSP_SERVICE_SOURCE)),)
COMMON_CPPFLAGS += -DSAM_CSP_SERVICE_HAVE_IMPLEMENTATION=1
PROJECT_SOURCES += $(SAM_CSP_SERVICE_SOURCE)
endif

SAM_CSP_RUNTIME_SOURCE := sam_ctl.X/iGRVT50/source/csp/sam_csp_runtime.c
ifneq ($(wildcard $(REPO_ROOT)/$(SAM_CSP_RUNTIME_SOURCE)),)
COMMON_CPPFLAGS += -DSAM_CSP_RUNTIME_HAVE_IMPLEMENTATION=1
PROJECT_SOURCES += $(SAM_CSP_RUNTIME_SOURCE)
endif

LIBCSP_SOURCES := \
	$(sort $(wildcard $(LIBCSP_DIR)/src/*.c)) \
	$(sort $(wildcard $(LIBCSP_DIR)/src/transport/*.c)) \
	$(sort $(wildcard $(LIBCSP_DIR)/src/crypto/*.c)) \
	$(sort $(wildcard $(LIBCSP_DIR)/src/interfaces/*.c)) \
	$(sort $(wildcard $(LIBCSP_DIR)/src/arch/*.c)) \
	$(sort $(wildcard $(LIBCSP_DIR)/src/arch/posix/*.c)) \
	$(LIBCSP_DIR)/src/rtable/csp_rtable.c \
	$(LIBCSP_DIR)/src/rtable/csp_rtable_static.c

PROJECT_OBJECTS := $(patsubst %.c,$(BUILD_DIR)/project/%.o,$(PROJECT_SOURCES))
LIBCSP_OBJECTS := $(patsubst $(LIBCSP_DIR)/%.c,$(BUILD_DIR)/libcsp/%.o,$(LIBCSP_SOURCES))

.PHONY: all clean print-sources test

all: $(TEST_BINARY)

test: $(TEST_BINARY)
	@$(TEST_ENV) $(TEST_BINARY) $(TEST)

print-sources:
	@printf '%s\n' $(LIBCSP_SOURCES)

clean:
	@rm -rf /tmp/csp-rs485-host-tests /tmp/csp-rs485-host-tests-sanitize

$(TEST_BINARY): $(PROJECT_OBJECTS) $(LIBCSP_OBJECTS)
	@mkdir -p $(dir $@)
	@$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD_DIR)/project/%.o: $(REPO_ROOT)/%.c
	@mkdir -p $(dir $@)
	@$(CC) $(COMMON_CPPFLAGS) $(PROJECT_CFLAGS) -c $< -o $@

$(BUILD_DIR)/libcsp/%.o: $(LIBCSP_DIR)/%.c
	@mkdir -p $(dir $@)
	@$(CC) $(COMMON_CPPFLAGS) $(THIRD_PARTY_CFLAGS) -c $< -o $@
