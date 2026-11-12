# Blynk IoT Cloud & Datastream Specification

This document defines the Blynk Cloud template configuration, virtual pin mappings, and mobile dashboard bindings used by `main_controller/main_controller.ino`.

---

## 1. Template Overview

* **Template Name:** `HAW AutonomousCar`
* **Hardware Platform:** ESP32 (Wi-Fi)
* **Firmware Library:** `BlynkSimpleEsp32` (v1.4.0)

---

## 2. Virtual Pin Datastreams

All datastreams are configured as **Integer** types on the Blynk server:

| Virtual Pin | Datastream Name | Firmware Variable | Data Type | Min | Max | Default | Mobile Widget | Functional Role |
| :--- | :--- | :--- | :--- | :---: | :---: | :---: | :--- | :--- |
| **`V0`** | `OnOff` | `carOn_DS` | Integer | `0` | `1` | `0` | Switch | Master power safety interlock (`0` = Idle/Off, `1` = Active) |
| **`V1`** | `Move Forward` | `btnFwd_DS` | Integer | `0` | `1` | `0` | Push Button | Digital forward drive command (manual mode) |
| **`V2`** | `Move Back` | `btnBack_DS` | Integer | `0` | `1` | `0` | Push Button | Digital reverse drive command (manual mode) |
| **`V3`** | `Car Speed` | `carSpeed_DS` | Integer | `0` | `255` | `200` | Slider | 8-bit PWM duty cycle for manual drive speed |
| **`V4`** | `CarX` | `carX_DS` | Integer | `0` | `1023` | `512` | Joystick (X) | Proportional front-wheel steering angle (`45°`–`135°`) |
| **`V5`** | `CarY` | `carY_DS` | Integer | `0` | `1023` | `512` | Joystick (Y) | Proportional forward/reverse throttle trigger |
| **`V6`** | `AutoMode` | `autoMode_DS` | Integer | `0` | `1` | `0` | Switch | Control mode selector (`0` = Manual, `1` = Autonomous) |

---

## 3. Firmware Signal Mapping & Deadzones

### Manual Drive (`V1`, `V2`, `V5`)
* **Forward Trigger:** Activates when `(btnFwd_DS == 1 && btnBack_DS == 0)` **or** `carY_DS >= 600`.
* **Reverse Trigger:** Activates when `(btnFwd_DS == 0 && btnBack_DS == 1)` **or** `carY_DS <= 400`.
* **Neutral Deadzone:** Joystick Y-axis values between `401` and `599` hold the motors stopped (`turnOffMotor()`) when neither button is pressed.

### Manual Steering (`V4`)
* **Centre Deadzone:** `450 <= carX_DS <= 550` commands the steering servo back to `90°` (`SERVO_CENTRE`).
* **Active Range:** Values outside the deadzone map across `0–1023` to `45°` (full left) through `135°` (full right), stepped incrementally by `STEERING_STEP` (`2°` per `5 ms` loop cycle) to prevent servo jitter.

### Autonomous Override (`V0`, `V6`)
* Toggling `V6` (`AutoMode`) to `1` while `V0` (`OnOff`) is `1` hands control to `runAutonomousMode()`, which overrides `carSpeed_DS` with the fixed `AUTO_MOTOR_SPEED` (`140` PWM) and constrains steering to `60°–120°`.
* Setting either `V0` or `V6` to `0` interrupts `waitWithBlynk()` within `5 ms`, immediately halting the drive motors and returning control to idle or manual mode.

---

## 4. Planned Telemetry Extensions

Future firmware iterations can add read-only datastreams (`ESP32` $\rightarrow$ `Blynk`) for live dashboard monitoring:
* **`V7` (`FrontDistance`):** Double (`0.0–200.0 cm`) streaming live HC-SR04 clearance.
* **`V8` (`NavState`):** String/Enum reporting current autonomous state (`CRUISING`, `SCANNING`, `REVERSING`, `TRAPPED`).