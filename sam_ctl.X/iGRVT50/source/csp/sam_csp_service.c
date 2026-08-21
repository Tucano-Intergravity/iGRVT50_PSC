#include "csp/sam_csp_service.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <csp/csp.h>
#include <csp_rs485_link.h>

#include "FreeRTOS.h"
#include "task.h"

#include "csp/sam_csp_config.h"
#include "csp/sam_csp_domain.h"
#include "csp/sam_csp_protocol.h"
#include "sam_ctl.h"
#include "statemachine.h"

static csp_socket_t *s_socket = NULL;
static volatile sam_csp_service_counters_t s_counters;

static uint16_t read_transaction_id(const uint8_t *request)
{
    return (uint16_t)(((uint16_t)request[2] << 8) | (uint16_t)request[3]);
}

static sam_csp_status_t domain_status(
    sam_csp_domain_result_t result,
    uint8_t *detail)
{
    if (detail != NULL) {
        *detail = 0U;
    }
    switch (result) {
        case SAM_CSP_DOMAIN_OK:
            return SAM_CSP_STATUS_OK;
        case SAM_CSP_DOMAIN_INVALID_STATE:
            return SAM_CSP_STATUS_INVALID_STATE;
        case SAM_CSP_DOMAIN_APPLY_FAILED:
            return SAM_CSP_STATUS_APPLY_FAILED;
        case SAM_CSP_DOMAIN_SNAPSHOT_FAILED:
        default:
            if (detail != NULL) {
                *detail = 2U;
            }
            return SAM_CSP_STATUS_INTERNAL_ERROR;
    }
}

static sam_csp_dispatch_action_t encode_status_response(
    const uint8_t *request,
    sam_csp_status_t status,
    uint8_t detail,
    uint8_t *response,
    size_t response_capacity,
    size_t *response_length)
{
    size_t length;

    length = SamCsp_EncodeStatus(
        request[1],
        read_transaction_id(request),
        status,
        detail,
        response,
        response_capacity);
    if (length == 0U) {
        return SAM_CSP_DISPATCH_DROP;
    }
    *response_length = length;
    return SAM_CSP_DISPATCH_RESPOND;
}

static sam_csp_dispatch_action_t encode_sensor_snapshot_response(
    const uint8_t *request,
    uint8_t *response,
    size_t response_capacity,
    size_t *response_length)
{
    sam_csp_sensor_snapshot_t snapshot;
    uint8_t detail = 0U;
    sam_csp_status_t status = domain_status(
        SamCspDomain_GetSensorSnapshot(&snapshot),
        &detail);

    if (status != SAM_CSP_STATUS_OK) {
        return encode_status_response(
            request,
            status,
            detail,
            response,
            response_capacity,
            response_length);
    }

    *response_length = SamCsp_EncodeSensorSnapshot(
        request[1],
        read_transaction_id(request),
        &snapshot,
        response,
        response_capacity);
    return (*response_length != 0U)
        ? SAM_CSP_DISPATCH_RESPOND
        : SAM_CSP_DISPATCH_DROP;
}

