#include "globals.h"
#include "sensors.h"

// ============================================================================
// INPUT VOLTAGE DETECTION
// Reads ADC pins to identify the connected supply rail and configure limits.
// ============================================================================
void CheckInput()
{
    isPowerInputOff = false;

    if (analogRead(V28_PIN) >= VOLT_THRESHOLD_28V)
    {
        inputVoltageLevel = 28;
        maxVoltage = 25.0f;
        minVoltage = 1.5f;
    }
    else if (analogRead(V20_PIN) >= VOLT_THRESHOLD_20V)
    {
        inputVoltageLevel = 20;
        maxVoltage = 15.0f;
        minVoltage = 1.5f;
    }
    else if (analogRead(V12_PIN) >= VOLT_THRESHOLD_12V)
    {
        inputVoltageLevel = 12;
        maxVoltage = 10.0f;
        minVoltage = 1.5f;
    }
    else if (analogRead(V5_PIN) >= VOLT_THRESHOLD_5V)
    {
        inputVoltageLevel = 5;
        maxVoltage = 3.0f;
        minVoltage = 0.5f;
    }
    else
    {
        inputVoltageLevel = 0;
        maxVoltage = 0.0f;
        minVoltage = 0.0f;
        isPowerInputOff = true;
    }

    // Clamp target to valid range when supply changes
    if (targetVoltage > maxVoltage)
        targetVoltage = maxVoltage / 2.0f;

    // Treat as no-input if output has already dropped out
    if (currentVoltage <= 0.3f)
        isPowerInputOff = true;

    if (isPowerInputOff)
    {
        voltageIncrement = 0.0f;
    }
    else if (inputVoltageLevel == 5)
    {
        voltageIncrement = 0.2f;
    }
    else
    {
        voltageIncrement = 0.5f;
    }
}

// ============================================================================
// AVERAGED SENSOR READINGS
// Accumulates 2 samples before updating the shared voltage/current values.
// ============================================================================
void ValueUpdate()
{
    static float voltSum  = 0.0f;
    static float ampSum   = 0.0f;
    static int   count    = 0;

    voltSum += VUpdate();
    ampSum  += AUpdate();
    count++;

    if (count >= 2)
    {
        currentVoltage   = voltSum / count;
        currentAmperage  = ampSum  / count;
        voltSum = 0.0f;
        ampSum  = 0.0f;
        count   = 0;
    }
}

float VUpdate()
{
    return ina219.getBusVoltage_V();
}

float AUpdate()
{
    float mA = ina219.getCurrent_mA();
    return (mA < 0.0f) ? 0.0f : mA / 1000.0f;
}
