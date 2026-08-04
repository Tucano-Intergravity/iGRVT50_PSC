#include "statemachine.h"

#define STATEMACHINE_INIT_HOLD_TICKS    1UL

typedef struct StateMachineControl {
    volatile eStateMachineMode currentMode;
    volatile eStateMachineMode previousMode;
    volatile eStateMachineMode requestedMode;
    volatile UInt32 tick100Hz;
    volatile UInt32 modeElapsedTicks;
    volatile UInt32 transitionCount;
    volatile UInt8 initialized;
} sStateMachineControl;

static sStateMachineControl s_stateMachine;

static UInt8 StateMachine_IsValidMode( eStateMachineMode mode )
{
    return (mode < STATE_MACHINE_MODE_COUNT) ? 1U : 0U;
}

static void StateMachine_EnterMode( eStateMachineMode nextMode )
{
    if( StateMachine_IsValidMode( nextMode ) == 0U )
    {
        return;
    }

    s_stateMachine.previousMode = s_stateMachine.currentMode;
    s_stateMachine.currentMode = nextMode;
    s_stateMachine.modeElapsedTicks = 0U;
    s_stateMachine.transitionCount++;

    if( nextMode == STATE_MACHINE_INIT_MODE )
    {
        s_stateMachine.requestedMode = STATE_MACHINE_NORMAL_MODE;
    }
    else
    {
        s_stateMachine.requestedMode = nextMode;
    }
}

static void StateMachine_InitMode( void )
{
}

static void StateMachine_NormalMode( void )
{
}

static void StateMachine_RunMode( void )
{
}

static void StateMachine_DiagnosticMode( void )
{
}

static eStateMachineMode StateMachine_SelectNextMode( void )
{
    eStateMachineMode nextMode = s_stateMachine.currentMode;

    switch( s_stateMachine.currentMode )
    {
        case STATE_MACHINE_INIT_MODE:
            StateMachine_InitMode();
            if( s_stateMachine.modeElapsedTicks >= STATEMACHINE_INIT_HOLD_TICKS )
            {
                nextMode = s_stateMachine.requestedMode;
            }
            break;

        case STATE_MACHINE_NORMAL_MODE:
            StateMachine_NormalMode();
            nextMode = s_stateMachine.requestedMode;
            break;

        case STATE_MACHINE_RUN_MODE:
            StateMachine_RunMode();
            nextMode = s_stateMachine.requestedMode;
            break;

        case STATE_MACHINE_DIAGNOSTIC_MODE:
            StateMachine_DiagnosticMode();
            nextMode = s_stateMachine.requestedMode;
            break;

        default:
            nextMode = STATE_MACHINE_INIT_MODE;
            break;
    }

    if( StateMachine_IsValidMode( nextMode ) == 0U )
    {
        nextMode = STATE_MACHINE_INIT_MODE;
    }

    return nextMode;
}

void StateMachine_Init( void )
{
    s_stateMachine.currentMode = STATE_MACHINE_INIT_MODE;
    s_stateMachine.previousMode = STATE_MACHINE_INIT_MODE;
    s_stateMachine.requestedMode = STATE_MACHINE_NORMAL_MODE;
    s_stateMachine.tick100Hz = 0U;
    s_stateMachine.modeElapsedTicks = 0U;
    s_stateMachine.transitionCount = 0U;
    s_stateMachine.initialized = 1U;
}

void StateMachine_100HzEvent( void )
{
    eStateMachineMode nextMode;

    if( s_stateMachine.initialized == 0U )
    {
        StateMachine_Init();
    }

    s_stateMachine.tick100Hz++;
    s_stateMachine.modeElapsedTicks++;

    nextMode = StateMachine_SelectNextMode();
    if( nextMode != s_stateMachine.currentMode )
    {
        StateMachine_EnterMode( nextMode );
    }
}

UInt8 StateMachine_RequestMode( eStateMachineMode mode )
{
    if( StateMachine_IsValidMode( mode ) == 0U )
    {
        return 0U;
    }

    s_stateMachine.requestedMode = mode;
    return 1U;
}

eStateMachineMode StateMachine_GetMode( void )
{
    return s_stateMachine.currentMode;
}

const char *StateMachine_GetModeName( eStateMachineMode mode )
{
    const char *name;

    switch( mode )
    {
        case STATE_MACHINE_INIT_MODE:
            name = "init_mode";
            break;

        case STATE_MACHINE_NORMAL_MODE:
            name = "normal_mode";
            break;

        case STATE_MACHINE_RUN_MODE:
            name = "run_mode";
            break;

        case STATE_MACHINE_DIAGNOSTIC_MODE:
            name = "diagnostic_mode";
            break;

        default:
            name = "unknown_mode";
            break;
    }

    return name;
}

void StateMachine_GetSnapshot( sStateMachineSnapshot *pSnapshot )
{
    if( pSnapshot == (sStateMachineSnapshot *)0 )
    {
        return;
    }

    pSnapshot->currentMode = s_stateMachine.currentMode;
    pSnapshot->previousMode = s_stateMachine.previousMode;
    pSnapshot->requestedMode = s_stateMachine.requestedMode;
    pSnapshot->tick100Hz = s_stateMachine.tick100Hz;
    pSnapshot->modeElapsedTicks = s_stateMachine.modeElapsedTicks;
    pSnapshot->transitionCount = s_stateMachine.transitionCount;
}
