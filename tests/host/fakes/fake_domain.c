#include "fake_domain.h"

#include "csp/sam_csp_service_internal.h"

#include <string.h>

static fake_domain_observations_t observations;
static sam_csp_domain_result_t apply_result;
static sam_csp_domain_result_t mode_result;
static sam_csp_domain_result_t snapshot_result;
static sam_csp_domain_result_t current_mode_result;
static sam_csp_snapshot_t snapshot_value;
static uint8_t current_mode_value;
static uint32_t tick_count_value;
static uint8_t link_state_value;
static uint8_t link_last_error_value;
static uint32_t link_counters_value[SAM_CSP_HEALTH_COUNTER_COUNT];

static sam_csp_domain_result_t fake_apply_outputs(
    const sam_csp_set_outputs_request_t *request)
{
    ++observations.apply_outputs_calls;
    if (request != NULL) {
        observations.last_outputs = *request;
    }
    return apply_result;
}

static sam_csp_domain_result_t fake_request_mode(uint8_t mode)
{
    ++observations.request_mode_calls;
    observations.last_mode = mode;
    return mode_result;
}

static sam_csp_domain_result_t fake_get_snapshot(
    sam_csp_snapshot_t *snapshot)
{
    ++observations.get_snapshot_calls;
    if (snapshot != NULL) {
        *snapshot = snapshot_value;
    }
    return snapshot_result;
}

static sam_csp_domain_result_t fake_get_current_mode(uint8_t *mode)
{
    ++observations.get_current_mode_calls;
    if (mode != NULL) {
        *mode = current_mode_value;
    }
    return current_mode_result;
}

static uint32_t fake_get_tick_count(void)
{
    ++observations.tick_count_calls;
    return tick_count_value;
}

static void fake_get_link_health(csp_rs485_health_t *health)
{
    ++observations.link_health_calls;
    if (health == NULL) {
        return;
    }
    memset(health, 0, sizeof(*health));
    health->state = (csp_rs485_link_state_t) link_state_value;
    health->last_error = (csp_rs485_fault_t) link_last_error_value;
    health->uart_errors = link_counters_value[0];
    health->dma_errors = link_counters_value[1];
    health->tx_timeouts = link_counters_value[2];
    health->tx_failures = link_counters_value[3];
    health->protocol_errors = link_counters_value[4];
    health->stream_dropped_bytes = link_counters_value[5];
    health->stream_high_watermark = link_counters_value[6];
    health->stream_discontinuities = link_counters_value[7];
    health->recovery_attempts = link_counters_value[8];
    health->recovery_successes = link_counters_value[9];
    health->recovery_failures = link_counters_value[10];
}

static const sam_csp_service_dependencies_t fake_dependencies = {
    .apply_outputs = fake_apply_outputs,
    .request_mode = fake_request_mode,
    .get_snapshot = fake_get_snapshot,
    .get_current_mode = fake_get_current_mode,
    .get_tick_count = fake_get_tick_count,
    .get_link_health = fake_get_link_health,
};

void fake_domain_reset(void)
{
    memset(&observations, 0, sizeof(observations));
    apply_result = SAM_CSP_DOMAIN_OK;
    mode_result = SAM_CSP_DOMAIN_OK;
    snapshot_result = SAM_CSP_DOMAIN_OK;
    current_mode_result = SAM_CSP_DOMAIN_OK;
    memset(&snapshot_value, 0, sizeof(snapshot_value));
    current_mode_value = 0U;
    tick_count_value = 0U;
    link_state_value = 0U;
    link_last_error_value = 0U;
    memset(link_counters_value, 0, sizeof(link_counters_value));
}

void fake_domain_bind(void)
{
    sam_csp_service_test_bind_dependencies(&fake_dependencies);
}

void fake_domain_get_observations(fake_domain_observations_t *result)
{
    if (result != NULL) {
        *result = observations;
    }
}

void fake_domain_set_apply_result(sam_csp_domain_result_t result)
{
    apply_result = result;
}

void fake_domain_set_mode_result(sam_csp_domain_result_t result)
{
    mode_result = result;
}

void fake_domain_set_snapshot_result(sam_csp_domain_result_t result)
{
    snapshot_result = result;
}

void fake_domain_set_snapshot(const sam_csp_snapshot_t *snapshot)
{
    if (snapshot != NULL) {
        snapshot_value = *snapshot;
    }
}

void fake_domain_set_current_mode(uint8_t mode, sam_csp_domain_result_t result)
{
    current_mode_value = mode;
    current_mode_result = result;
}

void fake_domain_set_tick_count(uint32_t ticks)
{
    tick_count_value = ticks;
}

void fake_domain_set_link_health(
    uint8_t state,
    uint8_t last_error,
    const uint32_t *counters,
    size_t counter_count)
{
    link_state_value = state;
    link_last_error_value = last_error;
    memset(link_counters_value, 0, sizeof(link_counters_value));
    if (counters != NULL) {
        if (counter_count > SAM_CSP_HEALTH_COUNTER_COUNT) {
            counter_count = SAM_CSP_HEALTH_COUNTER_COUNT;
        }
        memcpy(
            link_counters_value,
            counters,
            counter_count * sizeof(link_counters_value[0]));
    }
}
