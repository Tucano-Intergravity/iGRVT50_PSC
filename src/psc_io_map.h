#ifndef PSC_IO_MAP_H
#define PSC_IO_MAP_H

typedef enum PscHpvValve {
    PSC_HPV_SV_O1 = 1U,
    PSC_HPV_SV_O2 = 2U,
    PSC_HPV_SV_O3 = 3U,
    PSC_HPV_SPARE4 = 4U,
    PSC_HPV_SV_F1 = 5U,
    PSC_HPV_SV_F2 = 6U,
    PSC_HPV_SV_F3 = 7U,
    PSC_HPV_SPARE8 = 8U
} ePscHpvValve;

typedef enum PscLpvValve {
    PSC_LPV_SV_R01 = 1U,
    PSC_LPV_SV_R02 = 2U,
    PSC_LPV_SV_R03 = 3U,
    PSC_LPV_SV_R04 = 4U,
    PSC_LPV_SV_R05 = 5U,
    PSC_LPV_SV_R06 = 6U,
    PSC_LPV_SV_R07 = 7U,
    PSC_LPV_SV_R08 = 8U,
    PSC_LPV_SV_R09 = 9U,
    PSC_LPV_SV_R10 = 10U,
    PSC_LPV_SV_R11 = 11U,
    PSC_LPV_SV_R12 = 12U
} ePscLpvValve;

typedef enum PscPressureSensor {
    PSC_PT_O1 = 1U,
    PSC_PT_O2 = 2U,
    PSC_PT_O3 = 3U,
    PSC_PT_O4 = 4U,
    PSC_PT_F1 = 5U,
    PSC_PT_F2 = 6U,
    PSC_PT_F3 = 7U,
    PSC_PT_F4 = 8U,
    PSC_PT_C1 = 9U
} ePscPressureSensor;

#define PSC_IO_INDEX(channel)      ((unsigned int)((channel) - 1U))
#define PSC_IO_MASK(channel)       (1UL << PSC_IO_INDEX(channel))

#endif /* PSC_IO_MAP_H */
