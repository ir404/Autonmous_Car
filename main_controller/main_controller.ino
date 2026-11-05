/*
 * Project: SmartDrive – Autonomous Vehicle Project
 * File: main_controller.ino
 * 
 * Description: 
 * Master control firmware for an ESP32-based robotic vehicle. 
 * Handles IoT cloud connectivity via Blynk, featuring a dual-state 
 * architecture for manual remote teleoperation and autonomous 
 * obstacle avoidance (sensor sweep integration pending).
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

const int MOTOR_SPEED_VALUE = 200;          // PWM value: 0–255
const int STEERING_STEP = 2;                // Degrees to step per loop cycle (steering speed)

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
int autoMode_DS = 0;                        // a button (V6)

// Steering smoothing variables
int currentAngle = 90;                      // Tracks the physical servo position

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
    
    // Put the car in idle (i.e. motor off, steering centred)
    turnOffMotor();
    steeringServo.write(90);                // starting angle of 90 degrees (centre)
    sweepServo.write(90);                   // starting sweep angle of 90 degrees (centre)
}

// Receive virtual pin states from Blynk server.
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

// car halts
void turnOffMotor() {
    digitalWrite(MOTOR_FORWARD, LOW);
    digitalWrite(MOTOR_REVERSE, LOW);
    ledcWrite(MOTOR_SPEED, 0);              // ensure PWM signal is fully disabled
}

// car drives forward
void goFwd() {
    digitalWrite(MOTOR_FORWARD, HIGH);
    digitalWrite(MOTOR_REVERSE, LOW);
    ledcWrite(MOTOR_SPEED, carSpeed_DS);
}

// car reverses
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

    // ------------------- Distance Measurement -------------------
    // TODO!
    // ------------------------------------------------------------

    if (carOn_DS == 0) {                       // car is turned off
        turnOffMotor();
        currentAngle = 90;                  // keep internal tracker aligned with physical state
        steeringServo.write(90);            // return steering to centre
    } 
    else {                                  // car is turned on

        if (autoMode_DS == 1) {     // Auto mode
            // temporary safety stop
            turnOffMotor();
            // TODO: implement radar sweep and path decision logic
        }
        else {                      // Manual remote control
            // ----- forward/reverse movement logic -----
            if ((btnFwd_DS == 1 && btnBack_DS == 0) || (carY_DS >= 600)) {
                goFwd();
            } 
            else if ((btnFwd_DS == 0 && btnBack_DS == 1) || (carY_DS <= 400)) {
                reverse();
            } 
            else {
                turnOffMotor();
            }

            // ----- steering control -----
            int targetAngle;
            if (carX_DS >= 450 && carX_DS <= 550) {
                targetAngle = 90;                                   // deadzone - return steering to centre
            } 
            else {
                targetAngle = map(carX_DS, 0, 1023, 45, 135);       // map active steering (45 left, 135 right)
            }
            // Smoothly step the physical angle towards the target angle
            if (currentAngle < targetAngle) {
                currentAngle += STEERING_STEP;
                if (currentAngle > targetAngle) currentAngle = targetAngle;     // prevent overshooting
            } 
            else if (currentAngle > targetAngle) {
                currentAngle -= STEERING_STEP;
                if (currentAngle < targetAngle) currentAngle = targetAngle;     // prevent overshooting
            }
            steeringServo.write(currentAngle);          // write the incrementally smoothed angle to the servo
        }
    }
}
