/*
 * Project: SmartDrive – Autonomous Vehicle Project
 * File: main_controller.ino
 * 
 * Description: 
 * Master control firmware for an ESP32-based robotic vehicle. 
 * Handles IoT cloud connectivity via Blynk, featuring a dual-state 
 * architecture for manual remote teleoperation and stop-and-scan 
 * autonomous obstacle avoidance with stall-detection memory.
 */

// Credentials to connect to the BLYNK cloud server
#define BLYNK_TEMPLATE_ID   ""
#define BLYNK_TEMPLATE_NAME ""
#define BLYNK_AUTH_TOKEN    ""

// Libraries
#include <ESP32Servo.h>                     // version used: 3.0.5
#include <BlynkSimpleEsp32.h>               // version used: 1.4.0

// Pin connections
#define MOTOR_FORWARD       18
#define MOTOR_REVERSE       19
#define MOTOR_SPEED          5              // PWM pin for speed control (ENA)
#define SERVO_PIN           13              // Servo to steer the front wheels
#define SWEEP_SERVO_PIN      4              // Servo for ultrasonic sensor sweep
#define TRIG_PIN            12              // Ultrasonic sensor - trigger 
#define ECHO_PIN            14              // Ultrasonic sensor - echo

Servo steeringServo;
Servo sweepServo;                           // Object for the sweeping ultrasonic sensor servo

// General and manual control constants
const int SERVO_CENTRE           = 90;      // Neutral centre position for both servos
const int STEERING_STEP          = 2;       // Degrees per loop cycle for manual smoothing
const int MANUAL_STEER_LEFT      = 45;      // Max left steering angle in manual mode
const int MANUAL_STEER_RIGHT     = 135;     // Max right steering angle in manual mode

// Autonomous drive and recovery constants
const int AUTO_MOTOR_SPEED       = 140;     // Controlled autonomous speed (0–255)
const float MIN_SAFE_DISTANCE    = 20.0;    // Forward stopping threshold in cm
const float CRITICAL_DISTANCE    = 10.0;    // Reverse if max reading < 10 cm
const float MIN_CORNER_CLEARANCE = 6.0;     // Reverse if any angle < 6 cm (too tight to pivot)
const float STALL_MARGIN_CM      = 3.0;     // Max difference between scans to count as stalled
const int REVERSE_DURATION_MS    = 750;     // Reverse pulse duration in ms
const int TURN_PULSE_MS          = 350;     // Forward turn pulse duration after scanning in ms
const int MAX_REVERSE_ATTEMPTS   = 3;       // Safety cap to prevent endless reversing

// Ultrasonic range constants
const float MAX_SENSOR_RANGE     = 200.0;   // Maximum reliable echo distance in cm
const float TIMEOUT_DISTANCE     = 150.0;   // Fallback value when no echo returns

// Autonomous sweep configuration (±30 deg around 90 deg centre)
const int AUTO_STEER_LEFT        = 60;      // Max left steering angle in auto mode
const int AUTO_STEER_RIGHT       = 120;     // Max right steering angle in auto mode
const int SWEEP_MIN_ANGLE        = 60;      // Rightmost sweep limit (30 deg right of centre)
const int SWEEP_MAX_ANGLE        = 120;     // Leftmost sweep limit (30 deg left of centre)
const int SWEEP_STEP             = 10;      // Degrees between readings (60, 70, 80, 90, 100, 110, 120)
const int NUM_SWEEP_STEPS        = ((SWEEP_MAX_ANGLE - SWEEP_MIN_ANGLE) / SWEEP_STEP) + 1;

// Wi-Fi Credentials (e.g. mobile hotspot)
char ssid[] = "";               // hotspot name
char pass[] = "";               // hotspot password

// Blynk datastreams (DS)
int carOn_DS    = 0;                        // a switch (V0)
int btnFwd_DS   = 0;                        // a button (V1)
int btnBack_DS  = 0;                        // a button (V2)
int carSpeed_DS = 200;                      // a slider (V3), scale 0 to 255
int carX_DS     = 512;                      // joystick X-axis (V4)
int carY_DS     = 512;                      // joystick Y-axis (V5)
int autoMode_DS = 0;                        // a switch/button (V6)

// State and scan-memory variables
int currentAngle = SERVO_CENTRE;            // Tracks physical steering servo position
bool wasAutoMode = false;                   // Tracks mode transitions for clean resets
float distances[NUM_SWEEP_STEPS];           // Current autonomous scan readings
float prevDistances[NUM_SWEEP_STEPS];       // Previous scan readings for stall comparison
bool hasPrevScan = false;                   // Tracks whether prevDistances holds valid data

