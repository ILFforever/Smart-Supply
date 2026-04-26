#pragma once
#include "config.h"

// ============================================================================
// PID CONTROLLER
// Discrete-time PID with anti-windup integral clamping.
// Output range: [-MOTOR_MAX_SPEED, +MOTOR_MAX_SPEED]
//   Positive output → increase voltage (MOTOR_STATE_BACKWARD)
//   Negative output → decrease voltage (MOTOR_STATE_FORWARD)
// ============================================================================
struct PidController
{
    float         integralSum;
    float         prevError;
    unsigned long lastTime;   // 0 = uninitialized
};

float pidCompute(PidController* pid, float setpoint, float measured);
void  pidReset(PidController* pid);

// ============================================================================
// MOTOR / VOLTAGE CONTROL
// ============================================================================
void MotorState(int type, int speed);
void VoltADJ();
void AmpADJ();
