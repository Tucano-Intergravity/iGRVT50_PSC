#ifndef IGRVT50_STATEMACHINE_H
#define IGRVT50_STATEMACHINE_H

#include "sam_ctl.h"

#define STATEMACHINE_EVENT_HZ       100U
#define STATEMACHINE_EVENT_MS       10U

typedef enum StateMachineMode {
    STATE_MACHINE_INIT_MODE = 0U,
    STATE_MACHINE_NORMAL_MODE,
    STATE_MACHINE_RUN_MODE,
    STATE_MACHINE_DIAGNOSTIC_MODE,
    STATE_MACHINE_MODE_COUNT
} eStateMachineMode;

typedef struct StateMachineSnapshot {
    eStateMachineMode currentMode;
    eStateMachineMode previousMode;
    eStateMachineMode requestedMode;
    UInt32 tick100Hz;
    UInt32 modeElapsedTicks;
    UInt32 transitionCount;
} sStateMachineSnapshot;

void StateMachine_Init( void );
void StateMachine_100HzEvent( void );
UInt8 StateMachine_RequestMode( eStateMachineMode mode );
eStateMachineMode StateMachine_GetMode( void );
const char *StateMachine_GetModeName( eStateMachineMode mode );
void StateMachine_GetSnapshot( sStateMachineSnapshot *pSnapshot );

#endif /* IGRVT50_STATEMACHINE_H */
