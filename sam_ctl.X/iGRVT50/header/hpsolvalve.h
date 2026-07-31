#ifndef IGRVT50_HPSOLVALVE_H
#define IGRVT50_HPSOLVALVE_H

#include "sam_ctl.h"

#define HPSOLVALVE_CHANNEL_COUNT    1U
#define HPSOLVALVE_SV1              1U

void HpSolValve_Init( void );
void HpSolValve_Set( UInt8 ch, UInt8 on );
void HpSolValve_Toggle( UInt8 ch );
UInt8 HpSolValve_IsOn( UInt8 ch );

#endif /* IGRVT50_HPSOLVALVE_H */
