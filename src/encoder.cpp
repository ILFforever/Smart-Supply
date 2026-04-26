#include "globals.h"
#include "encoder.h"

// ============================================================================
// ISR — called by hardware interrupt on encoder pin change
// ============================================================================
void IRAM_ATTR readEncoderISR()
{
    rotaryEncoder.readEncoder_ISR();
}

// ============================================================================
// ROTARY ENCODER PROCESSING
// Called every loop() iteration.  Detects rotation direction plus single
// click, double click, and hold button events and sets the shared flags.
// ============================================================================
void rotary_loop()
{
    static int           lastEncoderValue     = 0;
    static unsigned long lastClickTime        = 0;
    static bool          waitingForDoubleClick = false;
    static unsigned long buttonDownTime       = 0;

    // ── Rotation detection ───────────────────────────────────────────────────
    if (rotaryEncoder.encoderChanged())
    {
        int current = rotaryEncoder.readEncoder();
        if (lastEncoderValue > current)
        {
            rotLeft  = true;
            rotRight = false;
        }
        else
        {
            rotLeft  = false;
            rotRight = true;
        }
        lastEncoderValue = current;
    }

    // ── Single vs. double click ──────────────────────────────────────────────
    if (rotaryEncoder.isEncoderButtonClicked())
    {
        if (!rotHold)
        {
            if ((millis() - lastClickTime > 500) && !waitingForDoubleClick)
            {
                lastClickTime        = millis();
                waitingForDoubleClick = true;
            }
            else
            {
                waitingForDoubleClick = false;
                rotDoubleClick = true;
            }
        }
    }
    else
    {
        if (waitingForDoubleClick && (millis() - lastClickTime > SINGLE_CLICK_TIME_MS))
        {
            waitingForDoubleClick = false;
            rotClick = true;
        }
    }

    // ── Hold detection ───────────────────────────────────────────────────────
    if (rotaryEncoder.isEncoderButtonDown())
    {
        if (!rotDown)
            buttonDownTime = millis();
        rotDown = true;
    }
    else
    {
        rotDown  = false;
        rotHold  = false;
    }

    if (rotDown && (millis() - buttonDownTime > HOLD_THRESHOLD_MS))
        rotHold = true;
}
