# ESP32 Lab Power Supply

An ESP32-based adjustable lab power supply controller with a TFT display, rotary encoder UI, and closed-loop PID voltage regulation via a servo-driven potentiometer.

## Hardware

| Component | Detail |
|---|---|
| MCU | ESP32 (DOIT DevKit v1) |
| Display | 240×240 TFT (TFT_eSPI) |
| Current/Voltage sensor | Adafruit INA219 (I²C) |
| User input | Rotary encoder with push button |
| Actuator | Continuous-rotation servo → trim potentiometer |
| Output control | Relay on GPIO 13 |
| Input detection | ADC pins for 5 V / 12 V / 20 V / 28 V rails |

## Features

- **Automatic input detection** — reads the connected supply rail and sets voltage limits accordingly
- **PID voltage regulation** — three-zone controller drives a servo to adjust the output potentiometer
- **Rotary encoder UI** — rotate to adjust voltage, single-click to confirm, double-click to toggle output, hold for preset voltages
- **Live TFT display** — voltage, current, wattage readout plus scrolling voltage and current graphs
- **Runtime timer** — tracks how long the output has been enabled
- **NVS persistence** — last target voltage is saved and restored on power-up

## Voltage Regulation Zones

| Indicator | Condition | Behaviour |
|---|---|---|
| Red | `\|error\| > 0.5 V` | Motor runs continuously at PID speed |
| Yellow | `0.05 V < \|error\| ≤ 0.5 V` | Adaptive knocking — pulse speed and interval scale with error |
| Green | `\|error\| ≤ 0.05 V` | Motor stopped, settled |

## Pin Map

| Pin | Function |
|---|---|
| 12 | Servo signal |
| 13 | Relay (output enable) |
| 14 | 28 V input detect |
| 27 | 20 V input detect |
| 34 | 12 V input detect |
| 25 | 5 V input detect |
| 32 | Encoder DT |
| 35 | Encoder CLK |
| 33 | Encoder SW |

SPI and I²C pins are configured via `TFT_eSPI` `User_Setup.h` and the Arduino Wire defaults.

## Build

Built with [PlatformIO](https://platformio.org/).

```bash
pio run -t upload
pio device monitor --baud 115200
```

Dependencies are declared in `platformio.ini` and fetched automatically.

## Project Structure

```
src/
├── Main.cpp        — globals, tasks, setup(), loop()
├── config.h        — all tunable constants and pin definitions
├── globals.h       — extern declarations shared across modules
├── sensors.h/cpp   — INA219 reading, input voltage detection
├── motor.h/cpp     — PID controller, servo driver, VoltADJ
├── display.h/cpp   — all screens, graphs, gauge overlay
├── encoder.h/cpp   — rotary encoder event detection
└── fonts/          — VLW and GFX font data headers
```

## Tuning

All tunable values live in `src/config.h`.

**PID gains**
```c
#define PID_KP  10.0f   // proportional — raise for faster response
#define PID_KI   0.5f   // integral     — eliminates steady-state error
#define PID_KD   0.0f   // derivative   — leave at 0 for mechanical systems
```

**Knocking (fine zone)**
```c
#define KNOCK_DURATION_MS       100   // pulse length
#define KNOCK_INTERVAL_SLOW_MS  250   // pause near settled threshold
#define KNOCK_INTERVAL_FAST_MS  200   // pause near coarse boundary
#define KNOCK_FINE_MAX_SPEED     20   // max servo speed in fine zone
#define MOTOR_MIN_SPEED          10   // raise if servo stalls at low speed
```

**Debug output** — set `DEBUG_KNOCK 1` in `config.h` to print zone transitions and knock events to Serial at 115200 baud.
