#include <csp/sam_csp_protocol.h>

#include <stdbool.h>

enum {
    SAM_CSP_SET_OUTPUTS_LPV_OFFSET = 4U,
    SAM_CSP_SET_OUTPUTS_HPV_OFFSET = 6U,
    SAM_CSP_SET_OUTPUTS_HEATER_OFFSET = 7U,
    SAM_CSP_SET_OUTPUTS_SPARK_OFFSET = 8U,
    SAM_CSP_SET_OUTPUTS_RESERVED_OFFSET = 9U,
    SAM_CSP_SNAPSHOT_SAMPLE_TIME_OFFSET = 6U,
    SAM_CSP_SNAPSHOT_CURRENT_MODE_OFFSET = 10U,
    SAM_CSP_SNAPSHOT_REQUESTED_MODE_OFFSET = 11U,
    SAM_CSP_SNAPSHOT_VALIDITY_OFFSET = 12U,
    SAM_CSP_SNAPSHOT_PT_OFFSET = 14U,
    SAM_CSP_SNAPSHOT_TC_OFFSET = 50U,
    SAM_CSP_HEALTH_UPTIME_OFFSET = 6U,
    SAM_CSP_HEALTH_LINK_STATE_OFFSET = 10U,
    SAM_CSP_HEALTH_LAST_ERROR_OFFSET = 11U,
    SAM_CSP_HEALTH_RESERVED_OFFSET = 12U,
    SAM_CSP_HEALTH_COUNTERS_OFFSET = 14U,
};

static uint16_t get_be16(const uint8_t *input)
{
    return (uint16_t) (((uint16_t) input[0] << 8U) | (uint16_t) input[1]);
}

static void put_be16(uint8_t *output, uint16_t value)
{
    output[0] = (uint8_t) (value >> 8U);
    output[1] = (uint8_t) value;
}

static void put_be32(uint8_t *output, uint32_t value)
{
    output[0] = (uint8_t) (value >> 24U);
    output[1] = (uint8_t) (value >> 16U);
    output[2] = (uint8_t) (value >> 8U);
    output[3] = (uint8_t) value;
}

static void put_be_i32(uint8_t *output, int32_t value)
{
    put_be32(output, (uint32_t) value);
}

static bool has_response_capacity(const uint8_t *output, size_t capacity, size_t length)
{
    return (output != NULL) && (capacity >= length);
}

static void put_response_header(
    uint8_t opcode,
    uint16_t transaction_id,
    sam_csp_status_t status,
    uint8_t detail,
    uint8_t *output)
{
    output[0] = SAM_CSP_PROTOCOL_VERSION;
    output[1] = opcode;
    put_be16(&output[2], transaction_id);
    output[4] = (uint8_t) status;
    output[5] = detail;
}

sam_csp_status_t sam_csp_decode_set_outputs(
    const uint8_t *data,
    size_t length,
    sam_csp_set_outputs_request_t *request,
    uint8_t *detail)
{
    sam_csp_set_outputs_request_t decoded;

    if ((data == NULL) || (request == NULL) || (detail == NULL)) {
        return SAM_CSP_STATUS_INVALID_ARGUMENT;
    }

    if (length < SAM_CSP_REQUEST_HEADER_LENGTH) {
        return SAM_CSP_DECODE_DROP;
    }

    if (data[0] != SAM_CSP_PROTOCOL_VERSION) {
        *detail = 0U;
        return SAM_CSP_STATUS_BAD_VERSION;
    }

    if (data[1] != SAM_CSP_OPCODE_SET_OUTPUTS) {
        *detail = data[1];
        return SAM_CSP_STATUS_BAD_OPCODE;
    }

    if (length != SAM_CSP_SET_OUTPUTS_REQUEST_LENGTH) {
        *detail = SAM_CSP_SET_OUTPUTS_REQUEST_LENGTH;
        return SAM_CSP_STATUS_BAD_LENGTH;
    }

    decoded.transaction_id = get_be16(&data[2]);
    decoded.lpv_on_mask = get_be16(&data[SAM_CSP_SET_OUTPUTS_LPV_OFFSET]);
    decoded.hpv_on_mask = data[SAM_CSP_SET_OUTPUTS_HPV_OFFSET];
    decoded.heater_on_mask = data[SAM_CSP_SET_OUTPUTS_HEATER_OFFSET];
    decoded.spark_on = data[SAM_CSP_SET_OUTPUTS_SPARK_OFFSET];

    if ((decoded.lpv_on_mask & 0xF000U) != 0U) {
        *detail = SAM_CSP_SET_OUTPUTS_LPV_OFFSET;
        return SAM_CSP_STATUS_INVALID_ARGUMENT;
    }

    if ((decoded.heater_on_mask & 0xF0U) != 0U) {
        *detail = SAM_CSP_SET_OUTPUTS_HEATER_OFFSET;
        return SAM_CSP_STATUS_INVALID_ARGUMENT;
    }

    if (decoded.spark_on > 1U) {
        *detail = SAM_CSP_SET_OUTPUTS_SPARK_OFFSET;
        return SAM_CSP_STATUS_INVALID_ARGUMENT;
    }

    if (data[SAM_CSP_SET_OUTPUTS_RESERVED_OFFSET] != 0U) {
        *detail = SAM_CSP_SET_OUTPUTS_RESERVED_OFFSET;
        return SAM_CSP_STATUS_INVALID_ARGUMENT;
    }

    *request = decoded;
    *detail = 0U;
    return SAM_CSP_STATUS_OK;
}