void setup() {
    // Initialise Blynk connection
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    // Configure PWM for motor speed control
    ledcAttach(MOTOR_SPEED, 1000, 8);        // 1 kHz, 8-bit resolution 

    // Configure motor control pins
    pinMode(MOTOR_FORWARD, OUTPUT);
    pinMode(MOTOR_REVERSE, OUTPUT);

    // Configure ultrasonic sensor pins
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    // Init servos
    steeringServo.attach(SERVO_PIN, 500, 2500);
    sweepServo.attach(SWEEP_SERVO_PIN, 500, 2500);
    
    // Put the car in idle (motor off, servos centred)
    turnOffMotor();
    steeringServo.write(SERVO_CENTRE);
    sweepServo.write(SERVO_CENTRE);
}

// Receive virtual pin states from Blynk server
BLYNK_WRITE(V0) {
    carOn_DS = param.asInt();
}
BLYNK_WRITE(V1) {
    btnFwd_DS = param.asInt();
}
BLYNK_WRITE(V2) {
    btnBack_DS = param.asInt();
}
BLYNK_WRITE(V3) {
    carSpeed_DS = param.asInt();
}
BLYNK_WRITE(V4) {
    carX_DS = param.asInt();
}
BLYNK_WRITE(V5) {
    carY_DS = param.asInt();
}
BLYNK_WRITE(V6) {
    autoMode_DS = param.asInt();
}

// Halt drive motors
void turnOffMotor() {
    digitalWrite(MOTOR_FORWARD, LOW);
    digitalWrite(MOTOR_REVERSE, LOW);
    ledcWrite(MOTOR_SPEED, 0);              // ensure PWM signal is fully disabled
}

// Drive forward at specified PWM speed
void goFwd(int speed) {
    digitalWrite(MOTOR_FORWARD, HIGH);
    digitalWrite(MOTOR_REVERSE, LOW);
    ledcWrite(MOTOR_SPEED, speed);
}

// Reverse at specified PWM speed
void reverse(int speed) {
    digitalWrite(MOTOR_FORWARD, LOW);
    digitalWrite(MOTOR_REVERSE, HIGH);
    ledcWrite(MOTOR_SPEED, speed);
}

// Keeps Blynk connection alive during autonomous waits and allows instant manual override
bool waitWithBlynk(unsigned long durationMs) {
    unsigned long start = millis();
    while (millis() - start < durationMs) {
        Blynk.run();
        if (carOn_DS == 0 || autoMode_DS == 0) {
            turnOffMotor();
            return false;                   // Abort immediately if switched off or to manual
        }
        delay(5);
    }
    return true;
}

// Measure ultrasonic distance in centimetres with timeout clamping
float getDistanceCM() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 30000);
    if (duration == 0) return TIMEOUT_DISTANCE;

    float distance = (duration * 0.0343) / 2.0;
    return (distance > MAX_SENSOR_RANGE) ? MAX_SENSOR_RANGE : distance;
}

// Check if current scan matches previous scan within margin (car failed to advance)
bool isStalled() {
    if (!hasPrevScan) return false;

    for (int i = 0; i < NUM_SWEEP_STEPS; i++) {
        if (fabs(distances[i] - prevDistances[i]) > STALL_MARGIN_CM) {
            return false;                   // Significant change detected; car moved
        }
    }
    return true;                            // All angles nearly identical; car is stuck
}

// Store current scan into memory for the next comparison
void saveScanHistory() {
    for (int i = 0; i < NUM_SWEEP_STEPS; i++) {
        prevDistances[i] = distances[i];
    }
    hasPrevScan = true;
}

// Reverse while steering towards the obstacle so the nose swings away from it
bool counterSteerReverse(int closestSweepAngle) {
    int reverseSteer = map(closestSweepAngle, SWEEP_MIN_ANGLE, SWEEP_MAX_ANGLE, AUTO_STEER_RIGHT, AUTO_STEER_LEFT);
    reverseSteer = constrain(reverseSteer, AUTO_STEER_LEFT, AUTO_STEER_RIGHT);

    currentAngle = reverseSteer;
    steeringServo.write(currentAngle);
    if (!waitWithBlynk(100)) return false;

    reverse(AUTO_MOTOR_SPEED);
    if (!waitWithBlynk(REVERSE_DURATION_MS)) return false;

    turnOffMotor();
    if (!waitWithBlynk(150)) return false;

    hasPrevScan = false;                    // Clear stale history after repositioning
    return true;
}