static sam_csp_dispatch_action_t handle_command(
    const uint8_t *request,
    size_t request_length,
    uint8_t *response,
    size_t response_capacity,
    size_t *response_length)
{
    uint8_t detail = 0U;
    sam_csp_status_t status;
    sam_csp_domain_result_t domain_result;

    switch (request[1]) {
        case SAM_CSP_OPCODE_SET_OUTPUTS: {
            sam_csp_set_outputs_request_t decoded;
            status = SamCsp_DecodeSetOutputs(
                request,
                request_length,
                &decoded,
                &detail);
            if (status == SAM_CSP_STATUS_OK) {
                domain_result = SamCspDomain_ApplyOutputs(&decoded);
                status = domain_status(domain_result, &detail);
                if (status == SAM_CSP_STATUS_OK) {
                    return encode_sensor_snapshot_response(
                        request,
                        response,
                        response_capacity,
                        response_length);
                }
            }
            return encode_status_response(
                request,
                status,
                detail,
                response,
                response_capacity,
                response_length);
        }

        case SAM_CSP_OPCODE_SET_LPV_OUTPUTS: {
            sam_csp_set_lpv_outputs_request_t decoded;
            status = SamCsp_DecodeSetLpvOutputs(
                request,
                request_length,
                &decoded,
                &detail);
            if (status == SAM_CSP_STATUS_OK) {
                domain_result = SamCspDomain_ApplyLpvOutputs(&decoded);
                status = domain_status(domain_result, &detail);
                if (status == SAM_CSP_STATUS_OK) {
                    return encode_sensor_snapshot_response(
                        request,
                        response,
                        response_capacity,
                        response_length);
                }
            }
            return encode_status_response(
                request,
                status,
                detail,
                response,
                response_capacity,
                response_length);
        }

        case SAM_CSP_OPCODE_SET_SIM_SENSOR_VALUES: {
            sam_csp_set_sim_sensor_request_t decoded;
            status = SamCsp_DecodeSetSimSensorValues(
                request,
                request_length,
                &decoded,
                &detail);
            if (status == SAM_CSP_STATUS_OK) {
                domain_result = SamCspDomain_SetSimSensors(&decoded);
                status = domain_status(domain_result, &detail);
                if (status == SAM_CSP_STATUS_OK) {
                    return encode_sensor_snapshot_response(
                        request,
                        response,
                        response_capacity,
                        response_length);
                }
            }
            return encode_status_response(
                request,
                status,
                detail,
                response,
                response_capacity,
                response_length);
        }

        case SAM_CSP_OPCODE_SIM_START:
        case SAM_CSP_OPCODE_SIM_STOP: {
            uint16_t transaction_id;
            status = SamCsp_DecodeHeader(
                request,
                request_length,
                request[1],
                SAM_CSP_SIM_REQUEST_LENGTH,
                &transaction_id,
                &detail);
            (void)transaction_id;
            if (status == SAM_CSP_STATUS_OK) {
                domain_result = (request[1] == SAM_CSP_OPCODE_SIM_START)
                    ? SamCspDomain_StartSim()
                    : SamCspDomain_StopSim();
                status = domain_status(domain_result, &detail);
            }
            return encode_status_response(
                request,
                status,
                detail,
                response,
                response_capacity,
                response_length);
        }

        case SAM_CSP_OPCODE_SET_MODE: {
            sam_csp_set_mode_request_t decoded;
            status = SamCsp_DecodeSetMode(
                request,
                request_length,
                &decoded,
                &detail);
            if (status == SAM_CSP_STATUS_OK) {
                domain_result = SamCspDomain_RequestMode(decoded.mode);
                status = domain_status(domain_result, &detail);
            }
            return encode_status_response(
                request,
                status,
                detail,
                response,
                response_capacity,
                response_length);
        }

        case SAM_CSP_OPCODE_THRUSTER_START: {
            sam_csp_thruster_start_request_t decoded;
            status = SamCsp_DecodeThrusterStart(
                request,
                request_length,
                &decoded,
                &detail);
            if (status == SAM_CSP_STATUS_OK) {
                domain_result =
                    SamCspDomain_StartThruster(&decoded);
                status = domain_status(domain_result, &detail);
            }
            return encode_status_response(
                request,
                status,
                detail,
                response,
                response_capacity,
                response_length);
        }

        case SAM_CSP_OPCODE_PAR_START: {
            sam_csp_par_start_request_t decoded;
            status = SamCsp_DecodeParStart(
                request,
                request_length,
                &decoded,
                &detail);
            if (status == SAM_CSP_STATUS_OK) {
                domain_result = SamCspDomain_StartPar(&decoded);
                status = domain_status(domain_result, &detail);
            }
            return encode_status_response(
                request,
                status,
                detail,
                response,
                response_capacity,
                response_length);
        }

        case SAM_CSP_OPCODE_PAR_STOP: {
            uint16_t transaction_id;
            status = SamCsp_DecodeHeader(
                request,
                request_length,
                request[1],
                SAM_CSP_PAR_STOP_REQUEST_LENGTH,
                &transaction_id,
                &detail);
            (void)transaction_id;
            if (status == SAM_CSP_STATUS_OK) {
                domain_result = SamCspDomain_StopPar();
                status = domain_status(domain_result, &detail);
            }
            return encode_status_response(
                request,
                status,
                detail,
                response,
                response_capacity,
                response_length);
        }

        case SAM_CSP_OPCODE_FAULT_CLEAR: {
            uint16_t transaction_id;
            status = SamCsp_DecodeHeader(
                request,
                request_length,
                request[1],
                SAM_CSP_FAULT_CLEAR_REQUEST_LENGTH,
                &transaction_id,
                &detail);
            (void)transaction_id;
            if (status == SAM_CSP_STATUS_OK) {
                domain_result = SamCspDomain_ClearFaults();
                status = domain_status(domain_result, &detail);
            }
            return encode_status_response(
                request,
                status,
                detail,
                response,
                response_capacity,
                response_length);
        }

        default:
            return encode_status_response(
                request,
                SAM_CSP_STATUS_BAD_OPCODE,
                request[1],
                response,
                response_capacity,
                response_length);
    }
}

