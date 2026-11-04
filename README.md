# SmartDrive – Autonomous Vehicle Project

## Project Overview
This is an ongoing university elective project focused on designing, assembling, and programming an autonomous robotic vehicle. The ultimate goal of this project is to develop a vehicle capable of navigating a realistic environment, avoiding obstacles, and making AI-based driving decisions, all while being monitored and teleoperated via an IoT cloud server. 

This project integrates hardware assembly, real-time electronics, path planning, and telemetry into a complete software architecture.

## Current Project Status
We have successfully assembled the 3D-printed chassis, wired the core drivetrain components, and established a cloud connection. Currently, the car can be remotely controlled (forward, reverse, speed, and steering diagnostics) over Wi-Fi using a custom mobile app interface. We are currently working on integrating the ultrasonic sensor for our first layer of environmental perception.

## Hardware Components (So Far)
*   **Microcontroller:** ESP32 Development Board (features built-in Wi-Fi and Bluetooth)
*   **Chassis:** Custom 3D-printed body
*   **Drivetrain:** TT Motors & standard rear wheels
*   **Motor Driver:** (Used to safely deliver sufficient power from the batteries to the TT motors)
*   **Steering:** Servo Motor (connected to front wheel linkage)
*   **Sensors:** HC-SR04 Ultrasonic Sensor (for obstacle distance measurement - integration in progress)
*   **Power Supply:** 2x 3.7V batteries connected to meet the voltage/current requirements, plus a master power switch.

## Software & Tools
*   **Programming Environment:** Arduino IDE (C++)
*   **IoT Platform:** Blynk Cloud & Blynk Mobile App (for remote control and telemetry)
*   **Libraries Used:**
    *   `BlynkSimpleEsp32` (v1.4.0) - Handles Wi-Fi connection and cloud communication.
    *   `ESP32Servo` (v3.0.5) - Generates 50Hz PWM signals for the steering servo without hardware timer conflicts.

## Future Milestones (Planned)
*   Integrate ultrasonic sensor logic to auto-brake when obstacles are detected.
*   Implement a 180° radar scan routine for obstacle avoidance and path planning.
*   Add a camera module for AI-based environment perception and image processing.
