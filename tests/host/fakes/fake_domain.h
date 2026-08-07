#ifndef TEST_FAKE_DOMAIN_H
#define TEST_FAKE_DOMAIN_H

#include <csp/sam_csp_domain.h>

#include <stddef.h>
#include <stdint.h>

typedef struct {
    size_t apply_outputs_calls;
    size_t request_mode_calls;
    size_t get_snapshot_calls;
    size_t get_current_mode_calls;
    size_t tick_count_calls;
    size_t link_health_calls;
    sam_csp_set_outputs_request_t last_outputs;
    uint8_t last_mode;
} fake_domain_observations_t;

void fake_domain_reset(void);
void fake_domain_bind(void);
void fake_domain_get_observations(fake_domain_observations_t *observations);
void fake_domain_set_apply_result(sam_csp_domain_result_t result);
void fake_domain_set_mode_result(sam_csp_domain_result_t result);
void fake_domain_set_snapshot_result(sam_csp_domain_result_t result);
void fake_domain_set_snapshot(const sam_csp_snapshot_t *snapshot);
void fake_domain_set_current_mode(uint8_t mode, sam_csp_domain_result_t result);
void fake_domain_set_tick_count(uint32_t ticks);
void fake_domain_set_link_health(
    uint8_t state,
    uint8_t last_error,
    const uint32_t *counters,
    size_t counter_count);

#endif
