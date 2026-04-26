#include "globals.h"
#include "display.h"

#include "fonts/NotoSansBold15.h"
#include "fonts/NotoSansBold36.h"
#include "fonts/bigFont.h"
#include "fonts/middleFont.h"
#include "fonts/smallFont.h"
#include "fonts/fatFont.h"
#include "fonts/hugeFatFont.h"

// ============================================================================
// DISPLAY ROUTING
// ============================================================================
void displayUpdate()
{
    if      (currentScreenIndex == SCREEN_MAIN)       Screen1();
    else if (currentScreenIndex == SCREEN_VOLT_GRAPH) Screen2();
    else if (currentScreenIndex == SCREEN_AMP_GRAPH)  Screen3();

    BotBar();
    RotAdjScr();

    // Double-click toggles power output
    if (rotDoubleClick)
    {
        rotDoubleClick       = false;
        isPowerOutputEnabled = !isPowerOutputEnabled;
    }

    // Single click cycles screens (only when encoder is idle)
    if (rotClick && !(rotLeft || rotRight || rotHold))
    {
        currentScreenIndex = (currentScreenIndex < 2) ? currentScreenIndex + 1 : 0;
        rotClick = false;
    }

    txtSprite.pushSprite(0, 0);
}

// ============================================================================
// SCREEN 1 — MAIN READOUT
// ============================================================================
void Screen1()
{
    txtSprite.setFreeFont(&Orbitron_Light_32);
    txtSprite.setTextDatum(0);
    sprite.fillSprite(TFT_BLACK);
    txtSprite.fillSprite(TFT_BLACK);

    // Border boxes
    txtSprite.fillRoundRect(100, 10,  125, 80, 8, TFT_WHITE);
    txtSprite.fillRoundRect( 10, 10,   80, 80, 8, TFT_WHITE);
    txtSprite.fillRoundRect(100, 100, 125, 80, 8, TFT_WHITE);
    txtSprite.fillRoundRect( 10, 100,  80, 80, 8, TFT_WHITE);

    // Left box footers (grey)
    txtSprite.fillRoundRect(10, 70, 80, 20, 8, TFT_DARKGREY);
    txtSprite.fillRect     (10, 70, 80, 10,    TFT_DARKGREY);
    txtSprite.fillRoundRect(10, 160, 80, 20, 8, TFT_DARKGREY);
    txtSprite.fillRect     (10, 160, 80, 10,    TFT_DARKGREY);

    // Right top footer colour indicates regulation status
    uint16_t statusColor;
    if      (isVoltageMismatched) statusColor = TFT_RED;
    else if (isKnockingEnabled)   statusColor = TFT_YELLOW;
    else if (isVoltageMatched)    statusColor = TFT_GREEN;
    else                          statusColor = TFT_YELLOW;

    txtSprite.fillRoundRect(100, 70, 125, 20, 8, statusColor);
    txtSprite.fillRect     (100, 70, 125, 10,    statusColor);

    // Bottom right footer (always green)
    txtSprite.fillRoundRect(100, 160, 125, 20, 8, TFT_GREEN);
    txtSprite.fillRect     (100, 160, 125, 10,    TFT_GREEN);

    // Labels
    txtSprite.setTextColor(TFT_BLACK);
    txtSprite.setTextSize(1);
    txtSprite.drawCentreString("Input-(V)", 52, 72, 2);
    if (isPowerInputOff)
        txtSprite.drawCentreString("No Input Detected", 160, 72, 2);
    else
        txtSprite.drawCentreString("Output-(V)", 160, 72, 2);
    txtSprite.drawCentreString("Watt-(W)",    52,  162, 2);
    txtSprite.drawCentreString("Amp Current", 160, 162, 2);

    // Voltage display — alternates between measured and target every ~1 s
    static unsigned long lastToggle = 0;
    txtSprite.loadFont(hugeFatFont);
    txtSprite.setTextDatum(4);

    if (millis() - lastToggle > 1000)
    {
        voltageDisplayTimer = (voltageDisplayTimer < 10) ? voltageDisplayTimer + 1 : 0;
        lastToggle = millis();
    }

    if (voltageDisplayTimer < 9)
    {
        txtSprite.setTextColor(TFT_BLACK);
        txtSprite.drawCentreString(String(currentVoltage), 160, 25, 1);
    }
    else
    {
        txtSprite.setTextColor(TFT_GREENYELLOW);
        txtSprite.drawCentreString(String(targetVoltage), 160, 25, 1);
    }

    txtSprite.setTextColor(TFT_BLACK);
    txtSprite.drawCentreString(String(currentVoltage * currentAmperage, 1), 50, 115, 1);

    if (!isPowerInputOff)
        txtSprite.drawCentreString(String(inputVoltageLevel), 50, 25, 1);
    else
        txtSprite.drawCentreString("--", 50, 25, 1);

    txtSprite.drawCentreString(String(currentAmperage), 160, 115, 1);
    txtSprite.unloadFont();
}

