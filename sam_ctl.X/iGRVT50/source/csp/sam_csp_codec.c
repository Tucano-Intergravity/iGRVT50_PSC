#include "csp/sam_csp_protocol.h"

#include <stddef.h>
#include <stdint.h>

static uint16_t get_be16(const uint8_t *data)
{
    return (uint16_t)(((uint16_t)data[0] << 8) | (uint16_t)data[1]);
}

static uint32_t get_be32(const uint8_t *data)
{
    return ((uint32_t)data[0] << 24)
        | ((uint32_t)data[1] << 16)
        | ((uint32_t)data[2] << 8)
        | (uint32_t)data[3];
}

static int32_t get_be_i32(const uint8_t *data)
{
    return (int32_t)get_be32(data);
}

static void put_be16(uint8_t *data, uint16_t value)
{
    data[0] = (uint8_t)(value >> 8);
    data[1] = (uint8_t)value;
}

static void put_be32(uint8_t *data, uint32_t value)
{
    data[0] = (uint8_t)(value >> 24);
    data[1] = (uint8_t)(value >> 16);
    data[2] = (uint8_t)(value >> 8);
    data[3] = (uint8_t)value;
}

static void put_be_i32(uint8_t *data, int32_t value)
{
    put_be32(data, (uint32_t)value);
}

sam_csp_status_t SamCsp_DecodeHeader(
    const uint8_t *data,
    size_t length,
    uint8_t expected_opcode,
    size_t expected_length,
    uint16_t *transaction_id,
    uint8_t *detail)
{
    if (detail != NULL) {
        *detail = 0U;
    }
    if (transaction_id != NULL) {
        *transaction_id = 0U;
    }
    if ((data == NULL) || (length < SAM_CSP_COMMON_REQUEST_LENGTH)) {
        return SAM_CSP_STATUS_DROP;
    }
    if (transaction_id != NULL) {
        *transaction_id = get_be16(&data[2]);
    }
    if (data[0] != SAM_CSP_PROTOCOL_VERSION) {
        return SAM_CSP_STATUS_BAD_VERSION;
    }
    if (data[1] != expected_opcode) {
        if (detail != NULL) {
            *detail = data[1];
        }
        return SAM_CSP_STATUS_BAD_OPCODE;
    }
    if (length != expected_length) {
        if (detail != NULL) {
            *detail = (uint8_t)expected_length;
        }
        return SAM_CSP_STATUS_BAD_LENGTH;
    }
    return SAM_CSP_STATUS_OK;
}

sam_csp_status_t SamCsp_DecodeSetOutputs(
    const uint8_t *data,
    size_t length,
    sam_csp_set_outputs_request_t *request,
    uint8_t *detail)
{
    uint16_t transaction_id;
    sam_csp_status_t status = SamCsp_DecodeHeader(
        data,
        length,
        SAM_CSP_OPCODE_SET_OUTPUTS,
        SAM_CSP_SET_OUTPUTS_REQUEST_LENGTH,
        &transaction_id,
        detail);

    if (status != SAM_CSP_STATUS_OK) {
        return status;
    }
    if (request == NULL) {
        if (detail != NULL) {
            *detail = 0U;
        }
        return SAM_CSP_STATUS_INTERNAL_ERROR;
    }

    const uint16_t lpv_mask = get_be16(&data[4]);
    if ((lpv_mask & (uint16_t)~SAM_CSP_LPV_VALID_MASK) != 0U) {
        if (detail != NULL) {
            *detail = 4U;
        }
        return SAM_CSP_STATUS_INVALID_ARGUMENT;
    }
    if ((data[7] & (uint8_t)~SAM_CSP_HEATER_VALID_MASK) != 0U) {
        if (detail != NULL) {
            *detail = 7U;
        }
        return SAM_CSP_STATUS_INVALID_ARGUMENT;
    }
    if (data[8] > 1U) {
        if (detail != NULL) {
            *detail = 8U;
        }
        return SAM_CSP_STATUS_INVALID_ARGUMENT;
    }
    if (data[9] != 0U) {
        if (detail != NULL) {
            *detail = 9U;
        }
        return SAM_CSP_STATUS_INVALID_ARGUMENT;
    }

    request->transaction_id = transaction_id;
    request->lpv_on_mask = lpv_mask;
    request->hpv_on_mask = data[6];
    request->heater_on_mask = data[7];
    request->spark_on = data[8];
    return SAM_CSP_STATUS_OK;
}

