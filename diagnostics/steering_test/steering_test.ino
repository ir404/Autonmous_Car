/*
  Diagnostics: Steering Servo Test

  File: diagnostics/steering_test.ino
  
  Description: Isolated test script to verify front wheel steering geometry and movement.
*/

#include <ESP32Servo.h>

#define SERVO_PIN 13 // Steering servo pin

Servo steeringServo;

void setup() {
    Serial.begin(115200);
    delay(1000); // Allow time for Serial Monitor connection

    Serial.println("\n=== Steering Servo Diagnostic Starting ===");

    // Reserve hardware timers for ESP32Servo
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);

    // Attach servo with standard 500us - 2500us pulse width bounds
    steeringServo.attach(SERVO_PIN, 500, 2500);

    // Initialise to centre position
    Serial.println("State: Centring (90 deg)");
    steeringServo.write(90);
    delay(2000);
}

void loop() {
    // 1. Steer Left
    Serial.println("State: Steering Left (45 deg)");
    steeringServo.write(45);
    delay(1500);

    // 2. Return to Centre
    Serial.println("State: Returning to Centre (90 deg)");
    steeringServo.write(90);
    delay(1500);

    // 3. Steer Right
    Serial.println("State: Steering Right (135 deg)");
    steeringServo.write(135);
    delay(1500);

    // 4. Return to Centre
    Serial.println("State: Returning to Centre (90 deg)");
    steeringServo.write(90);
    delay(1500);
}