/*
  Diagnostics: Stop-and-Scan Autonomous Obstacle Avoidance (with Scan Memory)
  
  File: diagnostics/radar_steering_test.ino

  Description: Drives forward while scanning straight ahead. When an obstacle is 
               detected within 20 cm, the car halts and sweeps a ±30 deg arc 
               (60–120 deg). Compares consecutive scans to detect physical stalls 
               and uses counter-steered reversing to swing the nose clear.
*/

#include <ESP32Servo.h>

// Pin definitions
#define MOTOR_FORWARD       18
#define MOTOR_REVERSE       19
#define MOTOR_SPEED          5              // PWM pin for speed control (ENA)
#define SERVO_PIN           13              // Front wheels steering servo
#define SWEEP_SERVO_PIN      4              // Sweeping ultrasonic sensor servo
#define TRIG_PIN            12              // Ultrasonic sensor - trigger 
#define ECHO_PIN            14              // Ultrasonic sensor - echo

Servo steeringServo;
Servo sweepServo;

// Drive and recovery constants
const int AUTO_MOTOR_SPEED       = 140;     // Controlled speed (0–255)
const float MIN_SAFE_DISTANCE    = 20.0;    // Forward stopping threshold in cm
const float CRITICAL_DISTANCE    = 10.0;    // Reverse if max reading < 10 cm
const float MIN_CORNER_CLEARANCE = 6.0;     // Reverse if any angle < 6 cm (too tight to pivot)
const float STALL_MARGIN_CM      = 3.0;     // Max difference between scans to count as stalled
const int REVERSE_DURATION_MS    = 500;     // Reverse pulse duration in ms (tune on floor)
const int MAX_REVERSE_ATTEMPTS   = 3;       // Safety cap to prevent endless reversing

// Ultrasonic range constants
const float MAX_SENSOR_RANGE     = 200.0;   // Maximum reliable echo distance in cm
const float TIMEOUT_DISTANCE     = 150.0;   // Fallback value when no echo returns

// Servo limits and sweep configuration (±30 deg around 90 deg centre)
const int SERVO_CENTRE           = 90;      // Neutral centre position for both servos
const int STEER_LEFT_LIMIT       = 60;      // Max left steering angle (30 deg left of centre)
const int STEER_RIGHT_LIMIT      = 120;     // Max right steering angle (30 deg right of centre)
const int SWEEP_MIN_ANGLE        = 60;      // Rightmost sweep limit (30 deg right of centre)
const int SWEEP_MAX_ANGLE        = 120;     // Leftmost sweep limit (30 deg left of centre)
const int SWEEP_STEP             = 10;      // Degrees between readings (60, 70, 80, 90, 100, 110, 120)
const int NUM_SWEEP_STEPS        = ((SWEEP_MAX_ANGLE - SWEEP_MIN_ANGLE) / SWEEP_STEP) + 1;

// Scan arrays and stall-detection memory
float distances[NUM_SWEEP_STEPS];           // Current scan readings
float prevDistances[NUM_SWEEP_STEPS];       // Previous scan readings for stall comparison
bool hasPrevScan = false;                   // Tracks whether prevDistances holds valid data

// Halt drive motors
void turnOffMotor() {
    digitalWrite(MOTOR_FORWARD, LOW);
    digitalWrite(MOTOR_REVERSE, LOW);
    ledcWrite(MOTOR_SPEED, 0);
}

// Drive forward at controlled speed
void goFwd() {
    digitalWrite(MOTOR_FORWARD, HIGH);
    digitalWrite(MOTOR_REVERSE, LOW);
    ledcWrite(MOTOR_SPEED, AUTO_MOTOR_SPEED);
}

// Reverse at controlled speed
void reverse() {
    digitalWrite(MOTOR_FORWARD, LOW);
    digitalWrite(MOTOR_REVERSE, HIGH);
    ledcWrite(MOTOR_SPEED, AUTO_MOTOR_SPEED);
}

// Measure distance in centimetres with timeout clamping
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
void counterSteerReverse(int closestSweepAngle) {
    int reverseSteer = map(closestSweepAngle, SWEEP_MIN_ANGLE, SWEEP_MAX_ANGLE, STEER_RIGHT_LIMIT, STEER_LEFT_LIMIT);
    reverseSteer = constrain(reverseSteer, STEER_LEFT_LIMIT, STEER_RIGHT_LIMIT);

    steeringServo.write(reverseSteer);
    delay(100);                             // Let steering servo reach angle before moving
    reverse();
    delay(REVERSE_DURATION_MS);
    turnOffMotor();
    delay(150);                             // Brief settle before next sweep

    hasPrevScan = false;                    // Clear stale history after repositioning
}