size_t sam_csp_encode_status(
    uint8_t opcode,
    uint16_t transaction_id,
    sam_csp_status_t status,
    uint8_t detail,
    uint8_t *output,
    size_t capacity)
{
    if (!has_response_capacity(
            output,
            capacity,
            SAM_CSP_RESPONSE_HEADER_LENGTH)) {
        return 0U;
    }

    put_response_header(opcode, transaction_id, status, detail, output);
    return SAM_CSP_RESPONSE_HEADER_LENGTH;
}

size_t sam_csp_encode_snapshot(
    uint8_t opcode,
    uint16_t transaction_id,
    const sam_csp_snapshot_t *snapshot,
    uint8_t *output,
    size_t capacity)
{
    if ((snapshot == NULL)
        || !has_response_capacity(
            output,
            capacity,
            SAM_CSP_SNAPSHOT_RESPONSE_LENGTH)) {
        return 0U;
    }

    put_response_header(
        opcode,
        transaction_id,
        SAM_CSP_STATUS_OK,
        0U,
        output);
    put_be32(&output[SAM_CSP_SNAPSHOT_SAMPLE_TIME_OFFSET], snapshot->sample_time_ms);
    output[SAM_CSP_SNAPSHOT_CURRENT_MODE_OFFSET] = snapshot->current_mode;
    output[SAM_CSP_SNAPSHOT_REQUESTED_MODE_OFFSET] = snapshot->requested_mode;
    put_be16(&output[SAM_CSP_SNAPSHOT_VALIDITY_OFFSET], snapshot->validity_mask);

    for (size_t index = 0U; index < SAM_CSP_SNAPSHOT_PT_COUNT; ++index) {
        put_be_i32(
            &output[SAM_CSP_SNAPSHOT_PT_OFFSET + (index * sizeof(int32_t))],
            snapshot->pt_millivolt[index]);
    }

    for (size_t index = 0U; index < SAM_CSP_SNAPSHOT_TC_COUNT; ++index) {
        put_be_i32(
            &output[SAM_CSP_SNAPSHOT_TC_OFFSET + (index * sizeof(int32_t))],
            snapshot->tc_microvolt[index]);
    }

    return SAM_CSP_SNAPSHOT_RESPONSE_LENGTH;
}

size_t sam_csp_encode_health(
    uint8_t opcode,
    uint16_t transaction_id,
    const sam_csp_health_t *health,
    uint8_t *output,
    size_t capacity)
{
    if ((health == NULL)
        || !has_response_capacity(
            output,
            capacity,
            SAM_CSP_HEALTH_RESPONSE_LENGTH)) {
        return 0U;
    }

    put_response_header(
        opcode,
        transaction_id,
        SAM_CSP_STATUS_OK,
        0U,
        output);
    put_be32(&output[SAM_CSP_HEALTH_UPTIME_OFFSET], health->uptime_ms);
    output[SAM_CSP_HEALTH_LINK_STATE_OFFSET] = health->link_state;
    output[SAM_CSP_HEALTH_LAST_ERROR_OFFSET] = health->last_error;
    output[SAM_CSP_HEALTH_RESERVED_OFFSET] = 0U;
    output[SAM_CSP_HEALTH_RESERVED_OFFSET + 1U] = 0U;

    for (size_t index = 0U; index < SAM_CSP_HEALTH_COUNTER_COUNT; ++index) {
        put_be32(
            &output[SAM_CSP_HEALTH_COUNTERS_OFFSET + (index * sizeof(uint32_t))],
            health->counters[index]);
    }

    return SAM_CSP_HEALTH_RESPONSE_LENGTH;
}