sam_csp_status_t SamCsp_DecodeSetLpvOutputs(
    const uint8_t *data,
    size_t length,
    sam_csp_set_lpv_outputs_request_t *request,
    uint8_t *detail)
{
    uint16_t transaction_id;
    sam_csp_status_t status = SamCsp_DecodeHeader(
        data,
        length,
        SAM_CSP_OPCODE_SET_LPV_OUTPUTS,
        SAM_CSP_SET_LPV_OUTPUTS_REQUEST_LENGTH,
        &transaction_id,
        detail);

    if (status != SAM_CSP_STATUS_OK) {
        return status;
    }
    if (request == NULL) {
        if (detail != NULL) {
            *detail = 0U;
        }
        return SAM_CSP_STATUS_INTERNAL_ERROR;
    }

    const uint16_t lpv_mask = get_be16(&data[4]);
    if ((lpv_mask & (uint16_t)~SAM_CSP_LPV_VALID_MASK) != 0U) {
        if (detail != NULL) {
            *detail = 4U;
        }
        return SAM_CSP_STATUS_INVALID_ARGUMENT;
    }

    request->transaction_id = transaction_id;
    request->lpv_on_mask = lpv_mask;
    return SAM_CSP_STATUS_OK;
}

sam_csp_status_t SamCsp_DecodeSetMode(
    const uint8_t *data,
    size_t length,
    sam_csp_set_mode_request_t *request,
    uint8_t *detail)
{
    uint16_t transaction_id;
    sam_csp_status_t status = SamCsp_DecodeHeader(
        data,
        length,
        SAM_CSP_OPCODE_SET_MODE,
        SAM_CSP_SET_MODE_REQUEST_LENGTH,
        &transaction_id,
        detail);
    if (status != SAM_CSP_STATUS_OK) {
        return status;
    }
    if (request == NULL) {
        return SAM_CSP_STATUS_INTERNAL_ERROR;
    }
    if (data[4] > 3U) {
        if (detail != NULL) {
            *detail = 4U;
        }
        return SAM_CSP_STATUS_INVALID_ARGUMENT;
    }
    request->transaction_id = transaction_id;
    request->mode = data[4];
    return SAM_CSP_STATUS_OK;
}

sam_csp_status_t SamCsp_DecodeThrusterStart(
    const uint8_t *data,
    size_t length,
    sam_csp_thruster_start_request_t *request,
    uint8_t *detail)
{
    uint16_t transaction_id;
    sam_csp_status_t status = SamCsp_DecodeHeader(
        data,
        length,
        SAM_CSP_OPCODE_THRUSTER_START,
        SAM_CSP_THRUSTER_START_REQUEST_LENGTH,
        &transaction_id,
        detail);
    if (status != SAM_CSP_STATUS_OK) {
        return status;
    }
    if (request == NULL) {
        return SAM_CSP_STATUS_INTERNAL_ERROR;
    }
    request->transaction_id = transaction_id;
    request->burn_time_ms = get_be32(&data[4]);
    request->sv_o3_open_delay_ms = get_be32(&data[8]);
    request->sv_f3_open_delay_ms = get_be32(&data[12]);
    request->spark_on_delay_ms = get_be32(&data[16]);
    request->spark_on_duration_ms = get_be32(&data[20]);
    request->sv_o3_close_delay_ms = get_be32(&data[24]);
    request->sv_f3_close_delay_ms = get_be32(&data[28]);
    if (request->burn_time_ms == 0U) {
        if (detail != NULL) {
            *detail = 4U;
        }
        return SAM_CSP_STATUS_INVALID_ARGUMENT;
    }
    return SAM_CSP_STATUS_OK;
}

sam_csp_status_t SamCsp_DecodeParStart(
    const uint8_t *data,
    size_t length,
    sam_csp_par_start_request_t *request,
    uint8_t *detail)
{
    uint16_t transaction_id;
    sam_csp_status_t status = SamCsp_DecodeHeader(
        data,
        length,
        SAM_CSP_OPCODE_PAR_START,
        SAM_CSP_PAR_START_REQUEST_LENGTH,
        &transaction_id,
        detail);

    if (status != SAM_CSP_STATUS_OK) {
        return status;
    }
    if (request == NULL) {
        return SAM_CSP_STATUS_INTERNAL_ERROR;
    }
    if ((data[4] & (uint8_t)~SAM_CSP_PAR_SELECTOR_VALID_MASK) != 0U) {
        if (detail != NULL) {
            *detail = 4U;
        }
        return SAM_CSP_STATUS_INVALID_ARGUMENT;
    }

    request->transaction_id = transaction_id;
    request->selector = data[4];
    return SAM_CSP_STATUS_OK;
}

sam_csp_status_t SamCsp_DecodeSetSimSensorValues(
    const uint8_t *data,
    size_t length,
    sam_csp_set_sim_sensor_request_t *request,
    uint8_t *detail)
{
    uint16_t transaction_id;
    size_t offset;
    size_t i;
    sam_csp_status_t status = SamCsp_DecodeHeader(
        data,
        length,
        SAM_CSP_OPCODE_SET_SIM_SENSOR_VALUES,
        SAM_CSP_SET_SIM_SENSOR_REQUEST_LENGTH,
        &transaction_id,
        detail);

    if (status != SAM_CSP_STATUS_OK) {
        return status;
    }
    if (request == NULL) {
        return SAM_CSP_STATUS_INTERNAL_ERROR;
    }

    request->transaction_id = transaction_id;
    offset = SAM_CSP_COMMON_REQUEST_LENGTH;
    for (i = 0U; i < SAM_CSP_PT_CHANNEL_COUNT; i++) {
        request->pt_millibar[i] = get_be_i32(&data[offset]);
        offset += 4U;
    }
    for (i = 0U; i < SAM_CSP_TC_CHANNEL_COUNT; i++) {
        request->tc_millikelvin[i] = get_be_i32(&data[offset]);
        offset += 4U;
    }
    return SAM_CSP_STATUS_OK;
}

