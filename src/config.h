#pragma once

// ============================================================================
// PIN DEFINITIONS
// ============================================================================
#define SERVO_PIN                 12
#define ROTARY_ENCODER_A_PIN      32
#define ROTARY_ENCODER_B_PIN      35
#define ROTARY_ENCODER_BUTTON_PIN 33
#define ROTARY_ENCODER_VCC_PIN    -1
#define ROTARY_ENCODER_STEPS       2

#define V28_PIN    14
#define V20_PIN    27
#define V12_PIN    34
#define V5_PIN     25
#define RELAY_PIN  13

// ============================================================================
// DISPLAY
// ============================================================================
#define DISPLAY_WIDTH  240
#define DISPLAY_HEIGHT 240

// ============================================================================
// ROTARY ENCODER TIMING
// ============================================================================
#define SINGLE_CLICK_TIME_MS  550
#define HOLD_THRESHOLD_MS     500
#define GAUGE_SCREEN_DURATION_MS 2000

// ============================================================================
// VOLTAGE THRESHOLD CONSTANTS (ADC values)
// ============================================================================
#define VOLT_THRESHOLD_28V 2800
#define VOLT_THRESHOLD_20V 2200
#define VOLT_THRESHOLD_12V 1500
#define VOLT_THRESHOLD_5V  2000

// ============================================================================
// MOTOR CONTROL
// ============================================================================
#define MOTOR_STATE_FORWARD  1
#define MOTOR_STATE_BACKWARD 2
#define MOTOR_STATE_STOP     3
#define MOTOR_MIN_SPEED      10
#define MOTOR_MAX_SPEED      90
#define MOTOR_CENTER_POSITION 90

// ============================================================================
// VOLTAGE REGULATION
// ============================================================================
#define VOLTAGE_TOLERANCE_LARGE  0.5f
#define VOLTAGE_TOLERANCE_SMALL  0.05f

// Knocking — fine-adjustment pulses for small errors.
// DURATION: how long each pulse lasts (must be > 20ms, one 50 Hz servo frame).
// INTERVAL_SLOW: pause between pulses when error is near the settled threshold.
// INTERVAL_FAST: pause between pulses when error is near the coarse boundary.
// FINE_MAX_SPEED: highest speed used inside the fine zone.
// All four values scale linearly with error — see VoltADJ Zone 2.
#define KNOCK_DURATION_MS       100
#define KNOCK_INTERVAL_SLOW_MS 250 
#define KNOCK_INTERVAL_FAST_MS 200
#define KNOCK_FINE_MAX_SPEED    20

// Coarse-mode stuck detection
#define STUCK_TIMEOUT_MS          5000
#define STUCK_SPEED_INCREMENT_MAX    3

// ============================================================================
// DEBUG
// Set DEBUG_KNOCK 1 to print knock events and zone transitions to Serial.
// Set to 0 before flashing a production build.
// ============================================================================
#define DEBUG_KNOCK 1

// ============================================================================
// PID TUNING
// Kp: proportional gain  — primary response speed
// Ki: integral gain       — eliminates steady-state error from pot friction
// Kd: derivative gain     — set to 0; mechanical systems amplify sensor noise
//
// Tuning guide:
//   1. Start with Ki=0, Kd=0. Raise Kp until output is snappy but not oscillating.
//   2. Add Ki slowly until steady-state error disappears.
//   3. Only add Kd if you see persistent overshoot after Ki is tuned.
// ============================================================================
#define PID_KP            10.0f
#define PID_KI             0.5f
#define PID_KD             0.0f
#define PID_INTEGRAL_MAX  30.0f   // Anti-windup clamp
#define PID_INTEGRAL_MIN -30.0f

// ============================================================================
// SCREENS
// ============================================================================
#define SCREEN_MAIN       0
#define SCREEN_VOLT_GRAPH 1
#define SCREEN_AMP_GRAPH  2

// ============================================================================
// GRAPH
// ============================================================================
#define GRAPH_BUFFER_SIZE  31
#define GRAPH_PADDING_LEFT  5
#define GRAPH_CALIBRATION -50
