#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include "FS.h"
#include "SPIFFS.h"
#include <Adafruit_INA219.h>
#include "TFT_eSPI.h"
// TaskScheduler.h defines non-inline functions — including it in multiple
// translation units causes ODR link errors.  Only Main.cpp includes it
// directly; here we just forward-declare the type we need for the extern.
class Scheduler;
#include "AiEsp32RotaryEncoder.h"
#include "ESP32Servo.h"
#include <Preferences.h>
#include "config.h"

// ============================================================================
// SHARED OBJECTS
// ============================================================================
extern Preferences            pref;
extern AiEsp32RotaryEncoder   rotaryEncoder;
extern TFT_eSPI               tft;
extern TFT_eSprite            sprite;
extern TFT_eSprite            txtSprite;
extern Adafruit_INA219        ina219;
extern Servo                  myservo;
extern Scheduler              userScheduler;

// ============================================================================
// POWER SUPPLY STATE
// ============================================================================
extern float currentVoltage;
extern float currentAmperage;
extern float targetVoltage;
extern bool  isPowerOutputEnabled;

// Input detection
extern int   inputVoltageLevel;
extern float minVoltage;
extern float maxVoltage;
extern float voltageIncrement;
extern bool  isPowerInputOff;

// Regulation status (used by display)
extern bool isVoltageMatched;
extern bool isVoltageMismatched;
extern bool isKnockingEnabled;

// ============================================================================
// ROTARY ENCODER STATE
// ============================================================================
extern bool rotRight;
extern bool rotLeft;
extern bool rotClick;
extern bool rotHold;
extern bool rotDown;
extern bool rotDoubleClick;

// ============================================================================
// DISPLAY STATE
// ============================================================================
extern int  currentScreenIndex;
extern int  voltageDisplayTimer;
extern int  switchSpeed;
extern int  graphCapacity;
extern int  graphBars;
extern int  sideShift;
extern int  topShift;
extern int  topShift2;
extern bool isAmpGraphScreen;
extern int  voltageGraphBuffer[GRAPH_BUFFER_SIZE];
extern int  voltageGraphPrevious[GRAPH_BUFFER_SIZE];
