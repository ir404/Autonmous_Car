/*
    Diagnostics: Blynk Joystick Teleoperation Test
    File: diagnostics/joystick_test.ino
    Description: Isolated test script for wireless remote control via Blynk app.
                 Features master power switch (V0), speed control (V3), Y-axis drive, 
                 and smoothed X-axis steering.
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

Servo steeringServo;

const int STEERING_STEP = 2;                // Degrees to step per loop cycle (steering speed)

// Wi-Fi Credentials
char ssid[] = "";               // hotspot name
char pass[] = "";               // hotspot password

// Blynk datastreams 
int carOn_DS    = 0;                        // Power switch (V0)
int carSpeed_DS = 200;                      // Speed slider (V3), scale 0 to 255
int carX_DS     = 512;                      // Joystick X-axis (V4)
int carY_DS     = 512;                      // Joystick Y-axis (V5)

// Steering smoothing variable
int currentAngle = 90;                      // Tracks the physical servo position

void setup() {
    // Initialise Blynk connection
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    // Configure PWM for motor speed control
    ledcAttach(MOTOR_SPEED, 1000, 8);        // 1 kHz, 8-bit resolution 

    // Configure motor control pins
    pinMode(MOTOR_FORWARD, OUTPUT);
    pinMode(MOTOR_REVERSE, OUTPUT);

    // Init steering servo
    steeringServo.attach(SERVO_PIN, 500, 2500);
    
    // Put the car in idle
    turnOffMotor();
    steeringServo.write(90);                // starting angle of 90 degrees (centre)
}

// Receive virtual pin states from Blynk server
BLYNK_WRITE(V0) {
    carOn_DS = param.asInt();
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

// Car halts
void turnOffMotor() {
    digitalWrite(MOTOR_FORWARD, LOW);
    digitalWrite(MOTOR_REVERSE, LOW);
    ledcWrite(MOTOR_SPEED, 0);              // ensure PWM signal is fully disabled
}

// Car drives forward
void goFwd() {
    digitalWrite(MOTOR_FORWARD, HIGH);
    digitalWrite(MOTOR_REVERSE, LOW);
    ledcWrite(MOTOR_SPEED, carSpeed_DS);
}

// Car reverses
void reverse() {
    digitalWrite(MOTOR_FORWARD, LOW);
    digitalWrite(MOTOR_REVERSE, HIGH);
    ledcWrite(MOTOR_SPEED, carSpeed_DS);
}

void loop() {
    // Wait 5ms before the next cycle (needed for smooth steering)
    delay(5);

    // Get incoming cloud messages and maintain the connection
    Blynk.run();

    if (carOn_DS == 0) {                       // Car is turned off
        turnOffMotor();
        currentAngle = 90;                  // keep internal tracker aligned with physical state
        steeringServo.write(90);            // return steering to centre
    } 
    else {                                  // Car is turned on

        // ----- Joystick Movement Control (Y-Axis) -----
        if (carY_DS >= 600) {
            goFwd();
        } 
        else if (carY_DS <= 400) {
            reverse();
        } 
        else {
            turnOffMotor();
        }

        // ----- Joystick Steering Control (X-Axis) -----
        int targetAngle;
        if (carX_DS >= 450 && carX_DS <= 550) {
            targetAngle = 90;               // Deadzone: target straight ahead
        } 
        else {
            targetAngle = map(carX_DS, 0, 1023, 45, 135); // Map active steering (45 left, 135 right)
        }

        // Smoothly step the physical angle towards the target angle
        if (currentAngle < targetAngle) {
            currentAngle += STEERING_STEP;
            if (currentAngle > targetAngle) currentAngle = targetAngle; // Prevent overshooting
        } 
        else if (currentAngle > targetAngle) {
            currentAngle -= STEERING_STEP;
            if (currentAngle < targetAngle) currentAngle = targetAngle; // Prevent overshooting
        }
        
        steeringServo.write(currentAngle);  // Write the incrementally smoothed angle to the servo
    }
}