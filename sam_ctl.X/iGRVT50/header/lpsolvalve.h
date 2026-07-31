#ifndef IGRVT50_LPSOLVALVE_H
#define IGRVT50_LPSOLVALVE_H

#include "sam_ctl.h"

#define LPSOLVALVE_CHANNEL_COUNT    1U
#define LPSOLVALVE_SV1              1U

void LpSolValve_Init( void );
void LpSolValve_Set( UInt8 ch, UInt8 on );
void LpSolValve_SetDuty( UInt8 ch, UInt8 dutyPct );
void LpSolValve_Toggle( UInt8 ch );
UInt8 LpSolValve_IsOn( UInt8 ch );
UInt8 LpSolValve_GetDuty( UInt8 ch );

#endif /* IGRVT50_LPSOLVALVE_H */