static sam_csp_dispatch_action_t handle_telemetry(
    const uint8_t *request,
    size_t request_length,
    uint8_t *response,
    size_t response_capacity,
    size_t *response_length)
{
    uint16_t transaction_id = 0U;
    uint8_t detail = 0U;
    sam_csp_status_t status;

    if (request[1] == SAM_CSP_OPCODE_GET_SENSOR_SNAPSHOT) {
        sam_csp_sensor_snapshot_t snapshot;
        status = SamCsp_DecodeHeader(
            request,
            request_length,
            SAM_CSP_OPCODE_GET_SENSOR_SNAPSHOT,
            SAM_CSP_SENSOR_REQUEST_LENGTH,
            &transaction_id,
            &detail);
        if (status == SAM_CSP_STATUS_OK) {
            status = domain_status(
                SamCspDomain_GetSensorSnapshot(&snapshot),
                &detail);
            if (status == SAM_CSP_STATUS_OK) {
                *response_length = SamCsp_EncodeSensorSnapshot(
                    request[1],
                    transaction_id,
                    &snapshot,
                    response,
                    response_capacity);
                return (*response_length != 0U)
                    ? SAM_CSP_DISPATCH_RESPOND
                    : SAM_CSP_DISPATCH_DROP;
            }
        }
        return encode_status_response(
            request,
            status,
            detail,
            response,
            response_capacity,
            response_length);
    }

    if (request[1] == SAM_CSP_OPCODE_GET_SOLVALVE_STATE) {
        sam_csp_solvalve_snapshot_t snapshot;
        status = SamCsp_DecodeHeader(
            request,
            request_length,
            SAM_CSP_OPCODE_GET_SOLVALVE_STATE,
            SAM_CSP_SOLVALVE_REQUEST_LENGTH,
            &transaction_id,
            &detail);
        if (status == SAM_CSP_STATUS_OK) {
            status = domain_status(
                SamCspDomain_GetSolvalveSnapshot(&snapshot),
                &detail);
            if (status == SAM_CSP_STATUS_OK) {
                *response_length = SamCsp_EncodeSolvalveSnapshot(
                    request[1],
                    transaction_id,
                    &snapshot,
                    response,
                    response_capacity);
                return (*response_length != 0U)
                    ? SAM_CSP_DISPATCH_RESPOND
                    : SAM_CSP_DISPATCH_DROP;
            }
        }
        return encode_status_response(
            request,
            status,
            detail,
            response,
            response_capacity,
            response_length);
    }

    return encode_status_response(
        request,
        SAM_CSP_STATUS_BAD_OPCODE,
        request[1],
        response,
        response_capacity,
        response_length);
}