// Sweep 60–120 deg arc; reverse if boxed in, too close to pivot, or stalled
bool scanAndSteer() {
    bool pathFound = false;
    int reverseAttempts = 0;

    while (!pathFound) {
        // Move sensor to rightmost start limit (60 deg) and allow time to settle
        sweepServo.write(SWEEP_MIN_ANGLE);
        if (!waitWithBlynk(150)) return false;

        // Sweep from 60 to 120 deg in 10 deg steps
        for (int angle = SWEEP_MIN_ANGLE, index = 0; angle <= SWEEP_MAX_ANGLE; angle += SWEEP_STEP, index++) {
            sweepServo.write(angle);
            if (!waitWithBlynk(80)) return false;
            distances[index] = getDistanceCM();
        }

        // Find both the widest gap and the closest obstacle across the sweep
        float maxDistance = 0.0;
        float minDistance = MAX_SENSOR_RANGE;
        int bestSweepAngle = SERVO_CENTRE;
        int closestSweepAngle = SERVO_CENTRE;

        for (int i = 0; i < NUM_SWEEP_STEPS; i++) {
            int angle = SWEEP_MIN_ANGLE + (i * SWEEP_STEP);

            if (distances[i] > maxDistance) {
                maxDistance = distances[i];
                bestSweepAngle = angle;
            }
            if (distances[i] < minDistance) {
                minDistance = distances[i];
                closestSweepAngle = angle;
            }
        }

        bool stalled = isStalled();

        // Reverse if boxed in, corner is too close to pivot, or scan matches last position
        if (maxDistance < CRITICAL_DISTANCE || minDistance < MIN_CORNER_CLEARANCE || stalled) {
            if (reverseAttempts >= MAX_REVERSE_ATTEMPTS) {
                currentAngle = SERVO_CENTRE;
                steeringServo.write(currentAngle);
                break;
            }

            if (!counterSteerReverse(closestSweepAngle)) return false;
            reverseAttempts++;
        }
        // Valid path found and car is not stalled -> save scan and steer towards widest gap
        else {
            saveScanHistory();

            int mappedAngle = map(bestSweepAngle, SWEEP_MIN_ANGLE, SWEEP_MAX_ANGLE, AUTO_STEER_RIGHT, AUTO_STEER_LEFT);
            currentAngle = constrain(mappedAngle, AUTO_STEER_LEFT, AUTO_STEER_RIGHT);
            steeringServo.write(currentAngle);
            pathFound = true;
        }
    }

    // Return ultrasonic sensor to centre before resuming
    sweepServo.write(SERVO_CENTRE);
    if (!waitWithBlynk(150)) return false;

    return pathFound;
}

// Execute autonomous stop-and-scan navigation
void runAutonomousMode() {
    // Centre servos cleanly when first switching into auto mode
    if (!wasAutoMode) {
        currentAngle = SERVO_CENTRE;
        steeringServo.write(currentAngle);
        sweepServo.write(SERVO_CENTRE);
        hasPrevScan = false;
        wasAutoMode = true;
    }

    float frontDistance = getDistanceCM();

    if (frontDistance > 0 && frontDistance <= MIN_SAFE_DISTANCE) {
        turnOffMotor();

        bool clearPathAvailable = scanAndSteer();

        if (clearPathAvailable) {
            goFwd(AUTO_MOTOR_SPEED);
            waitWithBlynk(TURN_PULSE_MS);
        }
    } 
    else {
        hasPrevScan = false;
        goFwd(AUTO_MOTOR_SPEED);
    }
}

// Execute manual remote control teleoperation
void runManualMode() {
    // Reset sweep sensor and scan memory when switching back from auto mode
    if (wasAutoMode) {
        sweepServo.write(SERVO_CENTRE);
        hasPrevScan = false;
        wasAutoMode = false;
    }

    // ----- forward/reverse movement logic -----
    if ((btnFwd_DS == 1 && btnBack_DS == 0) || (carY_DS >= 600)) {
        goFwd(carSpeed_DS);
    } 
    else if ((btnFwd_DS == 0 && btnBack_DS == 1) || (carY_DS <= 400)) {
        reverse(carSpeed_DS);
    } 
    else {
        turnOffMotor();
    }

    // ----- steering control -----
    int targetAngle;
    if (carX_DS >= 450 && carX_DS <= 550) {
        targetAngle = SERVO_CENTRE;                         // deadzone - return steering to centre
    } 
    else {
        targetAngle = map(carX_DS, 0, 1023, MANUAL_STEER_LEFT, MANUAL_STEER_RIGHT);
    }

    // Smoothly step the physical angle towards the target angle
    if (currentAngle < targetAngle) {
        currentAngle += STEERING_STEP;
        if (currentAngle > targetAngle) currentAngle = targetAngle;
    } 
    else if (currentAngle > targetAngle) {
        currentAngle -= STEERING_STEP;
        if (currentAngle < targetAngle) currentAngle = targetAngle;
    }
    steeringServo.write(currentAngle);
}

void loop() {
    // Wait 5ms before the next cycle (needed for smooth steering)
    delay(5);

    // Get incoming cloud messages and maintain the connection
    Blynk.run();

    if (carOn_DS == 0) {                    // Car is turned off
        turnOffMotor();
        currentAngle = SERVO_CENTRE;
        steeringServo.write(SERVO_CENTRE);
        sweepServo.write(SERVO_CENTRE);
        hasPrevScan = false;
        wasAutoMode = false;
    } 
    else {                                  // Car is turned on
        if (autoMode_DS == 1) {             // Auto mode
            runAutonomousMode();
        }
        else {                              // Manual remote control
            runManualMode();
        }
    }
}