// ============================================================================
// GRAPH HELPERS
// ============================================================================
void drawGraphGrid(int bars, int sideShift, int topShift)
{
    int spaceX = ((230 - sideShift) - (5 + sideShift)) / bars;
    int spaceY = ((200 - topShift)  - (5 + topShift))  / bars;

    for (int i = 1; i < bars; i++)
    {
        txtSprite.drawLine(5 + spaceX * i + sideShift, 5 + topShift,
                           5 + spaceX * i + sideShift, 200 - topShift, TFT_SILVER);
        txtSprite.drawLine(5 + sideShift, 5 + topShift + spaceY * i,
                           230 - sideShift, 5 + topShift + spaceY * i, TFT_SILVER);
    }

    txtSprite.fillRect(5, 5, 225, 27, TFT_DARKGREY);

    txtSprite.drawWideLine(5  + sideShift, 5  + topShift, 230 - sideShift, 5  + topShift, 2, TFT_DARKGREEN);
    txtSprite.drawWideLine(5  + sideShift, 30 + topShift, 230 - sideShift, 30 + topShift, 2, TFT_DARKGREEN);
    txtSprite.drawWideLine(5  + sideShift, 5  + topShift, 5   + sideShift, 200 - topShift, 2, TFT_DARKGREEN);
    txtSprite.drawWideLine(230 - sideShift, 5 + topShift, 230 - sideShift, 200 - topShift, 2, TFT_DARKGREEN);
    txtSprite.drawWideLine(5  + sideShift, 200 - topShift, 230 - sideShift, 200 - topShift, 2, TFT_DARKGREEN);
}

void drawGraphLine(const int* graphBuffer, int sideShift, int topShift, int calibration)
{
    uint16_t lineColor;
    if      (isVoltageMismatched) lineColor = TFT_DARKGREEN;
    else if (isKnockingEnabled)   lineColor = TFT_CYAN;
    else                          lineColor = TFT_WHITE;

    const int graphX = 5;
    const int graphY = 200;

    for (int i = 0; i < 28; i++)
    {
        txtSprite.drawLine(graphX + i * 8,       graphY - graphBuffer[i]     - calibration,
                           graphX + (i + 1) * 8, graphY - graphBuffer[i + 1] - calibration, lineColor);
        txtSprite.drawLine(graphX + i * 8,       graphY - graphBuffer[i]     - calibration - 1,
                           graphX + (i + 1) * 8, graphY - graphBuffer[i + 1] - calibration - 1, lineColor);
    }
}

void drawGraphHeader(const char* title, const char* maxLabel, int topShift)
{
    txtSprite.drawCentreString(title, 50, 10, 2);
    txtSprite.drawCentreString(String(currentVoltage) + " V", 145, 10, 2);
    txtSprite.drawCentreString(String(currentAmperage) + " A", 202, 10, 2);
    txtSprite.drawCentreString(String(graphCapacity) + " " + maxLabel, 200, 35 + topShift, 2);
    txtSprite.drawCentreString("0 " + String(maxLabel).substring(0, 4), 205, 180 - topShift, 2);
}

// ============================================================================
// SCREEN 2 — VOLTAGE GRAPH
// ============================================================================
void Screen2()
{
    txtSprite.setFreeFont(&Orbitron_Light_32);
    txtSprite.setTextDatum(0);
    sprite.fillSprite(TFT_BLACK);
    txtSprite.fillSprite(TFT_BLACK);
    isAmpGraphScreen = false;
    drawGraphLine(voltageGraphBuffer, sideShift, topShift, GRAPH_CALIBRATION);
    drawGraphGrid(graphBars, sideShift, topShift);
    drawGraphHeader("Volt Graph", "volt", topShift);
}