static sam_csp_dispatch_action_t handle_health(
    const uint8_t *request,
    size_t request_length,
    uint8_t *response,
    size_t response_capacity,
    size_t *response_length)
{
    csp_rs485_health_t link_health;
    sam_csp_health_snapshot_t snapshot;
    sOpuDebugMessage debug_message;
    uint8_t debug_index;
    uint16_t transaction_id = 0U;
    uint8_t detail = 0U;
    sam_csp_status_t status = SamCsp_DecodeHeader(
        request,
        request_length,
        SAM_CSP_OPCODE_GET_HEALTH,
        SAM_CSP_HEALTH_REQUEST_LENGTH,
        &transaction_id,
        &detail);

    if (status != SAM_CSP_STATUS_OK) {
        return encode_status_response(
            request,
            status,
            detail,
            response,
            response_capacity,
            response_length);
    }

    csp_rs485_link_get_health(&link_health);
    memset(&snapshot, 0, sizeof(snapshot));
    snapshot.uptime_ms = (uint32_t)xTaskGetTickCount();
    snapshot.current_mode = (uint8_t)StateMachine_GetMode();
    snapshot.link_state = (uint8_t)link_health.state;
    snapshot.last_error = (uint8_t)link_health.last_error;
    for (debug_index = 0U; debug_index < SAM_CSP_HEALTH_DEBUG_MAX_MESSAGES; debug_index++) {
        memset(&debug_message, 0, sizeof(debug_message));
        if (OpuDebug_PopMessage(&debug_message) == 0U) {
            break;
        }
        snapshot.debug_messages[debug_index].sequence = (uint32_t)debug_message.sequence;
        snapshot.debug_messages[debug_index].elapsed_ms = (uint32_t)debug_message.elapsedMs;
        snapshot.debug_messages[debug_index].source = (uint8_t)debug_message.source;
        snapshot.debug_messages[debug_index].event = (uint8_t)debug_message.event;
        snapshot.debug_messages[debug_index].mode = (uint8_t)debug_message.mode;
        snapshot.debug_messages[debug_index].reserved = 0U;
        snapshot.debug_count++;
    }
    snapshot.counters[0] = link_health.uart_errors;
    snapshot.counters[1] = link_health.dma_errors;
    snapshot.counters[2] = link_health.tx_timeouts;
    snapshot.counters[3] = link_health.tx_failures;
    snapshot.counters[4] = link_health.protocol_errors;
    snapshot.counters[5] = link_health.stream_dropped_bytes;
    snapshot.counters[6] = link_health.stream_high_watermark;
    snapshot.counters[7] = link_health.stream_discontinuities;
    snapshot.counters[8] = link_health.recovery_attempts;
    snapshot.counters[9] = link_health.recovery_successes;
    snapshot.counters[10] = link_health.recovery_failures;
    snapshot.thruster_fault_flags = (uint32_t)Opu_GetThrusterFaultFlags();

    *response_length = SamCsp_EncodeHealthSnapshot(
        request[1],
        transaction_id,
        &snapshot,
        response,
        response_capacity);
    return (*response_length != 0U)
        ? SAM_CSP_DISPATCH_RESPOND
        : SAM_CSP_DISPATCH_DROP;
}

