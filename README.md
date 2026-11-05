# SmartDrive – Autonomous Vehicle Project

## Project Overview
This is an ongoing university elective project focused on designing, assembling, and programming an autonomous robotic vehicle. The ultimate goal of this project is to develop a vehicle capable of navigating a realistic environment, avoiding obstacles, and making AI-based driving decisions, all while being monitored and teleoperated via an IoT cloud server. 

This project integrates hardware assembly, real-time electronics, path planning, and telemetry into a complete software architecture.

## Current Project Status
We have successfully assembled the 3D-printed chassis, wired the core drivetrain components, and established a cloud connection. Currently, the car can be remotely controlled over Wi-Fi using a custom mobile app interface featuring dynamic joystick mapping and algorithmically smoothed steering. We have established a dual Auto/Manual mode architecture and are currently working on integrating the ultrasonic sensor and its panning servo for our first layer of autonomous environmental perception.

## Hardware Components (So Far)
-   **Microcontroller:** ESP32 Development Board (features built-in Wi-Fi and Bluetooth)
-   **Chassis:** Custom 3D-printed body
-   **Drivetrain:** TT Motors & standard rear wheels
-   **Motor Driver:** (Used to safely deliver sufficient power from the batteries to the TT motors)
-   **Steering:** Servo Motor (connected to front wheel linkage)
-   **Sensors:** HC-SR04 Ultrasonic Sensor (for obstacle distance measurement - integration in progress)
-   **Sensor Sweep:** Second Servo Motor (to pan the ultrasonic sensor 180° - hardware pending)
-   **Power Supply:** 2x 3.7V batteries connected to meet the voltage/current requirements, plus a master power switch.

## Software & Tools
-   **Programming Environment:** Arduino IDE (C++)
-   **IoT Platform:** Blynk Cloud & Blynk Mobile App (for remote control and telemetry)
-   **Libraries Used:**
    -   `BlynkSimpleEsp32` (v1.4.0) - Handles Wi-Fi connection and cloud communication.
    -   `ESP32Servo` (v3.0.5) - Generates 50Hz PWM signals for the servos without hardware timer conflicts.

## Directory Structure
-   `main_controller/` - Contains the main project sketch housing the master state machine, teleoperation, and autonomous modes.
-   `diagnostics/` - A suite of standalone test sketches used to isolate and verify individual components (steering smoothing, ultrasonic polling, auto-braking, radar sweeps).
-   [`DEV_TIMELINE.md`](DEV_TIMELINE.md) - A detailed, session-by-session development log tracking milestones and hardware learnings.

## Future Milestones (Planned)
-   Integrate ultrasonic sensor logic to auto-brake when obstacles are detected.
-   Implement a 180° radar scan routine for obstacle avoidance and path planning.
-   Add a camera module for AI-based environment perception and image processing.