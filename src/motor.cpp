#include "globals.h"
#include "motor.h"

// ============================================================================
// PID IMPLEMENTATION
// ============================================================================

void pidReset(PidController* pid)
{
    pid->integralSum = 0.0f;
    pid->prevError   = 0.0f;
    pid->lastTime    = 0;
}

// Returns a value in [-MOTOR_MAX_SPEED, +MOTOR_MAX_SPEED].
// Positive → need more voltage (BACKWARD). Negative → need less (FORWARD).
float pidCompute(PidController* pid, float setpoint, float measured)
{
    unsigned long now = millis();

    // First call — initialise without producing an output
    if (pid->lastTime == 0)
    {
        pid->lastTime  = now;
        pid->prevError = setpoint - measured;
        return 0.0f;
    }

    float dt = (now - pid->lastTime) / 1000.0f;
    pid->lastTime = now;

    // Guard against unreasonably long gaps (e.g. after power-off pause)
    if (dt < 0.001f || dt > 1.0f)
        return 0.0f;

    float error = setpoint - measured;

    // Proportional
    float pTerm = PID_KP * error;

    // Integral with anti-windup via symmetric clamping
    pid->integralSum += error * dt;
    pid->integralSum  = constrain(pid->integralSum, PID_INTEGRAL_MIN, PID_INTEGRAL_MAX);
    float iTerm = PID_KI * pid->integralSum;

    // Derivative on error
    float dTerm = PID_KD * (error - pid->prevError) / dt;
    pid->prevError = error;

    return constrain(pTerm + iTerm + dTerm,
                     -(float)MOTOR_MAX_SPEED, (float)MOTOR_MAX_SPEED);
}

// ============================================================================
// MOTOR STATE — continuous-rotation servo centred at 90°
// ============================================================================
void MotorState(int type, int speed)
{
    switch (type)
    {
    case MOTOR_STATE_FORWARD:   // Decrease output voltage
        myservo.write(MOTOR_CENTER_POSITION - speed);
        break;
    case MOTOR_STATE_BACKWARD:  // Increase output voltage
        myservo.write(MOTOR_CENTER_POSITION + speed);
        break;
    case MOTOR_STATE_STOP:
        myservo.write(MOTOR_CENTER_POSITION);
        break;
    }
}