size_t SamCsp_EncodeStatus(
    uint8_t opcode,
    uint16_t transaction_id,
    sam_csp_status_t status,
    uint8_t detail,
    uint8_t *output,
    size_t capacity)
{
    if ((output == NULL) || (capacity < SAM_CSP_COMMON_RESPONSE_LENGTH)) {
        return 0U;
    }
    output[0] = SAM_CSP_PROTOCOL_VERSION;
    output[1] = opcode;
    put_be16(&output[2], transaction_id);
    output[4] = (uint8_t)status;
    output[5] = detail;
    return SAM_CSP_COMMON_RESPONSE_LENGTH;
}

size_t SamCsp_EncodeSensorSnapshot(
    uint8_t opcode,
    uint16_t transaction_id,
    const sam_csp_sensor_snapshot_t *snapshot,
    uint8_t *output,
    size_t capacity)
{
    size_t offset;
    size_t i;

    if ((snapshot == NULL) || (output == NULL)
        || (capacity < SAM_CSP_SENSOR_RESPONSE_LENGTH)) {
        return 0U;
    }

    offset = SamCsp_EncodeStatus(
        opcode,
        transaction_id,
        SAM_CSP_STATUS_OK,
        0U,
        output,
        capacity);
    if (offset == 0U) {
        return 0U;
    }

    put_be32(&output[offset], snapshot->sample_time_ms);
    offset += 4U;
    output[offset++] = snapshot->current_mode;
    output[offset++] = snapshot->requested_mode;
    put_be16(&output[offset], snapshot->validity_mask);
    offset += 2U;
    for (i = 0U; i < SAM_CSP_PT_CHANNEL_COUNT; i++) {
        put_be_i32(&output[offset], snapshot->pt_millivolt[i]);
        offset += 4U;
    }
    for (i = 0U; i < SAM_CSP_PT_CHANNEL_COUNT; i++) {
        put_be_i32(&output[offset], snapshot->pt_millibar[i]);
        offset += 4U;
    }
    for (i = 0U; i < SAM_CSP_TC_CHANNEL_COUNT; i++) {
        put_be_i32(&output[offset], snapshot->tc_microvolt[i]);
        offset += 4U;
    }
    for (i = 0U; i < SAM_CSP_TC_CHANNEL_COUNT; i++) {
        put_be_i32(&output[offset], snapshot->tc_millikelvin[i]);
        offset += 4U;
    }
    return offset;
}

size_t SamCsp_EncodeSolvalveSnapshot(
    uint8_t opcode,
    uint16_t transaction_id,
    const sam_csp_solvalve_snapshot_t *snapshot,
    uint8_t *output,
    size_t capacity)
{
    size_t offset;

    if ((snapshot == NULL) || (output == NULL)
        || (capacity < SAM_CSP_SOLVALVE_RESPONSE_LENGTH)) {
        return 0U;
    }
    offset = SamCsp_EncodeStatus(
        opcode,
        transaction_id,
        SAM_CSP_STATUS_OK,
        0U,
        output,
        capacity);
    if (offset == 0U) {
        return 0U;
    }
    put_be32(&output[offset], snapshot->sample_time_ms);
    offset += 4U;
    output[offset++] = snapshot->current_mode;
    put_be16(&output[offset], snapshot->lpv_on_mask);
    offset += 2U;
    output[offset++] = snapshot->hpv_on_mask;
    output[offset++] = snapshot->heater_on_mask;
    output[offset++] = snapshot->spark_on;
    return offset;
}

size_t SamCsp_EncodeHealthSnapshot(
    uint8_t opcode,
    uint16_t transaction_id,
    const sam_csp_health_snapshot_t *snapshot,
    uint8_t *output,
    size_t capacity)
{
    size_t offset;

    if ((snapshot == NULL) || (output == NULL)
        || (capacity < SAM_CSP_HEALTH_RESPONSE_LENGTH)) {
        return 0U;
    }
    offset = SamCsp_EncodeStatus(
        opcode,
        transaction_id,
        SAM_CSP_STATUS_OK,
        0U,
        output,
        capacity);
    if (offset == 0U) {
        return 0U;
    }
    put_be32(&output[offset], snapshot->uptime_ms);
    offset += 4U;
    output[offset++] = snapshot->current_mode;
    output[offset++] = snapshot->link_state;
    output[offset++] = snapshot->last_error;
    put_be32(&output[offset], snapshot->thruster_fault_flags);
    offset += 4U;
    return offset;
}
