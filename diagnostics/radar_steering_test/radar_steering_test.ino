/*
  Diagnostics: Radar Sweep and Autonomous Steering
  
  File: diagnostics/radar_steering_test.ino

  Description: Sweeps a second servo from 0 to 180 degrees, capturing distances 
               every 15 degrees into an array. Automatically calculates the most 
               open path and steers the front wheels towards it.
*/

#include <ESP32Servo.h>

#define SERVO_PIN           13              // Front wheels steering servo
#define SWEEP_SERVO_PIN      4              // New servo for ultrasonic sensor
#define TRIG_PIN            12              // Ultrasonic sensor - trigger 
#define ECHO_PIN            14              // Ultrasonic sensor - echo

Servo steeringServo;
Servo sweepServo;

// Radar variables
const int SWEEP_STEP = 15;                  // Degrees between each measurement
float distances[13];                        // Array to hold 13 readings (0, 15, 30... 180)
int currentSweepAngle = 0;
int sweepDirection = 1;                     // 1 for sweeping up, -1 for sweeping down

const float MIN_SAFE_DISTANCE = 15.0;       // Minimum clearance threshold in cm

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n=== Radar & Steering Diagnostic Starting ===");

    // Configure sensor pins
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    // Initialise servos
    steeringServo.attach(SERVO_PIN, 500, 2500);
    sweepServo.attach(SWEEP_SERVO_PIN, 500, 2500);
    
    // Centre both servos at startup
    steeringServo.write(90);
    sweepServo.write(90);
    delay(1000);
}

// Function to measure distance
float getDistanceCM() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 30000);
    if (duration == 0) return 400.0;        // Return max range if no echo
    return (duration * 0.0343) / 2.0;
}

// Function to analyse array and steer
void evaluatePathAndSteer() {
    float maxDistance = 0.0;
    int bestSweepAngle = 90;                // Default to straight ahead

    // Find the angle with the maximum clear distance
    for (int i = 0; i <= 12; i++) {
        if (distances[i] > maxDistance) {
            maxDistance = distances[i];
            bestSweepAngle = i * SWEEP_STEP; // Convert array index back to degrees
        }
    }

    Serial.print("Sweep Complete. Best Path at Sweep Angle: ");
    Serial.print(bestSweepAngle);
    Serial.print(" deg (Clearance: ");
    Serial.print(maxDistance);
    Serial.println(" cm)");

    // Decision Logic
    if (maxDistance > MIN_SAFE_DISTANCE) {
        // Map the radar's best angle to the steering mechanism's limits (45 to 135)
        // Assuming Sweep 0 = Right and Sweep 180 = Left
        int steeringAngle = map(bestSweepAngle, 0, 180, 135, 45); 
        
        Serial.print("--> DECISION: Path clear. Steer wheels to ");
        Serial.println(steeringAngle);
        steeringServo.write(steeringAngle);
    } 
    else {
        Serial.println("--> DECISION: ALL PATHS BLOCKED (< 15cm). Keep wheels straight to halt/reverse.");
        steeringServo.write(90); 
    }
    Serial.println("--------------------------------------------------");
}

void loop() {
    // 1. Move sensor to current angle
    sweepServo.write(currentSweepAngle);
    
    // 2. Allow time for servo mechanical movement and sensor ping
    delay(50); 
    
    // 3. Take measurement and save to array index (e.g., angle 45 / 15 = index 3)
    int arrayIndex = currentSweepAngle / SWEEP_STEP;
    distances[arrayIndex] = getDistanceCM();

    // 4. Update the sweep angle for the next loop
    currentSweepAngle += (SWEEP_STEP * sweepDirection);

    // 5. Check if sweep hit the boundaries (0 or 180)
    if (currentSweepAngle >= 180 || currentSweepAngle <= 0) {
        // Ensure we capture the absolute edge (0 or 180) before evaluating
        sweepServo.write(currentSweepAngle);
        delay(50);
        distances[currentSweepAngle / SWEEP_STEP] = getDistanceCM();

        // Evaluate the full array and steer the front wheels
        evaluatePathAndSteer();

        // Reverse the sweep direction for the next pass
        sweepDirection *= -1; 
    }
}
