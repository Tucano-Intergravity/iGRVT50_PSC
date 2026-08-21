#ifndef SAM_CSP_PROTOCOL_H
#define SAM_CSP_PROTOCOL_H

#include <stddef.h>
#include <stdint.h>

#define SAM_CSP_PROTOCOL_VERSION              1U
#define SAM_CSP_PT_CHANNEL_COUNT              9U
#define SAM_CSP_TC_CHANNEL_COUNT              5U

#define SAM_CSP_COMMON_REQUEST_LENGTH         4U
#define SAM_CSP_COMMON_RESPONSE_LENGTH        6U

#define SAM_CSP_OPCODE_SET_OUTPUTS            0x01U
#define SAM_CSP_OPCODE_SET_MODE               0x02U
#define SAM_CSP_OPCODE_THRUSTER_START         0x03U
#define SAM_CSP_OPCODE_PAR_START              0x04U
#define SAM_CSP_OPCODE_PAR_STOP               0x05U
#define SAM_CSP_OPCODE_SET_LPV_OUTPUTS        0x06U
#define SAM_CSP_OPCODE_SET_SIM_SENSOR_VALUES  0x07U
#define SAM_CSP_OPCODE_SIM_START              0x08U
#define SAM_CSP_OPCODE_SIM_STOP               0x09U
#define SAM_CSP_OPCODE_FAULT_CLEAR            0x0AU

#define SAM_CSP_OPCODE_GET_SENSOR_SNAPSHOT    0x01U
#define SAM_CSP_OPCODE_GET_SOLVALVE_STATE     0x02U
#define SAM_CSP_OPCODE_GET_HEALTH             0x01U

#define SAM_CSP_SET_OUTPUTS_REQUEST_LENGTH    10U
#define SAM_CSP_SET_MODE_REQUEST_LENGTH       5U
#define SAM_CSP_THRUSTER_START_REQUEST_LENGTH 32U
#define SAM_CSP_PAR_START_REQUEST_LENGTH      5U
#define SAM_CSP_PAR_STOP_REQUEST_LENGTH       4U
#define SAM_CSP_SET_LPV_OUTPUTS_REQUEST_LENGTH 6U
#define SAM_CSP_SET_SIM_SENSOR_REQUEST_LENGTH 60U
#define SAM_CSP_SIM_REQUEST_LENGTH            4U
#define SAM_CSP_FAULT_CLEAR_REQUEST_LENGTH    4U
#define SAM_CSP_SENSOR_REQUEST_LENGTH         4U
#define SAM_CSP_SOLVALVE_REQUEST_LENGTH       4U
#define SAM_CSP_HEALTH_REQUEST_LENGTH         4U

#define SAM_CSP_SENSOR_RESPONSE_LENGTH        126U
#define SAM_CSP_SOLVALVE_RESPONSE_LENGTH      16U
#define SAM_CSP_HEALTH_DEBUG_MAX_MESSAGES     4U
#define SAM_CSP_HEALTH_RESPONSE_LENGTH        110U

#define SAM_CSP_LPV_VALID_MASK                0x0FFFU
#define SAM_CSP_HEATER_VALID_MASK             0x0FU
#define SAM_CSP_PAR_OXIDIZER_PT_O4_MASK       0x01U
#define SAM_CSP_PAR_FUEL_PT_F4_MASK           0x02U
#define SAM_CSP_PAR_SELECTOR_VALID_MASK       0x03U

typedef enum {
    SAM_CSP_STATUS_OK = 0,
    SAM_CSP_STATUS_BAD_VERSION = 1,
    SAM_CSP_STATUS_BAD_LENGTH = 2,
    SAM_CSP_STATUS_BAD_OPCODE = 3,
    SAM_CSP_STATUS_INVALID_ARGUMENT = 4,
    SAM_CSP_STATUS_INVALID_STATE = 5,
    SAM_CSP_STATUS_APPLY_FAILED = 6,
    SAM_CSP_STATUS_INTERNAL_ERROR = 7,
    SAM_CSP_STATUS_BUSY = 8,
    SAM_CSP_STATUS_DROP = 255
} sam_csp_status_t;

typedef struct {
    uint16_t transaction_id;
    uint16_t lpv_on_mask;
    uint8_t hpv_on_mask;
    uint8_t heater_on_mask;
    uint8_t spark_on;
} sam_csp_set_outputs_request_t;

typedef struct {
    uint16_t transaction_id;
    uint16_t lpv_on_mask;
} sam_csp_set_lpv_outputs_request_t;

typedef struct {
    uint16_t transaction_id;
    uint8_t mode;
} sam_csp_set_mode_request_t;

