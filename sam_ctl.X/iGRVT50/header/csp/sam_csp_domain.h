#ifndef SAM_CSP_DOMAIN_H
#define SAM_CSP_DOMAIN_H

#include <stdint.h>

#include "csp/sam_csp_protocol.h"

typedef enum {
    SAM_CSP_DOMAIN_OK = 0,
    SAM_CSP_DOMAIN_INVALID_STATE,
    SAM_CSP_DOMAIN_APPLY_FAILED,
    SAM_CSP_DOMAIN_SNAPSHOT_FAILED
} sam_csp_domain_result_t;

sam_csp_domain_result_t SamCspDomain_ApplyOutputs(
    const sam_csp_set_outputs_request_t *request);
sam_csp_domain_result_t SamCspDomain_ApplyLpvOutputs(
    const sam_csp_set_lpv_outputs_request_t *request);
sam_csp_domain_result_t SamCspDomain_RequestMode(uint8_t mode);
sam_csp_domain_result_t SamCspDomain_StartThruster(
    const sam_csp_thruster_start_request_t *request);
sam_csp_domain_result_t SamCspDomain_StartPar(
    const sam_csp_par_start_request_t *request);
sam_csp_domain_result_t SamCspDomain_StopPar(void);
sam_csp_domain_result_t SamCspDomain_StartSim(void);
sam_csp_domain_result_t SamCspDomain_StopSim(void);
sam_csp_domain_result_t SamCspDomain_SetSimSensors(
    const sam_csp_set_sim_sensor_request_t *request);
sam_csp_domain_result_t SamCspDomain_ClearFaults(void);
sam_csp_domain_result_t SamCspDomain_GetSensorSnapshot(
    sam_csp_sensor_snapshot_t *snapshot);
sam_csp_domain_result_t SamCspDomain_GetSolvalveSnapshot(
    sam_csp_solvalve_snapshot_t *snapshot);

void SamCspDomain_RecordLpvState(uint8_t ch, uint8_t on);
void SamCspDomain_RecordHpvState(uint8_t ch, uint8_t on);
void SamCspDomain_RecordHeaterState(uint8_t ch, uint8_t on);
void SamCspDomain_RecordSparkPlugState(uint8_t on);
void SamCspDomain_RecordAllOutputsOff(void);

#endif /* SAM_CSP_DOMAIN_H */