// ============================================================================
// VOLTAGE ADJUSTMENT — PID + two-zone motor control
//
// Zone 1 — Coarse (|error| > VOLTAGE_TOLERANCE_LARGE):
//   Motor runs continuously.  If stuck for STUCK_TIMEOUT_MS the PID integral
//   builds naturally, but a one-time speed boost also helps.
//
// Zone 2 — Fine  (VOLTAGE_TOLERANCE_SMALL < |error| ≤ VOLTAGE_TOLERANCE_LARGE):
//   Motor pulses (knocks).  Each pulse lasts KNOCK_DURATION_MS, then the
//   motor stops for KNOCK_INTERVAL_MS before the next pulse.  Direction and
//   magnitude come from the PID so the pulses get smaller as error shrinks.
//
// Zone 3 — Settled (|error| ≤ VOLTAGE_TOLERANCE_SMALL):
//   Motor stopped.
//
// Bugs fixed vs. original:
//   • Knock stop was dead code (inner if always true) — replaced with a
//     proper two-state machine (knockActive flag).
//   • stuckStartTime initialised to 0 caused false "stuck" on first coarse
//     entry — now only starts when first entering the coarse zone.
//   • Change-detection used int cast of float target — now uses 0.1 V steps.
// ============================================================================
void VoltADJ()
{
    static PidController  pid             = {0.0f, 0.0f, 0};
    static bool           stuckTimerActive = false;
    static unsigned long  stuckStartTime   = 0;
    static int            stuckSpeedBoost  = 0;
    static bool           knockActive      = false;
    static unsigned long  knockStartTime   = 0;
    static int            lastZone         = 0;   // 1=coarse, 2=fine, 3=settled
    static int            lastTargetTenths = -1;  // target × 10, integer

    // Reset PID and knock state when the target changes (0.1 V precision)
    int targetTenths = (int)(targetVoltage * 10.0f);
    if (targetTenths != lastTargetTenths)
    {
        pidReset(&pid);
        isKnockingEnabled = false;
        stuckTimerActive  = false;
        stuckSpeedBoost   = 0;
        knockActive       = false;
        lastZone          = 0;
        lastTargetTenths  = targetTenths;
    }

    // No input — stop and clear state
    if (isPowerInputOff)
    {
        pidReset(&pid);
        isKnockingEnabled = false;
        stuckTimerActive  = false;
        knockActive       = false;
        MotorState(MOTOR_STATE_STOP, 0);
        return;
    }

    float error    = targetVoltage - currentVoltage;
    float absError = fabsf(error);

    // ── Zone 3: settled ──────────────────────────────────────────────────────
    if (absError <= VOLTAGE_TOLERANCE_SMALL)
    {
        // Bleed integral slowly so it's ready if friction creeps the output
        pid.integralSum = constrain(pid.integralSum, -1.0f, 1.0f);
        isVoltageMatched    = true;
        isVoltageMismatched = false;
        isKnockingEnabled   = false;
        stuckTimerActive    = false;
        stuckSpeedBoost     = 0;
        knockActive         = false;
#if DEBUG_KNOCK
        if (lastZone != 3)
            Serial.printf("[ZONE3]  settled  err=%.3fV  integ=%.2f\n",
                          error, pid.integralSum);
#endif
        lastZone            = 3;
        MotorState(MOTOR_STATE_STOP, 0);
        return;
    }

    float pidOutput  = pidCompute(&pid, targetVoltage, currentVoltage);
    int   motorSpeed = constrain((int)fabsf(pidOutput), MOTOR_MIN_SPEED, MOTOR_MAX_SPEED);

    // ── Zone 2: fine / knocking ───────────────────────────────────────────────
    if (absError <= VOLTAGE_TOLERANCE_LARGE)
    {
        isKnockingEnabled   = true;
        isVoltageMismatched = false;
        isVoltageMatched    = false;
        stuckTimerActive    = false;
        stuckSpeedBoost     = 0;

        // On first entry into the fine zone (from either coarse or settled),
        // skip the idle wait so correction starts immediately.
        if (lastZone != 2)
        {
            knockActive    = false;
            knockStartTime = 0;  // sentinel: triggers immediate first knock
#if DEBUG_KNOCK
            Serial.printf("[ZONE2] entry from zone %d  err=%.3fV  pid=%.1f\n",
                          lastZone, error, pidOutput);
#endif
        }
        lastZone = 2;

        // Scale interval and speed linearly across the fine zone:
        //   near settled boundary  → slow / gentle  (INTERVAL_SLOW, MIN_SPEED)
        //   near coarse boundary   → fast / harder  (INTERVAL_FAST, FINE_MAX_SPEED)
        int errorInt = (int)(absError * 1000.0f);
        int loInt    = (int)(VOLTAGE_TOLERANCE_SMALL * 1000.0f);
        int hiInt    = (int)(VOLTAGE_TOLERANCE_LARGE * 1000.0f);

        unsigned long adaptiveInterval = (unsigned long)map(errorInt, loInt, hiInt,
                                             KNOCK_INTERVAL_SLOW_MS, KNOCK_INTERVAL_FAST_MS);
        int adaptiveSpeed = map(errorInt, loInt, hiInt, MOTOR_MIN_SPEED, KNOCK_FINE_MAX_SPEED);
        adaptiveSpeed = constrain(adaptiveSpeed, MOTOR_MIN_SPEED, KNOCK_FINE_MAX_SPEED);

        if (knockActive)
        {
            // End pulse after KNOCK_DURATION_MS
            if (millis() - knockStartTime >= KNOCK_DURATION_MS)
            {
                knockActive = false;
                MotorState(MOTOR_STATE_STOP, 0);
            }
            // else: keep running at already-set direction
        }
        else
        {
            // Start next pulse once idle interval has elapsed.
            // knockStartTime == 0 means "fire immediately" (fresh zone entry).
            if (knockStartTime == 0 || millis() - knockStartTime >= adaptiveInterval)
            {
                knockActive    = true;
                knockStartTime = millis();
                const char* dir = (pidOutput < 0.0f) ? "FWD" : "BWD";
#if DEBUG_KNOCK
                Serial.printf("[KNOCK]  err=%.3fV  spd=%d  interval=%lums  %s\n",
                              error, adaptiveSpeed, adaptiveInterval, dir);
#endif
                if (pidOutput < 0.0f)
                    MotorState(MOTOR_STATE_FORWARD,  adaptiveSpeed);
                else
                    MotorState(MOTOR_STATE_BACKWARD, adaptiveSpeed);
            }
            else
            {
                MotorState(MOTOR_STATE_STOP, 0);
            }
        }
        return;
    }

    // ── Zone 1: coarse ───────────────────────────────────────────────────────
    isKnockingEnabled   = false;
    isVoltageMismatched = true;
    isVoltageMatched    = false;
    knockActive         = false;
#if DEBUG_KNOCK
    if (lastZone != 1)
        Serial.printf("[ZONE1]  coarse  err=%.3fV  pid=%.1f\n", error, pidOutput);
#endif
    lastZone            = 1;

    // Arm stuck timer on first entry into coarse zone
    if (!stuckTimerActive)
    {
        stuckStartTime   = millis();
        stuckTimerActive = true;
        stuckSpeedBoost  = 0;
    }

    // If still not converging, nudge speed above what PID gives
    if (millis() - stuckStartTime > STUCK_TIMEOUT_MS)
    {
        stuckSpeedBoost  = min(stuckSpeedBoost + 1, STUCK_SPEED_INCREMENT_MAX);
        motorSpeed       = min(motorSpeed + stuckSpeedBoost, MOTOR_MAX_SPEED);
        stuckStartTime   = millis();
    }

    if (pidOutput < 0.0f)
        MotorState(MOTOR_STATE_FORWARD,  motorSpeed);
    else
        MotorState(MOTOR_STATE_BACKWARD, motorSpeed);
}

// ============================================================================
// AMP ADJUSTMENT — placeholder (digital POT not yet implemented)
// ============================================================================
void AmpADJ()
{
}
