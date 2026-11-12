# SmartDrive – Autonomous Vehicle Project

## Project Overview
This is an ongoing university elective project focused on designing, assembling, and programming an autonomous robotic vehicle. The ultimate goal of this project is to develop a vehicle capable of navigating a realistic environment, avoiding obstacles, and making AI-based driving decisions, all while being monitored and teleoperated via an IoT cloud server. 

This project integrates hardware assembly, real-time electronics, path planning, and telemetry into a complete software architecture.

## Current Project Status
We have assembled the 3D-printed chassis, wired the core drivetrain and sensor subsystems, and integrated both manual teleoperation and autonomous obstacle avoidance into `main_controller.ino`. Over Wi-Fi via the Blynk mobile app, the vehicle supports smooth joystick-driven manual control as well as an instant-override autonomous mode powered by a **Stop-and-Scan** radar routine (`±30°` sweep arc), scan-memory stall detection, and counter-steered reverse recovery.

## Hardware Components
-   **Microcontroller:** ESP32 Development Board (features built-in Wi-Fi and Bluetooth)
-   **Chassis:** Custom 3D-printed body
-   **Drivetrain:** TT Motors & standard rear wheels
-   **Motor Driver:** Dual H-bridge motor driver (delivers regulated current from the batteries to the TT motors)
-   **Steering:** Servo Motor (connected to front wheel linkage on `GPIO 13`)
-   **Sensors:** HC-SR04 Ultrasonic Sensor (`GPIO 12` Trigger, `GPIO 14` Echo) for forward and angular obstacle distance measurement
-   **Sensor Sweep:** SG90 Micro Servo (`GPIO 4`) mounted with the HC-SR04 to sweep a bounded `60°–120°` arc
-   **Power Supply:** 2x 3.7V batteries connected in series to meet voltage/current requirements, plus a master power switch

## Software & Tools
-   **Programming Environment:** Arduino IDE (C++)
-   **IoT Platform:** Blynk Cloud & Blynk Mobile App (for remote control, mode switching, and telemetry)
-   **Libraries Used:**
    -   `BlynkSimpleEsp32` (v1.4.0) - Handles Wi-Fi connection and cloud communication.
    -   `ESP32Servo` (v3.0.5) - Generates 50Hz PWM signals for the steering and sweep servos without hardware timer conflicts.

## Directory Structure
-   `main_controller/` - Contains `main_controller.ino`, the master firmware housing the Blynk cloud state machine, `runManualMode()`, and `runAutonomousMode()`.
-   `diagnostics/` - A suite of standalone test sketches used to isolate and verify individual components (steering smoothing, ultrasonic polling, auto-braking, stop-and-scan radar sweeps).
-   [`AUTONOMOUS_MODE.md`](AUTONOMOUS_MODE.md) - Technical breakdown of the stop-and-scan pipeline, scan-memory stall detection, counter-steered reverse recovery, and tuning constants.
-   [`BLYNK_SETUP.md`](BLYNK_SETUP.md) - Specification of Blynk Cloud template settings, virtual pin datastreams (`V0–V6`), and joystick deadzone mappings.
-   [`TIMELINE.md`](TIMELINE.md) - A detailed, session-by-session development log tracking milestones and hardware learnings.

## Future Milestones (Planned)
-   Train an AI model for sign recognition to aid with autonmous driving.
-   Add a camera module for AI-based environment perception and vision-guided navigation.
-   Stream live distance telemetry and autonomous state feedback to the Blynk dashboard.