// Sweep 60–120 deg arc; reverse if boxed in, too close to pivot, or stalled
bool scanAndSteer() {
    bool pathFound = false;
    int reverseAttempts = 0;

    while (!pathFound) {
        // Move sensor to rightmost start limit (60 deg) and allow time to settle
        sweepServo.write(SWEEP_MIN_ANGLE);
        delay(150);

        // Sweep from 60 to 120 deg in 10 deg steps
        for (int angle = SWEEP_MIN_ANGLE, index = 0; angle <= SWEEP_MAX_ANGLE; angle += SWEEP_STEP, index++) {
            sweepServo.write(angle);
            delay(80);                      // Wait for servo movement and echo stability
            distances[index] = getDistanceCM();
        }

        // Find both the widest gap and the closest obstacle across the sweep
        float maxDistance = 0.0;
        float minDistance = MAX_SENSOR_RANGE;
        int bestSweepAngle = SERVO_CENTRE;
        int closestSweepAngle = SERVO_CENTRE;

        for (int i = 0; i < NUM_SWEEP_STEPS; i++) {
            int currentAngle = SWEEP_MIN_ANGLE + (i * SWEEP_STEP);

            if (distances[i] > maxDistance) {
                maxDistance = distances[i];
                bestSweepAngle = currentAngle;
            }
            if (distances[i] < minDistance) {
                minDistance = distances[i];
                closestSweepAngle = currentAngle;
            }
        }

        bool stalled = isStalled();

        Serial.print("Scan Complete | Max: ");
        Serial.print(maxDistance, 1);
        Serial.print(" cm (at ");
        Serial.print(bestSweepAngle);
        Serial.print(" deg) | Min: ");
        Serial.print(minDistance, 1);
        Serial.print(" cm (at ");
        Serial.print(closestSweepAngle);
        Serial.println(" deg)");

        // Reverse if boxed in, corner is too close to pivot, or scan matches last position
        if (maxDistance < CRITICAL_DISTANCE || minDistance < MIN_CORNER_CLEARANCE || stalled) {
            if (reverseAttempts >= MAX_REVERSE_ATTEMPTS) {
                Serial.println("--> Trapped after max reverse attempts. Remaining stopped.");
                steeringServo.write(SERVO_CENTRE);
                break;
            }

            if (stalled) {
                Serial.println("--> Stall detected (scan matches history). Counter-reversing...");
            } else if (minDistance < MIN_CORNER_CLEARANCE) {
                Serial.println("--> Corner too close (< 6 cm) to turn. Counter-reversing...");
            } else {
                Serial.println("--> All paths < 10 cm. Counter-reversing...");
            }

            counterSteerReverse(closestSweepAngle);
            reverseAttempts++;
        }
        // Valid path found and car is not stalled -> save scan and steer towards widest gap
        else {
            saveScanHistory();

            int mappedAngle = map(bestSweepAngle, SWEEP_MIN_ANGLE, SWEEP_MAX_ANGLE, STEER_RIGHT_LIMIT, STEER_LEFT_LIMIT);
            int steeringAngle = constrain(mappedAngle, STEER_LEFT_LIMIT, STEER_RIGHT_LIMIT);
            steeringServo.write(steeringAngle);
            pathFound = true;

            Serial.print("--> Steering front wheels to ");
            Serial.print(steeringAngle);
            Serial.println(" deg.");
        }
    }

    // Return ultrasonic sensor to centre before resuming
    sweepServo.write(SERVO_CENTRE);
    delay(150);
    Serial.println("--------------------------------------------------");

    return pathFound;
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n=== Stop-and-Scan Autonomous Diagnostic Starting ===");

    // Configure PWM and motor pins
    ledcAttach(MOTOR_SPEED, 1000, 8);        // 1 kHz, 8-bit resolution
    pinMode(MOTOR_FORWARD, OUTPUT);
    pinMode(MOTOR_REVERSE, OUTPUT);

    // Configure ultrasonic sensor pins
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    // Initialise and centre both servos
    steeringServo.attach(SERVO_PIN, 500, 2500);
    sweepServo.attach(SWEEP_SERVO_PIN, 500, 2500);
    steeringServo.write(SERVO_CENTRE);
    sweepServo.write(SERVO_CENTRE);

    turnOffMotor();
    delay(1000);
}

void loop() {
    // Continuously scan straight ahead
    float frontDistance = getDistanceCM();

    if (frontDistance > 0 && frontDistance <= MIN_SAFE_DISTANCE) {
        // 1. Stop immediately upon detecting an obstacle within 20 cm
        turnOffMotor();
        Serial.print("Obstacle at ");
        Serial.print(frontDistance, 1);
        Serial.println(" cm -> Halting to scan.");

        // 2. Sweep arc, check stall memory, steer, and re-centre sensor
        bool clearPathAvailable = scanAndSteer();

        // 3. Resume driving if a path was selected
        if (clearPathAvailable) {
            goFwd();
            delay(350);                     // Brief turn pulse so front sensor clears the obstacle
        }
    } 
    else {
        // Path ahead is clear; clear old obstacle memory and drive forward
        hasPrevScan = false;
        goFwd();
    }

    delay(50);
}