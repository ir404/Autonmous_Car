# Autonomous Navigation Architecture

This document explains the hardware interaction, control flow, decision logic, and fail-safe recovery mechanisms behind the autonomous obstacle-avoidance mode in `main_controller/main_controller.ino`.

---

## 1. System Overview

The vehicle uses a **Stop-and-Scan** navigation strategy rather than continuous radar sweeping while in motion. Halting the drive motors before sweeping the ultrasonic sensor serves two purposes:
1. **Measurement Stability:** Prevents chassis movement from skewing angular distance readings mid-sweep.
2. **Power Integrity:** Avoids simultaneous current draw spikes from the TT drive motors and the SG90 sweep servo on the battery pack.

### Hardware Mapping
| Component | Function | ESP32 Pin | Operating Range |
| :--- | :--- | :--- | :--- |
| **Steering Servo** | Front wheel Ackermann steering | `GPIO 13` | `60°` (Left) – `90°` (Centre) – `120°` (Right) |
| **Sweep Servo** | Rotates ultrasonic sensor head | `GPIO 4` | `60°` (Right) – `90°` (Centre) – `120°` (Left) |
| **HC-SR04 Trigger** | Emits 40 kHz ultrasonic pulse | `GPIO 12` | 10 µs HIGH pulse |
| **HC-SR04 Echo** | Measures reflected pulse duration | `GPIO 14` | `pulseIn()` timeout at 30,000 µs |
| **Motor Driver** | Forward, Reverse, and PWM Enable | `GPIO 18, 19, 5` | 8-bit PWM (`0–255`) at 1 kHz |

---

## 2. Cloud Coexistence & Instant Override

Autonomous mode is activated via Blynk when both the master power switch (`carOn_DS` on `V0`) and the mode switch (`autoMode_DS` on `V6`) are set to `1`.

Because a full sensor sweep and reverse recovery take several hundred milliseconds, standard blocking `delay()` calls would starve `Blynk.run()`, disconnecting the vehicle from the cloud and preventing emergency stops. Instead, all pauses inside the autonomous pipeline route through `waitWithBlynk(durationMs)`:
* Continuously polls `Blynk.run()` in 5 ms increments during servo settling and motor pulses.
* Monitors `carOn_DS` and `autoMode_DS` in real time. If either switch is toggled off mid-manoeuvre, the motors halt immediately and control returns to `loop()`.

---

## 3. Navigation Control Pipeline

### Step 1: Mode Initialisation
When switching into autonomous mode (`!wasAutoMode`), `runAutonomousMode()` centres both servos to `90°`, clears any leftover scan history (`hasPrevScan = false`), and sets `wasAutoMode = true` so the vehicle always begins scanning from a known neutral alignment.

### Step 2: Forward Cruise & Straight-Ahead Polling
While the path ahead is clear (`frontDistance > 20.0 cm`):
* The sweep servo remains locked at `90°` (dead ahead).
* The vehicle drives forward at a reduced, controlled PWM duty cycle (`AUTO_MOTOR_SPEED = 140`) to ensure sufficient reaction distance before braking.
* Whenever unobstructed forward travel occurs, `hasPrevScan` is reset to `false` so stale obstacle data from earlier encounters is discarded.

### Step 3: Obstacle Halt & Bounded Radar Sweep
As soon as `getDistanceCM()` detects an obstacle at or below `MIN_SAFE_DISTANCE` (`20.0 cm`), `turnOffMotor()` halts the car and calls `scanAndSteer()`:
1. **Bounded Arc (`60°` to `120°`):** Rather than sweeping a full `0°–180°` arc—where shallow wall angles cause 40 kHz sound waves to bounce away without returning an echo—the sweep is constrained to `±30°` around centre.
2. **7-Point Sampling:** The sensor steps from `60°` (right) to `120°` (left) in `10°` increments (`60°, 70°, 80°, 90°, 100°, 110°, 120°`), waiting `80 ms` per step for mechanical settling and echo stability.
3. **Timeout & Range Clamping:** In `getDistanceCM()`, echo timeouts (`duration == 0`) return `TIMEOUT_DISTANCE` (`150.0 cm`) and valid echoes are capped at `MAX_SENSOR_RANGE` (`200.0 cm`). This prevents a lost echo on an angled surface from outranking a genuinely clear path.
4. **Extremum Extraction:** The sweep records both the widest open gap (`maxDistance` at `bestSweepAngle`) and the closest obstruction (`minDistance` at `closestSweepAngle`).