// ============================================================================
// SCREEN 3 — AMPERAGE GRAPH
// ============================================================================
void Screen3()
{
    txtSprite.setFreeFont(&Orbitron_Light_32);
    txtSprite.setTextDatum(0);
    sprite.fillSprite(TFT_BLACK);
    txtSprite.fillSprite(TFT_BLACK);
    isAmpGraphScreen = true;

    const int graphX = 5;
    const int graphY = 200;

    for (int i = 0; i < 28; i++)
    {
        txtSprite.drawLine(graphX + i * 8,       graphY - voltageGraphBuffer[i],
                           graphX + (i + 1) * 8, graphY - voltageGraphBuffer[i + 1], TFT_WHITE);
        txtSprite.drawLine(graphX + i * 8,       graphY - voltageGraphBuffer[i]     - 1,
                           graphX + (i + 1) * 8, graphY - voltageGraphBuffer[i + 1] - 1, TFT_WHITE);
    }

    drawGraphGrid(graphBars, sideShift, topShift);
    drawGraphHeader("Amp Graph", "Amps", topShift);
}

// ============================================================================
// BOTTOM BAR — POWER STATUS + RUNTIME TIMER
// ============================================================================
void BotBar()
{
    static bool          wasPowerOn       = false;
    static unsigned long powerOnStartTime = 0;
    static unsigned long elapsedTime      = 0;

    char timeString[9];

    if (isPowerOutputEnabled)
    {
        digitalWrite(RELAY_PIN, HIGH);

        if (!wasPowerOn)
        {
            powerOnStartTime = millis();
            wasPowerOn       = true;
        }

        elapsedTime = millis() - powerOnStartTime;

        int hours   = (elapsedTime / 3600000UL) % 24;
        int minutes = (elapsedTime / 60000UL)   % 60;
        int seconds = (elapsedTime / 1000UL)    % 60;
        sprintf(timeString, "%02d:%02d:%02d", hours, minutes, seconds);

        if (currentScreenIndex == SCREEN_MAIN)
        {
            txtSprite.loadFont(fatFont);
            txtSprite.setTextDatum(0);
            txtSprite.fillRoundRect(10, 190, 120, 40, 8, TFT_GREEN);
            txtSprite.fillRoundRect(140, 190, 85, 40, 8, TFT_GREEN);
            txtSprite.drawCentreString(timeString, 70, 198, 1);
            txtSprite.drawCentreString("PW-ON",   182, 198, 1);
        }
        else
        {
            txtSprite.loadFont(middleFont);
            txtSprite.setTextDatum(0);
            txtSprite.setTextColor(TFT_WHITE);
            txtSprite.fillRect(5, 205, 226, 35, TFT_DARKGREEN);
            txtSprite.drawCentreString(timeString, 70, 210, 1);
            txtSprite.drawCentreString("PW-ON",   182, 210, 1);
        }
    }
    else
    {
        digitalWrite(RELAY_PIN, LOW);
        wasPowerOn  = false;
        elapsedTime = 0;
        strcpy(timeString, "00:00:00");

        if (currentScreenIndex == SCREEN_MAIN)
        {
            txtSprite.loadFont(fatFont);
            txtSprite.setTextDatum(0);
            txtSprite.fillRoundRect(10, 190, 120, 40, 8, TFT_DARKGREY);
            txtSprite.fillRoundRect(140, 190, 85, 40, 8, TFT_DARKGREY);
            txtSprite.drawCentreString(timeString, 70, 198, 1);
            txtSprite.drawCentreString("PW-OFF", 182, 198, 1);
        }
        else
        {
            txtSprite.loadFont(middleFont);
            txtSprite.setTextDatum(0);
            txtSprite.setTextColor(TFT_WHITE);
            txtSprite.fillRect(5, 205, 226, 35, TFT_RED);
            txtSprite.drawCentreString(timeString, 70, 210, 1);
            txtSprite.drawCentreString("PW-OFF", 182, 210, 1);
        }
    }

    txtSprite.unloadFont();
}

