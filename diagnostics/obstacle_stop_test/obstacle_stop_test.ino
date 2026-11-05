/*
  Diagnostics: Obstacle Stop Test

  File: diagnostics/obstacle_stop_test.ino
  
  Description: Drives the car forward until an object is detected within 20 cm, 
               then automatically stops the motors. No Wi-Fi or Blynk required.
*/

#include <ESP32Servo.h>                     // Version 3.0.5

#define MOTOR_FORWARD       18
#define MOTOR_REVERSE       19
#define MOTOR_SPEED          5              // PWM pin for speed control (ENA)
#define SERVO_PIN           13              // Steering servo
#define TRIG_PIN            12              // Ultrasonic sensor - trigger
#define ECHO_PIN            14              // Ultrasonic sensor - echo

Servo steeringServo;

const int MOTOR_SPEED_VALUE = 200;          // Motor speed (0–255)
const float STOP_DISTANCE_CM = 10.0;        // Distance threshold to stop (in cm)

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n=== Obstacle Stop Diagnostic Starting ===");

    // Configure PWM for motor speed control
    ledcAttach(MOTOR_SPEED, 1000, 8);        // 1 kHz, 8-bit resolution

    // Configure motor control pins
    pinMode(MOTOR_FORWARD, OUTPUT);
    pinMode(MOTOR_REVERSE, OUTPUT);

    // Configure ultrasonic sensor pins
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    // Initialise steering to centre (90 degrees)
    steeringServo.attach(SERVO_PIN, 500, 2500);
    steeringServo.write(90);

    // Ensure motor is stopped at startup
    turnOffMotor();
}

// Function to measure and calculate distance in centimetres
float getDistanceCM() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // 30ms timeout prevents code from hanging if out of range
    long duration = pulseIn(ECHO_PIN, HIGH, 30000);

    if (duration == 0) {
        return 400.0; // Return maximum distance if no echo received
    }
    return (duration * 0.0343) / 2.0;
}

// Stop motor movement
void turnOffMotor() {
    digitalWrite(MOTOR_FORWARD, LOW);
    digitalWrite(MOTOR_REVERSE, LOW);
    ledcWrite(MOTOR_SPEED, 0);
}

// Drive car forward
void goFwd() {
    digitalWrite(MOTOR_FORWARD, HIGH);
    digitalWrite(MOTOR_REVERSE, LOW);
    ledcWrite(MOTOR_SPEED, MOTOR_SPEED_VALUE);
}

void loop() {
    float distance = getDistanceCM();

    Serial.print("Distance: ");
    Serial.print(distance, 1);
    Serial.print(" cm | Action: ");

    // Safety check: stop if an obstacle is within threshold
    if (distance > 0 && distance <= STOP_DISTANCE_CM) {
        turnOffMotor();
        Serial.println("OBSTACLE DETECTED -> STOPPING");
    } else {
        goFwd();
        Serial.println("CLEAR -> DRIVING FORWARD");
    }

    // 50ms sampling rate for sensor stability
    delay(50);
}