sam_csp_dispatch_action_t SamCspService_Dispatch(
    uint8_t source_address,
    uint8_t destination_address,
    uint8_t destination_port,
    const uint8_t *request,
    size_t request_length,
    uint8_t *response,
    size_t response_capacity,
    size_t *response_length)
{
    if (response_length != NULL) {
        *response_length = 0U;
    }
    if ((request == NULL) || (response == NULL) || (response_length == NULL)) {
        s_counters.malformed_packets++;
        return SAM_CSP_DISPATCH_DROP;
    }
    if (source_address != SAM_CSP_OBC_ADDRESS) {
        s_counters.rejected_peers++;
        return SAM_CSP_DISPATCH_DROP;
    }
    if (destination_address != SAM_CSP_LOCAL_ADDRESS) {
        s_counters.rejected_peers++;
        return SAM_CSP_DISPATCH_DROP;
    }
    if ((destination_port == CSP_PING) || (destination_port == CSP_REBOOT)) {
        return SAM_CSP_DISPATCH_STANDARD_SERVICE;
    }
    if ((destination_port != SAM_CSP_PORT_COMMAND)
        && (destination_port != SAM_CSP_PORT_TELEMETRY)
        && (destination_port != SAM_CSP_PORT_DIAGNOSTICS)) {
        s_counters.dropped_ports++;
        return SAM_CSP_DISPATCH_DROP;
    }
    if (request_length < SAM_CSP_COMMON_REQUEST_LENGTH) {
        s_counters.malformed_packets++;
        return SAM_CSP_DISPATCH_DROP;
    }
    if (request[0] != SAM_CSP_PROTOCOL_VERSION) {
        return encode_status_response(
            request,
            SAM_CSP_STATUS_BAD_VERSION,
            0U,
            response,
            response_capacity,
            response_length);
    }

    switch (destination_port) {
        case SAM_CSP_PORT_COMMAND:
            return handle_command(
                request,
                request_length,
                response,
                response_capacity,
                response_length);
        case SAM_CSP_PORT_TELEMETRY:
            return handle_telemetry(
                request,
                request_length,
                response,
                response_capacity,
                response_length);
        case SAM_CSP_PORT_DIAGNOSTICS:
            return handle_health(
                request,
                request_length,
                response,
                response_capacity,
                response_length);
    }
    s_counters.dropped_ports++;
    return SAM_CSP_DISPATCH_DROP;
}

int SamCspService_Open(void)
{
    if (s_socket != NULL) {
        return 0;
    }

    s_socket = csp_socket(CSP_SO_NONE);
    if (s_socket == NULL) {
        return -1;
    }
    if (csp_bind(s_socket, CSP_ANY) != CSP_ERR_NONE) {
        s_socket = NULL;
        return -2;
    }
    if (csp_listen(s_socket, SAM_CSP_CONN_QUEUE_LENGTH) != CSP_ERR_NONE) {
        s_socket = NULL;
        return -3;
    }
    return 0;
}

void SamCspService_GetCounters(sam_csp_service_counters_t *counters)
{
    if (counters == NULL) {
        return;
    }
    taskENTER_CRITICAL();
    *counters = s_counters;
    taskEXIT_CRITICAL();
}

void SamCspService_Task(void *argument)
{
    uint8_t response_buffer[SAM_CSP_BUFFER_DATA_SIZE];
    (void)argument;

    for (;;) {
        csp_conn_t *conn = csp_accept(s_socket, CSP_MAX_TIMEOUT);
        if (conn == NULL) {
            continue;
        }

        for (;;) {
            csp_packet_t *packet =
                csp_read(conn, SAM_CSP_SERVICE_READ_TIMEOUT_MS);
            if (packet == NULL) {
                break;
            }

            const uint8_t source = (uint8_t)csp_conn_src(conn);
            const uint8_t destination = (uint8_t)csp_conn_dst(conn);
            const uint8_t dport = (uint8_t)csp_conn_dport(conn);
            size_t response_length = 0U;
            sam_csp_dispatch_action_t action = SamCspService_Dispatch(
                source,
                destination,
                dport,
                packet->data,
                packet->length,
                response_buffer,
                sizeof(response_buffer),
                &response_length);

            if (action == SAM_CSP_DISPATCH_STANDARD_SERVICE) {
                csp_service_handler(conn, packet);
                packet = NULL;
            } else {
                csp_buffer_free(packet);
                packet = NULL;
            }

            if ((action == SAM_CSP_DISPATCH_RESPOND)
                && (response_length > 0U)) {
                csp_packet_t *reply = csp_buffer_get(response_length);
                if (reply == NULL) {
                    s_counters.allocation_failures++;
                    continue;
                }
                memcpy(reply->data, response_buffer, response_length);
                reply->length = (uint16_t)response_length;
                if (!csp_send(conn, reply, 0U)) {
                    csp_buffer_free(reply);
                    s_counters.send_failures++;
                }
            }
        }
        csp_close(conn);
    }
}
