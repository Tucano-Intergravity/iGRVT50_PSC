#ifndef SAM_CSP_RUNTIME_H
#define SAM_CSP_RUNTIME_H

#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

typedef struct {
    uint8_t ready;
    int32_t init_code;
    TaskHandle_t router_task;
    TaskHandle_t service_task;
} sam_csp_runtime_status_t;

int32_t SamCspRuntime_Init(void);
uint8_t SamCspRuntime_IsReady(void);
void SamCspRuntime_GetStatus(sam_csp_runtime_status_t *status);

#endif /* SAM_CSP_RUNTIME_H */
