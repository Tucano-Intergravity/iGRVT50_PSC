#include <csp/sam_csp_runtime.h>

#include "sam_csp_runtime_internal.h"

#include "FreeRTOS.h"
#include "task.h"

#include <csp/csp.h>
#include <csp/sam_csp_config.h>
#include <csp/sam_csp_service.h>
#include <csp/samv71_rs485_port.h>

#include <csp_rs485_link.h>

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef CSP_RS485_HOST_TEST
#include "definitions.h"
#include <stdio.h>
#endif

static StackType_t router_stack[SAM_CSP_ROUTER_TASK_STACK_WORDS];
static StaticTask_t router_task_storage;
static StackType_t service_stack[SAM_CSP_SERVICE_TASK_STACK_WORDS];
static StaticTask_t service_task_storage;

static _Atomic bool initialization_attempted;
static _Atomic bool runtime_ready;
static _Atomic int runtime_init_code = SAM_CSP_RUNTIME_ERR_CSP_INIT;
static TaskHandle_t router_task_handle;
static TaskHandle_t service_task_handle;

static void router_task(void *argument)
{
    (void) argument;
    for (;;) {
        (void) csp_route_work(CSP_MAX_TIMEOUT);
    }
}

static void default_force_receive(void)
{
    samv71_rs485_port_ops.force_receive_mode(
        &samv71_rs485_port_context);
}

static void default_report_failure(int init_code)
{
#ifndef CSP_RS485_HOST_TEST
    char message[40];
    const int length = snprintf(
        message,
        sizeof(message),
        "CSP init failed: %d\r\n",
        init_code);
    if (length > 0) {
        size_t write_length = (size_t) length;
        if (write_length > sizeof(message)) {
            write_length = sizeof(message);
        }
        (void) USART0_Write(message, write_length);
    }
#else
    (void) init_code;
#endif
}

static const sam_csp_runtime_ops_t default_runtime_ops = {
    .csp_init = csp_init,
    .link_init = csp_rs485_link_init,
    .link_deinit = csp_rs485_link_deinit,
    .link_get_interface = csp_rs485_link_get_interface,
    .route_set = csp_route_set,
    .task_create_static = xTaskCreateStatic,
    .task_delete = vTaskDelete,
    .service_prepare = sam_csp_service_prepare,
    .link_force_receive = default_force_receive,
    .report_failure = default_report_failure,
};

static const sam_csp_runtime_ops_t *runtime_ops = &default_runtime_ops;

static int fail_initialization(int code, bool link_established)
{
    atomic_store_explicit(&runtime_ready, false, memory_order_release);
    atomic_store_explicit(&runtime_init_code, code, memory_order_release);
    if (service_task_handle != NULL) {
        runtime_ops->task_delete(service_task_handle);
        service_task_handle = NULL;
    }
    if (router_task_handle != NULL) {
        runtime_ops->task_delete(router_task_handle);
        router_task_handle = NULL;
    }
    if (link_established) {
        runtime_ops->link_deinit();
    }
    runtime_ops->link_force_receive();
    runtime_ops->report_failure(code);
    return code;
}

