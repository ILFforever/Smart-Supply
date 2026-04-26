// ============================================================================
// MAIN — global definitions, task scheduler, setup, loop
// ============================================================================
#include <TaskScheduler.h>  // must be first — only included in this TU
#include "globals.h"
#include "sensors.h"
#include "motor.h"
#include "display.h"
#include "encoder.h"

// ============================================================================
// GLOBAL OBJECT DEFINITIONS
// ============================================================================
Preferences           pref;
AiEsp32RotaryEncoder  rotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN,
                                    ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN,
                                    ROTARY_ENCODER_STEPS);
TFT_eSPI              tft;
TFT_eSprite           sprite(&tft);
TFT_eSprite           txtSprite(&tft);
Adafruit_INA219       ina219;
Servo                 myservo;
Scheduler             userScheduler;

// ============================================================================
// GLOBAL VARIABLE DEFINITIONS
// ============================================================================
float currentVoltage      = 0.0f;
float currentAmperage     = 0.0f;
float targetVoltage       = 0.0f;
bool  isPowerOutputEnabled = false;

int   inputVoltageLevel   = 0;
float minVoltage          = 0.0f;
float maxVoltage          = 0.0f;
float voltageIncrement    = 0.0f;
bool  isPowerInputOff     = true;

bool  isVoltageMatched    = false;
bool  isVoltageMismatched = false;
bool  isKnockingEnabled   = false;

bool  rotRight       = false;
bool  rotLeft        = false;
bool  rotClick       = false;
bool  rotHold        = false;
bool  rotDown        = false;
bool  rotDoubleClick = false;

int   currentScreenIndex  = SCREEN_MAIN;
int   voltageDisplayTimer = 0;
int   switchSpeed         = MOTOR_MIN_SPEED;
int   graphCapacity       = 0;
int   graphBars           = 0;
int   sideShift           = 0;
int   topShift            = 0;
int   topShift2           = 25;
bool  isAmpGraphScreen    = false;
int   voltageGraphBuffer[GRAPH_BUFFER_SIZE]   = {0};
int   voltageGraphPrevious[GRAPH_BUFFER_SIZE] = {0};

// ============================================================================
// TASK SCHEDULER
// ============================================================================
Task taskChkInput (TASK_SECOND * 0.5,  TASK_FOREVER, &CheckInput);
Task taskVolt     (TASK_SECOND * 0.01, TASK_FOREVER, &VoltADJ);
Task taskValue    (TASK_SECOND * 0.005,TASK_FOREVER, &ValueUpdate);
Task taskDisplay  (TASK_SECOND * 0.05, TASK_FOREVER, &displayUpdate);
Task taskGraphRec (TASK_SECOND * 0.20, TASK_FOREVER, &GraphRec);

// ============================================================================
// SETUP
// ============================================================================
void setup()
{
    Serial.begin(115200);

    // Servo
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
    myservo.setPeriodHertz(50);
    myservo.attach(SERVO_PIN, 1000, 2000);

    // INA219
    if (!ina219.begin())
        Serial.println("Failed to find INA219 chip");

    // Restore last target voltage
    pref.begin("LastVolt", false);
    targetVoltage = pref.getFloat("LastVolt", 0.0f);

    // Display
    tft.begin();
    tft.fillScreen(TFT_RED);
    delay(500);
    tft.setRotation(2);
    tft.setSwapBytes(true);
    tft.fillScreen(TFT_BLACK);

    // Input voltage pins
    pinMode(V28_PIN,   INPUT_PULLDOWN);
    pinMode(V20_PIN,   INPUT_PULLDOWN);
    pinMode(V12_PIN,   INPUT_PULLDOWN);
    pinMode(V5_PIN,    INPUT_PULLDOWN);
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, HIGH);
    delay(1000);

    CheckInput();

    // Sprites
    sprite.createSprite(DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
    sprite.fillSprite(TFT_BLACK);
    sprite.setSwapBytes(true);
    txtSprite.createSprite(235, 235);
    txtSprite.setTextColor(TFT_WHITE, TFT_BLACK);
    txtSprite.setSwapBytes(true);

    // Tasks
    userScheduler.addTask(taskVolt);
    userScheduler.addTask(taskValue);
    userScheduler.addTask(taskDisplay);
    userScheduler.addTask(taskChkInput);
    userScheduler.addTask(taskGraphRec);
    taskVolt.enable();
    taskValue.enable();
    taskDisplay.enable();
    taskChkInput.enable();
    taskGraphRec.enable();

    // Rotary encoder
    rotaryEncoder.begin();
    rotaryEncoder.setup(readEncoderISR);
    rotaryEncoder.setBoundaries(-1000, 1000, false);
    rotaryEncoder.disableAcceleration();
}

// ============================================================================
// LOOP
// ============================================================================
void loop()
{
    rotary_loop();
    userScheduler.execute();
}
