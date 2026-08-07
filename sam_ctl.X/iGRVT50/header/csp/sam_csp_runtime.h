#ifndef SAM_CSP_RUNTIME_H
#define SAM_CSP_RUNTIME_H

#include "FreeRTOS.h"
#include "task.h"

#include <stdbool.h>

typedef enum {
    SAM_CSP_RUNTIME_OK = 0,
    SAM_CSP_RUNTIME_ERR_CSP_INIT = -1,
    SAM_CSP_RUNTIME_ERR_LINK_INIT = -2,
    SAM_CSP_RUNTIME_ERR_INTERFACE = -3,
    SAM_CSP_RUNTIME_ERR_ROUTE = -4,
    SAM_CSP_RUNTIME_ERR_ROUTER_TASK = -5,
    SAM_CSP_RUNTIME_ERR_SERVICE_SOCKET = -6,
    SAM_CSP_RUNTIME_ERR_SERVICE_BIND = -7,
    SAM_CSP_RUNTIME_ERR_SERVICE_LISTEN = -8,
    SAM_CSP_RUNTIME_ERR_SERVICE_TASK = -9,
} sam_csp_runtime_init_code_t;

typedef struct {
    bool ready;
    int init_code;
    TaskHandle_t router_task;
    TaskHandle_t service_task;
} sam_csp_runtime_status_t;

int SamCspRuntime_Init(void);
bool SamCspRuntime_IsReady(void);
int SamCspRuntime_GetInitCode(void);
void SamCspRuntime_GetStatus(sam_csp_runtime_status_t *status);
TaskHandle_t SamCspRuntime_GetRouterTaskHandle(void);
TaskHandle_t SamCspRuntime_GetServiceTaskHandle(void);

#endif