int SamCspRuntime_Init(void)
{
    bool expected = false;
    if (!atomic_compare_exchange_strong_explicit(
            &initialization_attempted,
            &expected,
            true,
            memory_order_acq_rel,
            memory_order_acquire)) {
        return atomic_load_explicit(
            &runtime_init_code, memory_order_acquire);
    }

    csp_conf_t configuration;
    csp_conf_get_defaults(&configuration);
    configuration.address = SAM_CSP_LOCAL_ADDRESS;
    configuration.conn_max = SAM_CSP_CONN_MAX;
    configuration.conn_queue_length = SAM_CSP_CONN_QUEUE_LENGTH;
    configuration.fifo_length = SAM_CSP_FIFO_LENGTH;
    configuration.port_max_bind = SAM_CSP_MAX_BIND_PORT;
    configuration.buffers = SAM_CSP_BUFFER_COUNT;
    configuration.buffer_data_size = SAM_CSP_BUFFER_DATA_SIZE;
    configuration.conn_dfl_so = CSP_O_NONE;

    if (runtime_ops->csp_init(&configuration) != CSP_ERR_NONE) {
        return fail_initialization(SAM_CSP_RUNTIME_ERR_CSP_INIT, false);
    }

    const csp_rs485_link_config_t link_configuration = {
        .port_ops = &samv71_rs485_port_ops,
        .port_context = &samv71_rs485_port_context,
        .tx_margin_ms = SAM_CSP_TX_MARGIN_MS,
        .recovery_retry_ms = SAM_CSP_RECOVERY_RETRY_MS,
        .task_priority = SAM_CSP_LINK_TASK_PRIORITY,
        .task_stack_words = SAM_CSP_LINK_TASK_STACK_WORDS,
    };
    if (runtime_ops->link_init(&link_configuration) != CSP_ERR_NONE) {
        return fail_initialization(SAM_CSP_RUNTIME_ERR_LINK_INIT, false);
    }

    csp_iface_t *const interface = runtime_ops->link_get_interface();
    if (interface == NULL) {
        return fail_initialization(SAM_CSP_RUNTIME_ERR_INTERFACE, true);
    }
    if (runtime_ops->route_set(
            SAM_CSP_PEER_ADDRESS,
            interface,
            CSP_NO_VIA_ADDRESS) != CSP_ERR_NONE) {
        return fail_initialization(SAM_CSP_RUNTIME_ERR_ROUTE, true);
    }

    router_task_handle = runtime_ops->task_create_static(
        router_task,
        "csp-router",
        SAM_CSP_ROUTER_TASK_STACK_WORDS,
        NULL,
        SAM_CSP_ROUTER_TASK_PRIORITY,
        router_stack,
        &router_task_storage);
    if (router_task_handle == NULL) {
        return fail_initialization(SAM_CSP_RUNTIME_ERR_ROUTER_TASK, true);
    }

    const int service_result = runtime_ops->service_prepare();
    if (service_result != CSP_ERR_NONE) {
        int code = SAM_CSP_RUNTIME_ERR_SERVICE_SOCKET;
        if ((service_result == SAM_CSP_RUNTIME_ERR_SERVICE_BIND)
            || (service_result == SAM_CSP_RUNTIME_ERR_SERVICE_LISTEN)) {
            code = service_result;
        }
        return fail_initialization(code, true);
    }

    service_task_handle = runtime_ops->task_create_static(
        sam_csp_service_task,
        "csp-service",
        SAM_CSP_SERVICE_TASK_STACK_WORDS,
        NULL,
        SAM_CSP_SERVICE_TASK_PRIORITY,
        service_stack,
        &service_task_storage);
    if (service_task_handle == NULL) {
        return fail_initialization(SAM_CSP_RUNTIME_ERR_SERVICE_TASK, true);
    }

    atomic_store_explicit(
        &runtime_init_code, SAM_CSP_RUNTIME_OK, memory_order_release);
    atomic_store_explicit(&runtime_ready, true, memory_order_release);
    return SAM_CSP_RUNTIME_OK;
}

bool SamCspRuntime_IsReady(void)
{
    return atomic_load_explicit(&runtime_ready, memory_order_acquire);
}

int SamCspRuntime_GetInitCode(void)
{
    return atomic_load_explicit(&runtime_init_code, memory_order_acquire);
}

void SamCspRuntime_GetStatus(sam_csp_runtime_status_t *status)
{
    if (status == NULL) {
        return;
    }
    status->ready = SamCspRuntime_IsReady();
    status->init_code = SamCspRuntime_GetInitCode();
    status->router_task = router_task_handle;
    status->service_task = service_task_handle;
}

TaskHandle_t SamCspRuntime_GetRouterTaskHandle(void)
{
    return router_task_handle;
}

TaskHandle_t SamCspRuntime_GetServiceTaskHandle(void)
{
    return service_task_handle;
}

#ifdef CSP_RS485_HOST_TEST
void sam_csp_runtime_test_bind_ops(const sam_csp_runtime_ops_t *ops)
{
    runtime_ops = (ops != NULL) ? ops : &default_runtime_ops;
}

void sam_csp_runtime_test_reset(void)
{
    atomic_store_explicit(
        &initialization_attempted, false, memory_order_relaxed);
    atomic_store_explicit(&runtime_ready, false, memory_order_relaxed);
    atomic_store_explicit(
        &runtime_init_code,
        SAM_CSP_RUNTIME_ERR_CSP_INIT,
        memory_order_relaxed);
    router_task_handle = NULL;
    service_task_handle = NULL;
    runtime_ops = &default_runtime_ops;
}
#endif