### Step 4: Stall-Detection Memory & Reverse Triggers
Because the vehicle uses front-wheel steering rather than on-the-spot tank rotation, an open reading on one side of the sweep does not guarantee the chassis has enough turning radius to clear a close wall on the opposite side. Before steering forward, the controller checks three reverse conditions:

1. **Boxed In (`maxDistance < 10.0 cm`):** Every angle across the `60°–120°` arc is blocked below `CRITICAL_DISTANCE`.
2. **Insufficient Pivot Room (`minDistance < 6.0 cm`):** At least one angle in the arc is closer than `MIN_CORNER_CLEARANCE`, meaning the front bumper corner would clip the obstacle while turning.
3. **Scan-Memory Stall Detection (`isStalled()`):** If the car previously chose a path, executed its forward turn pulse, and immediately re-triggered at `<= 20.0 cm`, `isStalled()` compares the new 7-element `distances[]` array against `prevDistances[]`. If every angle matches the previous scan within `STALL_MARGIN_CM` (`3.0 cm`), the vehicle physically failed to advance (e.g. wedged against a wall at an oblique angle) and forces a reverse.

### Step 5: Counter-Steered Reverse Recovery
When any reverse condition triggers, `counterSteerReverse(closestSweepAngle)` executes:
1. **Counter-Steering:** Maps `closestSweepAngle` to steer the front wheels **towards** the side of the closest obstacle. Reversing with wheels turned toward the wall swings the front nose away from the obstruction rather than backing straight along the collision trajectory.
2. **Timed Reverse Pulse:** Drives backward at `AUTO_MOTOR_SPEED` (`140`) for `REVERSE_DURATION_MS` (`750 ms`), halts, clears `hasPrevScan`, and loops back to perform a fresh radar sweep.
3. **Dead-End Safety Cap:** If the vehicle remains trapped after `MAX_REVERSE_ATTEMPTS` (`3` consecutive reverses), it centres the steering and remains stationary to protect the motors and battery.

### Step 6: Steering & Turn Clearance Pulse
Once a valid, non-stalled path is identified:
1. **Save Scan History:** `saveScanHistory()` copies the current 7 readings into `prevDistances[]` for stall verification on the next cycle.
2. **Map Sweep to Steering:** `bestSweepAngle` (`60°–120°`) is inversely mapped to `steeringServo` (`120°–60°`) so a clear gap on the left (`120°` sweep) steers the front wheels left (`60°` steer), and vice versa.
3. **Re-Centre & Clear:** The sweep servo returns to `90°`, and the car drives forward for `TURN_PULSE_MS` (`350 ms`) to pivot the chassis away from the obstacle before resuming normal forward polling.

---

## 4. Tuning Reference

All behavioural thresholds are defined as constants at the top of `main_controller/main_controller.ino` for floor calibration:

| Constant | Default Value | Purpose |
| :--- | :--- | :--- |
| `AUTO_MOTOR_SPEED` | `140` (PWM) | Autonomous drive speed (`0–255`); lower gives more braking margin |
| `MIN_SAFE_DISTANCE` | `20.0 cm` | Distance at which forward motion stops to initiate a scan |
| `CRITICAL_DISTANCE` | `10.0 cm` | Minimum required clearance in the best direction to avoid reversing |
| `MIN_CORNER_CLEARANCE` | `6.0 cm` | Minimum allowable proximity on any single angle before forcing reverse |
| `STALL_MARGIN_CM` | `3.0 cm` | Tolerance band when comparing consecutive scans for stall detection |
| `REVERSE_DURATION_MS` | `750 ms` | Duration of each counter-steered reverse pulse |
| `TURN_PULSE_MS` | `350 ms` | Forward drive duration after steering to pivot clear of an obstacle |
| `MAX_REVERSE_ATTEMPTS` | `3` | Maximum consecutive reverse attempts before halting in a dead end |