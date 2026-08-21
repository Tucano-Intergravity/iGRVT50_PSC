#ifndef SAM_CSP_CONFIG_H
#define SAM_CSP_CONFIG_H

#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

#define SAM_CSP_LOCAL_ADDRESS                 0x10U
#define SAM_CSP_OBC_ADDRESS                   0x0AU

#define SAM_CSP_PORT_COMMAND                  10U
#define SAM_CSP_PORT_TELEMETRY                11U
#define SAM_CSP_PORT_DIAGNOSTICS              12U
#define SAM_CSP_PORT_MAX_BIND                 SAM_CSP_PORT_DIAGNOSTICS

#define SAM_CSP_CONN_MAX                      4U
#define SAM_CSP_CONN_QUEUE_LENGTH             10U
#define SAM_CSP_FIFO_LENGTH                   25U
#define SAM_CSP_BUFFER_COUNT                  20U
#define SAM_CSP_BUFFER_DATA_SIZE              300U

#define SAM_CSP_ROUTER_TASK_PRIORITY          (tskIDLE_PRIORITY + 4U)
#define SAM_CSP_RS485_TASK_PRIORITY           (tskIDLE_PRIORITY + 3U)
#define SAM_CSP_SERVICE_TASK_PRIORITY         (tskIDLE_PRIORITY + 2U)
#define SAM_CSP_ROUTER_TASK_STACK_WORDS       512U
#define SAM_CSP_RS485_TASK_STACK_WORDS        512U
#define SAM_CSP_SERVICE_TASK_STACK_WORDS      512U

#define SAM_CSP_TX_MARGIN_MS                  5U
#define SAM_CSP_RECOVERY_RETRY_MS             25U
#define SAM_CSP_SERVICE_READ_TIMEOUT_MS       100U

#define SAM_CSP_RESPONDER_ONLY                1U

#endif /* SAM_CSP_CONFIG_H */
