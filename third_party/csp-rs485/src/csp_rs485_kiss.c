#include "csp_rs485_internal.h"

#include <csp/csp_endian.h>

#include <csp_rs485_profile.h>

#include <stddef.h>
#include <stdint.h>

#define KISS_FEND UINT8_C(0xC0)
#define KISS_FESC UINT8_C(0xDB)
#define KISS_TFEND UINT8_C(0xDC)
#define KISS_TFESC UINT8_C(0xDD)
#define KISS_TNC_DATA UINT8_C(0x00)

static size_t append_escaped_byte(
    uint8_t value,
    uint8_t *output,
    size_t output_position)
{
    if (value == KISS_FEND) {
        output[output_position] = KISS_FESC;
        output[output_position + 1U] = KISS_TFEND;
        return output_position + 2U;
    }

    if (value == KISS_FESC) {
        output[output_position] = KISS_FESC;
        output[output_position + 1U] = KISS_TFESC;
        return output_position + 2U;
    }

    output[output_position] = value;
    return output_position + 1U;
}

csp_rs485_kiss_encode_result_t csp_rs485_kiss_encode(
    csp_id_t id,
    const uint8_t *packet_data,
    size_t packet_data_length,
    uint8_t *output,
    size_t output_capacity,
    size_t *output_length)
{
    if ((output == NULL)
        || (output_length == NULL)
        || ((packet_data == NULL) && (packet_data_length > 0U))
        || (packet_data_length > CSP_RS485_CSP_BUFFER_DATA_SIZE)) {
        return CSP_RS485_KISS_ENCODE_INVALID_ARGUMENT;
    }

    const size_t raw_length =
        CSP_RS485_CSP_HEADER_SIZE + packet_data_length;
    const size_t worst_case_length = 3U + (2U * raw_length);
    if (output_capacity < worst_case_length) {
        return CSP_RS485_KISS_ENCODE_OUTPUT_TOO_SMALL;
    }

    const uint32_t network_id = csp_hton32(id.ext);
    const uint8_t *header = (const uint8_t *) &network_id;
    size_t position = 0U;

    output[position] = KISS_FEND;
    ++position;
    output[position] = KISS_TNC_DATA;
    ++position;

    for (size_t index = 0U; index < CSP_RS485_CSP_HEADER_SIZE; ++index) {
        position = append_escaped_byte(header[index], output, position);
    }

    for (size_t index = 0U; index < packet_data_length; ++index) {
        position =
            append_escaped_byte(packet_data[index], output, position);
    }

    output[position] = KISS_FEND;
    ++position;
    *output_length = position;
    return CSP_RS485_KISS_ENCODE_OK;
}