typedef struct {
    uint16_t transaction_id;
    uint32_t burn_time_ms;
    uint32_t sv_o3_open_delay_ms;
    uint32_t sv_f3_open_delay_ms;
    uint32_t spark_on_delay_ms;
    uint32_t spark_on_duration_ms;
    uint32_t sv_o3_close_delay_ms;
    uint32_t sv_f3_close_delay_ms;
} sam_csp_thruster_start_request_t;

typedef struct {
    uint16_t transaction_id;
    uint8_t selector;
} sam_csp_par_start_request_t;

typedef struct {
    uint16_t transaction_id;
    int32_t pt_millibar[SAM_CSP_PT_CHANNEL_COUNT];
    int32_t tc_millikelvin[SAM_CSP_TC_CHANNEL_COUNT];
} sam_csp_set_sim_sensor_request_t;

typedef struct {
    uint32_t sample_time_ms;
    uint8_t current_mode;
    uint8_t requested_mode;
    uint16_t validity_mask;
    int32_t pt_millivolt[SAM_CSP_PT_CHANNEL_COUNT];
    int32_t pt_millibar[SAM_CSP_PT_CHANNEL_COUNT];
    int32_t tc_microvolt[SAM_CSP_TC_CHANNEL_COUNT];
    int32_t tc_millikelvin[SAM_CSP_TC_CHANNEL_COUNT];
} sam_csp_sensor_snapshot_t;

typedef struct {
    uint32_t sample_time_ms;
    uint8_t current_mode;
    uint16_t lpv_on_mask;
    uint8_t hpv_on_mask;
    uint8_t heater_on_mask;
    uint8_t spark_on;
} sam_csp_solvalve_snapshot_t;

typedef struct {
    uint32_t sequence;
    uint32_t elapsed_ms;
    uint8_t source;
    uint8_t event;
    uint8_t mode;
    uint8_t reserved;
} sam_csp_debug_message_t;

typedef struct {
    uint32_t uptime_ms;
    uint8_t current_mode;
    uint8_t link_state;
    uint8_t last_error;
    uint8_t debug_count;
    sam_csp_debug_message_t debug_messages[SAM_CSP_HEALTH_DEBUG_MAX_MESSAGES];
    uint32_t counters[11];
    uint32_t thruster_fault_flags;
} sam_csp_health_snapshot_t;

sam_csp_status_t SamCsp_DecodeHeader(
    const uint8_t *data,
    size_t length,
    uint8_t expected_opcode,
    size_t expected_length,
    uint16_t *transaction_id,
    uint8_t *detail);
sam_csp_status_t SamCsp_DecodeSetOutputs(
    const uint8_t *data,
    size_t length,
    sam_csp_set_outputs_request_t *request,
    uint8_t *detail);
sam_csp_status_t SamCsp_DecodeSetLpvOutputs(
    const uint8_t *data,
    size_t length,
    sam_csp_set_lpv_outputs_request_t *request,
    uint8_t *detail);
sam_csp_status_t SamCsp_DecodeSetMode(
    const uint8_t *data,
    size_t length,
    sam_csp_set_mode_request_t *request,
    uint8_t *detail);
sam_csp_status_t SamCsp_DecodeThrusterStart(
    const uint8_t *data,
    size_t length,
    sam_csp_thruster_start_request_t *request,
    uint8_t *detail);
sam_csp_status_t SamCsp_DecodeParStart(
    const uint8_t *data,
    size_t length,
    sam_csp_par_start_request_t *request,
    uint8_t *detail);
sam_csp_status_t SamCsp_DecodeSetSimSensorValues(
    const uint8_t *data,
    size_t length,
    sam_csp_set_sim_sensor_request_t *request,
    uint8_t *detail);

size_t SamCsp_EncodeStatus(
    uint8_t opcode,
    uint16_t transaction_id,
    sam_csp_status_t status,
    uint8_t detail,
    uint8_t *output,
    size_t capacity);
size_t SamCsp_EncodeSensorSnapshot(
    uint8_t opcode,
    uint16_t transaction_id,
    const sam_csp_sensor_snapshot_t *snapshot,
    uint8_t *output,
    size_t capacity);
size_t SamCsp_EncodeSolvalveSnapshot(
    uint8_t opcode,
    uint16_t transaction_id,
    const sam_csp_solvalve_snapshot_t *snapshot,
    uint8_t *output,
    size_t capacity);
size_t SamCsp_EncodeHealthSnapshot(
    uint8_t opcode,
    uint16_t transaction_id,
    const sam_csp_health_snapshot_t *snapshot,
    uint8_t *output,
    size_t capacity);

#endif /* SAM_CSP_PROTOCOL_H */
