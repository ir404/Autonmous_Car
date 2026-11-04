# Development Timeline

This document tracks our team's weekly progress, hardware learnings, and software milestones during the SmartDrive project. Sessions are held every Thursday.

### Session 1: October 15, 2026 - Hardware Fundamentals
*   **Introduction:** Received the project overview and goals.
*   **Drivetrain Basics:** Learned about TT motors and why they are necessary for providing torque to the wheels.
*   **Microcontroller:** Explored the ESP32 development board and its various pinouts (GPIO, PWM capabilities, etc.).
*   **Power Management:** Discussed the motor driver's role in protecting the microcontroller and providing sufficient current to the motors.
*   **Power Supply:** Examined the two 3.7V batteries and learned how they are wired together to supply the required voltage for the car's electronics.

### Session 2: October 22, 2026 - Assembly & Basic Movement
*   **Hardware Assembly:** Received the 3D-printed chassis. Assembled the ESP32 board, batteries, power switch, motor driver, TT motors, and rear wheels onto the frame.
*   **Motor Programming:** Wrote our first C++ code to drive the TT motors forward and backward.
*   **Steering Logic:** Learned the principles of servo motors and Pulse Width Modulation (PWM). Programmed a basic test sequence for the front-wheel steering.
*   **Sensors:** Received an introduction to the Ultrasonic sensor (HC-SR04) and the physics of how it calculates distance. 
*   *Note: Ultrasonic sensor physical connection and code implementation are pending.*

### Session 3: October 29, 2026 - IoT & Cloud Integration
*   **Steering Fixes:** Debugged and successfully finalised the steering code to work cleanly alongside the motor controls.
*   **Wi-Fi Integration:** Learned how to utilize the ESP32's onboard Wi-Fi module for remote telemetry.
*   **Blynk Server Setup:** Created a project on the Blynk IoT server, generated our auth tokens, and configured our virtual datastreams.
*   **Mobile Interface:** Built a basic User Interface in the Blynk mobile app (buttons for forward/reverse, a switch for power, and a slider for speed). 
*   **Milestone:** Successfully connected the car to the mobile app hotspot and remote-controlled the steering and drive motors over the cloud!
