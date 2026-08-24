#include "csp/sam_csp_runtime.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <csp/csp.h>
#include <csp/arch/csp_system.h>
#include <csp/csp_error.h>
#include <csp/csp_rtable.h>
#include <csp_rs485_link.h>

#include "definitions.h"

#include "FreeRTOS.h"
#include "task.h"

#include "csp/sam_csp_config.h"
#include "csp/sam_csp_service.h"
#include "csp/samv71_rs485_port.h"

typedef enum {
    SAM_CSP_INIT_OK = 0,
    SAM_CSP_INIT_ERR_CSP = -1,
    SAM_CSP_INIT_ERR_LINK = -2,
    SAM_CSP_INIT_ERR_ROUTE = -3,
    SAM_CSP_INIT_ERR_ROUTER_TASK = -4,
    SAM_CSP_INIT_ERR_SERVICE_SOCKET = -5,
    SAM_CSP_INIT_ERR_SERVICE_TASK = -6
} sam_csp_init_code_t;

static StaticTask_t s_router_task_storage;
static StackType_t s_router_task_stack[SAM_CSP_ROUTER_TASK_STACK_WORDS];
static StaticTask_t s_service_task_storage;
static StackType_t s_service_task_stack[SAM_CSP_SERVICE_TASK_STACK_WORDS];

static volatile uint8_t s_ready = 0U;
static volatile int32_t s_init_code = SAM_CSP_INIT_OK;
static TaskHandle_t s_router_task = NULL;
static TaskHandle_t s_service_task = NULL;

static int sam_csp_reboot_callback(void)
{
    taskDISABLE_INTERRUPTS();
    NVIC_SystemReset();
    for (;;) {
    }
}

static void router_task(void *argument)
{
    (void)argument;

    for (;;) {
        (void)csp_route_work(CSP_MAX_TIMEOUT);
    }
}

int32_t SamCspRuntime_Init(void)
{
    csp_conf_t conf;
    csp_rs485_link_config_t link_config;
    csp_iface_t *iface;
    int result;

    if (s_ready != 0U) {
        return SAM_CSP_INIT_OK;
    }

    csp_conf_get_defaults(&conf);
    conf.address = SAM_CSP_LOCAL_ADDRESS;
    conf.hostname = "PSC";
    conf.model = "SAM_CTL";
    conf.revision = "PSC_CSP";
    conf.conn_max = SAM_CSP_CONN_MAX;
    conf.conn_queue_length = SAM_CSP_CONN_QUEUE_LENGTH;
    conf.fifo_length = SAM_CSP_FIFO_LENGTH;
    conf.port_max_bind = SAM_CSP_PORT_MAX_BIND;
    conf.buffers = SAM_CSP_BUFFER_COUNT;
    conf.buffer_data_size = SAM_CSP_BUFFER_DATA_SIZE;
    conf.conn_dfl_so = CSP_O_NONE;

    result = csp_init(&conf);
    if (result != CSP_ERR_NONE) {
        s_init_code = SAM_CSP_INIT_ERR_CSP;
        printf("CSP init failed: %ld\r\n", (long)s_init_code);
        return s_init_code;
    }

    csp_sys_set_reboot(sam_csp_reboot_callback);

    link_config.port_ops = Samv71Rs485Port_GetOps();
    link_config.port_context = Samv71Rs485Port_GetContext();
    link_config.tx_margin_ms = SAM_CSP_TX_MARGIN_MS;
    link_config.recovery_retry_ms = SAM_CSP_RECOVERY_RETRY_MS;
    link_config.task_priority = SAM_CSP_RS485_TASK_PRIORITY;
    link_config.task_stack_words = SAM_CSP_RS485_TASK_STACK_WORDS;
    result = csp_rs485_link_init(&link_config);
    if (result != CSP_ERR_NONE) {
        s_init_code = SAM_CSP_INIT_ERR_LINK;
        Samv71Rs485Port_ForceReceiveMode();
        printf("CSP RS485 link failed: %ld\r\n", (long)s_init_code);
        return s_init_code;
    }

    iface = csp_rs485_link_get_interface();
    result = csp_route_set(SAM_CSP_OBC_ADDRESS, iface, CSP_NO_VIA_ADDRESS);
    if (result != CSP_ERR_NONE) {
        s_init_code = SAM_CSP_INIT_ERR_ROUTE;
        csp_rs485_link_deinit();
        Samv71Rs485Port_ForceReceiveMode();
        printf("CSP route failed: %ld\r\n", (long)s_init_code);
        return s_init_code;
    }
    result = csp_route_set(CSP_BROADCAST_ADDR, iface, CSP_NO_VIA_ADDRESS);
    if (result != CSP_ERR_NONE) {
        s_init_code = SAM_CSP_INIT_ERR_ROUTE;
        csp_rs485_link_deinit();
        Samv71Rs485Port_ForceReceiveMode();
        printf("CSP broadcast route failed: %ld\r\n", (long)s_init_code);
        return s_init_code;
    }

    s_router_task = xTaskCreateStatic(
        router_task,
        "csp-router",
        SAM_CSP_ROUTER_TASK_STACK_WORDS,
        NULL,
        SAM_CSP_ROUTER_TASK_PRIORITY,
        s_router_task_stack,
        &s_router_task_storage);
    if (s_router_task == NULL) {
        s_init_code = SAM_CSP_INIT_ERR_ROUTER_TASK;
        csp_rs485_link_deinit();
        Samv71Rs485Port_ForceReceiveMode();
        printf("CSP router task failed: %ld\r\n", (long)s_init_code);
        return s_init_code;
    }

    result = SamCspService_Open();
    if (result != 0) {
        s_init_code = SAM_CSP_INIT_ERR_SERVICE_SOCKET;
        csp_rs485_link_deinit();
        Samv71Rs485Port_ForceReceiveMode();
        printf("CSP service socket failed: %ld\r\n", (long)s_init_code);
        return s_init_code;
    }

    s_service_task = xTaskCreateStatic(
        SamCspService_Task,
        "csp-service",
        SAM_CSP_SERVICE_TASK_STACK_WORDS,
        NULL,
        SAM_CSP_SERVICE_TASK_PRIORITY,
        s_service_task_stack,
        &s_service_task_storage);
    if (s_service_task == NULL) {
        s_init_code = SAM_CSP_INIT_ERR_SERVICE_TASK;
        csp_rs485_link_deinit();
        Samv71Rs485Port_ForceReceiveMode();
        printf("CSP service task failed: %ld\r\n", (long)s_init_code);
        return s_init_code;
    }

    s_init_code = SAM_CSP_INIT_OK;
    s_ready = 1U;
    printf("CSP RS485 responder ready: local=%u obc=%u\r\n",
           (unsigned)SAM_CSP_LOCAL_ADDRESS,
           (unsigned)SAM_CSP_OBC_ADDRESS);
    return SAM_CSP_INIT_OK;
}

uint8_t SamCspRuntime_IsReady(void)
{
    return s_ready;
}

void SamCspRuntime_GetStatus(sam_csp_runtime_status_t *status)
{
    if (status == NULL) {
        return;
    }
    status->ready = s_ready;
    status->init_code = s_init_code;
    status->router_task = s_router_task;
    status->service_task = s_service_task;
}
