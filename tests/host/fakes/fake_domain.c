#include "fake_domain.h"

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
    /* Bound to the service dependency seam once the Task 9 implementation exists. */
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