// ============================================================================
// ROTARY ENCODER VOLTAGE ADJUSTMENT OVERLAY
// Shows a gauge for GAUGE_SCREEN_DURATION_MS after each encoder movement.
//   Normal mode  — increment/decrement by voltageIncrement, confirm with click.
//   Hold mode    — scroll through preset voltages (1.5, 3.3, 5 … 25 V).
// ============================================================================
void RotAdjScr()
{
    static unsigned long lastRotationTime = 0;
    static float         temporaryVoltage = 0.0f;
    static bool          isVoltageSet     = false;
    static int           presetIndex      = 0;

    if (rotLeft || rotRight || rotHold)
        lastRotationTime = millis();

    if ((millis() - lastRotationTime < GAUGE_SCREEN_DURATION_MS) &&
        (millis() > GAUGE_SCREEN_DURATION_MS))
    {
        if (temporaryVoltage == 0.0f)
            temporaryVoltage = targetVoltage;

        // ── Normal (not holding) ─────────────────────────────────────────────
        if (!rotHold && !isVoltageSet)
        {
            txtSprite.fillRoundRect(5, 5, 227, 227, 8, TFT_NAVY);
            txtSprite.fillSmoothCircle(120, 120, 110, TFT_BLACK);
            txtSprite.fillSmoothCircle(120, 120, 95,  TFT_WHITE);
            txtSprite.fillSmoothCircle(120, 120, 85,  TFT_LIGHTGREY, TFT_WHITE);
            txtSprite.fillRect(69,  163, 103, 30, TFT_WHITE);
            txtSprite.fillRect(62,  175, 115, 20, TFT_WHITE);
            txtSprite.fillRect(73,  195,  94,  8, TFT_WHITE);
            txtSprite.fillRect(90,  203,  60,  5, TFT_WHITE);
            txtSprite.fillCircle(173, 173, 10, TFT_LIGHTGREY);
            txtSprite.fillSmoothCircle(120, 120, 65, TFT_WHITE);

            int gaugeVal = map((int)(temporaryVoltage * 10.0f),
                               (int)(minVoltage * 10.0f),
                               (int)(maxVoltage * 10.0f), 0, 100);
            drawGauge(gaugeVal);

            txtSprite.fillSmoothCircle(120, 120, 30, TFT_DARKGREY);
            txtSprite.setTextColor(TFT_BLACK);
            txtSprite.loadFont(hugeFatFont);
            txtSprite.drawCentreString(String(temporaryVoltage, 1), 120, 105, 1);

            if (rotClick)
            {
                txtSprite.fillCircle(120, 180, 10, TFT_GREEN);
                lastRotationTime = GAUGE_SCREEN_DURATION_MS + 500;
                voltageDisplayTimer = 9;
                targetVoltage = temporaryVoltage;
                rotClick = false;
                pref.putFloat("LastVolt", targetVoltage);
            }
            else
            {
                txtSprite.fillCircle(120, 180, 10, TFT_YELLOW);
            }
            txtSprite.unloadFont();
        }
        // Commit on hold release
        else if (!rotHold && isVoltageSet)
        {
            isVoltageSet    = false;
            lastRotationTime = GAUGE_SCREEN_DURATION_MS + 500;
            voltageDisplayTimer = 9;
            targetVoltage   = temporaryVoltage;
            pref.putFloat("LastVolt", targetVoltage);
        }
        // ── Hold — preset selection ──────────────────────────────────────────
        else if (rotHold)
        {
            txtSprite.fillRoundRect(5, 5, 227, 227, 8, TFT_DARKGREEN);
            txtSprite.fillSmoothCircle(120, 120, 110, TFT_BLACK);
            txtSprite.fillSmoothCircle(120, 120, 95,  TFT_WHITE);
            txtSprite.fillSmoothCircle(120, 120, 85,  TFT_LIGHTGREY, TFT_WHITE);
            txtSprite.fillRect(69,  163, 103, 30, TFT_WHITE);
            txtSprite.fillRect(62,  175, 115, 20, TFT_WHITE);
            txtSprite.fillRect(73,  195,  94,  8, TFT_WHITE);
            txtSprite.fillRect(90,  203,  60,  5, TFT_WHITE);
            txtSprite.fillCircle(173, 173, 10, TFT_LIGHTGREY);
            txtSprite.fillSmoothCircle(120, 120, 65, TFT_WHITE);

            int gaugeVal = map((int)(temporaryVoltage * 10.0f),
                               (int)(minVoltage * 10.0f),
                               (int)(maxVoltage * 10.0f), 0, 100);
            drawGauge(gaugeVal);

            txtSprite.fillSmoothCircle(120, 120, 30, TFT_DARKGREY);
            txtSprite.setTextColor(TFT_BLACK);
            txtSprite.loadFont(hugeFatFont);
            txtSprite.drawCentreString(String(temporaryVoltage, 1), 120, 105, 1);
            txtSprite.unloadFont();

            int maxPresets;
            if      (inputVoltageLevel == 5)  maxPresets = 7;
            else if (inputVoltageLevel == 12) maxPresets = 4;
            else if (inputVoltageLevel == 20) maxPresets = 6;
            else if (inputVoltageLevel == 28) maxPresets = 8;
            else                              maxPresets = 0;

            if (rotLeft)
            {
                presetIndex = max(0, presetIndex - 1);
                rotLeft     = false;
                isVoltageSet = true;
            }
            else if (rotRight)
            {
                presetIndex = min(maxPresets - 1, presetIndex + 1);
                rotRight    = false;
                isVoltageSet = true;
            }

            static const float presetVoltages[8] =
                { 1.5f, 3.3f, 5.0f, 9.0f, 12.0f, 15.0f, 20.0f, 25.0f };
            temporaryVoltage = presetVoltages[presetIndex];
        }

        // ── Fine voltage step when not holding ───────────────────────────────
        if (!rotHold)
        {
            if (rotLeft)
            {
                temporaryVoltage = max(minVoltage, temporaryVoltage - voltageIncrement);
                rotLeft = false;
            }
            else if (rotRight)
            {
                temporaryVoltage = min(maxVoltage, temporaryVoltage + voltageIncrement);
                rotRight = false;
            }
        }
    }
    else
    {
        temporaryVoltage = targetVoltage;
    }
}

