# Development Timeline

This document tracks our team's weekly progress, hardware learnings, and software milestones during the SmartDrive project. Sessions are held every Thursday.

### Session 1: 15 October 2026 - Hardware Fundamentals
-   **Introduction:** Received the project overview and goals.
-   **Drivetrain Basics:** Learned about TT motors and why they are necessary for providing torque to the wheels.
-   **Microcontroller:** Explored the ESP32 development board and its various pinouts (GPIO, PWM capabilities, etc.).
-   **Power Management:** Discussed the motor driver's role in protecting the microcontroller and providing sufficient current to the motors.
-   **Power Supply:** Examined the two 3.7V batteries and learned how they are wired together to supply the required voltage for the car's electronics.

### Session 2: 22 October 2026 - Assembly & Basic Movement
-   **Hardware Assembly:** Received the 3D-printed chassis. Assembled the ESP32 board, batteries, power switch, motor driver, TT motors, and rear wheels onto the frame.
-   **Motor Programming:** Wrote our first C++ code to drive the TT motors forward and backward.
-   **Steering Logic:** Learned the principles of servo motors and Pulse Width Modulation (PWM). Programmed a basic test sequence for the front-wheel steering.
-   **Sensors:** Received an introduction to the Ultrasonic sensor (HC-SR04) and the physics of how it calculates distance. 
-   *Note: Ultrasonic sensor physical connection and code implementation are pending.*

### Session 3: 29 October 2026 - IoT & Cloud Integration
-   **Steering Fixes:** Debugged and successfully finalised the steering code to work cleanly alongside the motor controls.
-   **Wi-Fi Integration:** Learned how to utilize the ESP32's onboard Wi-Fi module for remote telemetry.
-   **Blynk Server Setup:** Created a project on the Blynk IoT server, generated our auth tokens, and configured our virtual datastreams.
-   **Mobile Interface:** Built a basic User Interface in the Blynk mobile app (buttons for forward/reverse, a switch for power, and a slider for speed). 
-   **Milestone:** Successfully connected the car to the mobile app hotspot and remote-controlled the steering and drive motors over the cloud!

### Session 4: 05 November 2026 - Advanced Control & Modular Diagnostics
-   **Code Architecture & State Management:** Refactored Blynk datastream variables using the `_DS` suffix for consistency. Added a dedicated Auto/Manual mode toggle switch (`autoMode_DS` on V6) with safety stops to isolate control loops.
-   **Joystick Control & Deadzones:** Integrated Blynk joystick inputs (X/Y axes) to drive dynamic motor speed and front-wheel steering angle (45° to 135°), establishing a central X-axis deadzone (450–550) to keep steering straight when idle.
-   **Steering Smoothing:** Implemented a slew rate limiter (`STEERING_STEP`) inside the main loop to incrementally step the servo angle, eliminating physical wheel jitter.
-   **Hardware Preparation:** Defined `SWEEP_SERVO_PIN` (GPIO 4) and initialised the `sweepServo` object in setup to prepare for the upcoming sensor radar integration (next session).
-   **Isolated Diagnostic Suite:** Created a structured diagnostic directory with standalone test sketches for individual component verification:
    - [Isolated Steering Test](diagnostics/steering_test/steering_test.ino)
    - [Standalone Ultrasonic Test](diagnostics/ultrasonic_sensor_test/ultrasonic_sensor_test.ino)
    - [Obstacle Auto-Stop Test](diagnostics/obstacle_stop_test/obstacle_stop_test.ino)
    - [Radar Sweep & Autonomous Steering Test](diagnostics/radar_steering_test/radar_steering_test.ino)
    - [Blynk Joystick Teleoperation Test](diagnostics/joystick_test/joystick_test.ino)