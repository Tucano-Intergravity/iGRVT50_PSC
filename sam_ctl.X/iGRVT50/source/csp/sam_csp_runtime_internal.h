#ifndef SAM_CSP_RUNTIME_INTERNAL_H
#define SAM_CSP_RUNTIME_INTERNAL_H

#include "FreeRTOS.h"
#include "task.h"

#include <csp/csp.h>

#include <csp_rs485_link.h>

#include <stdint.h>

typedef struct {
    int (*csp_init)(const csp_conf_t *configuration);
    int (*link_init)(const csp_rs485_link_config_t *configuration);
    void (*link_deinit)(void);
    csp_iface_t *(*link_get_interface)(void);
    int (*route_set)(uint8_t destination, csp_iface_t *interface, uint8_t via);
    TaskHandle_t (*task_create_static)(
        TaskFunction_t function,
        const char *name,
        uint32_t stack_words,
        void *argument,
        UBaseType_t priority,
        StackType_t *stack,
        StaticTask_t *task_storage);
    void (*task_delete)(TaskHandle_t task);
    int (*service_prepare)(void);
    void (*link_force_receive)(void);
    void (*report_failure)(int init_code);
} sam_csp_runtime_ops_t;

#ifdef CSP_RS485_HOST_TEST
void sam_csp_runtime_test_bind_ops(const sam_csp_runtime_ops_t *ops);
void sam_csp_runtime_test_reset(void);
#endif

#endif