// ============================================================================
// GRAPH DATA RECORDING
// Shifts the buffer left by one sample and appends the current reading.
// Bug fixed: original shift loop read voltageGraphPrevious[30] which was
// never written, producing garbage in the oldest buffer slot.
// ============================================================================
void GraphRec()
{
    // Determine graph scale
    if (!isAmpGraphScreen)
    {
        if      (targetVoltage > 20) { graphCapacity = 25; graphBars = 8; }
        else if (targetVoltage > 15) { graphCapacity = 20; graphBars = 7; }
        else if (targetVoltage > 10) { graphCapacity = 15; graphBars = 6; }
        else if (targetVoltage >  5) { graphCapacity = 10; graphBars = 5; }
        else                         { graphCapacity =  5; graphBars = 4; }
    }
    else
    {
        if      (currentAmperage > 3) { graphCapacity = 4; graphBars = 7; }
        else if (currentAmperage > 2) { graphCapacity = 3; graphBars = 6; }
        else if (currentAmperage > 1) { graphCapacity = 2; graphBars = 5; }
        else                          { graphCapacity = 1; graphBars = 4; }
    }

    int pixelHeight = isAmpGraphScreen
        ? map((int)(currentAmperage * 20), 0, graphCapacity * 20,
              0, (200 - topShift2) - (5 + topShift2))
        : map((int)(currentVoltage * 20), 0, graphCapacity * 20,
              0, (200 - topShift)  - (5 + topShift));

    // Shift buffer left by 1, then append new sample at [29]
    memmove(voltageGraphBuffer, voltageGraphBuffer + 1, 29 * sizeof(int));
    voltageGraphBuffer[29] = pixelHeight;
}

// ============================================================================
// GAUGE — 270° arc from 135° to 405°
// value: 0–100 (clamped)
// ============================================================================
void drawGauge(int value)
{
    value = constrain(value, 0, 100);

    const uint16_t GAUGE_COLOR     = TFT_DARKGREEN;
    const int      GAUGE_RADIUS    = 85;
    const int      GAUGE_THICKNESS = 20;
    const int      CX              = 120;
    const int      CY              = 120;
    const int      ANGLE_START     = 135;
    const int      ANGLE_END       = 405;

    for (int angle = ANGLE_START; angle <= ANGLE_END; angle++)
    {
        if (map(angle, ANGLE_START, ANGLE_END, 0, 100) > value)
            continue;

        for (int i = 0; i < GAUGE_THICKNESS; i++)
        {
            int   r    = GAUGE_RADIUS - i;
            float a0   = radians(angle);
            float a1   = radians(angle + 1);
            txtSprite.drawLine(CX + r * cos(a0), CY + r * sin(a0),
                               CX + r * cos(a1), CY + r * sin(a1), GAUGE_COLOR);
        }
    }

    // Needle
    int   needleAngle  = map(value, 0, 100, ANGLE_START, ANGLE_END);
    float needleRad    = radians(needleAngle);
    int   needleR      = GAUGE_RADIUS - GAUGE_THICKNESS / 2;
    int   needleX      = CX + needleR * cos(needleRad);
    int   needleY      = CY + needleR * sin(needleRad);

    txtSprite.drawWideLine(CX, CY, needleX, needleY, 10, GAUGE_COLOR);

    // End caps
    float startRad = radians(ANGLE_START);
    int   startX   = CX + needleR * cos(startRad);
    int   startY   = CY + needleR * sin(startRad);
    txtSprite.fillCircle(startX,  startY,  GAUGE_THICKNESS / 2, GAUGE_COLOR);
    txtSprite.fillCircle(needleX, needleY, GAUGE_THICKNESS / 2, GAUGE_COLOR);
}
