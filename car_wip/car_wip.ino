/*
    Project - Autonmous Car 
*/

// Credentials to connect to the BLYNK cloud server
#define BLYNK_TEMPLATE_ID   ""
#define BLYNK_TEMPLATE_NAME ""
#define BLYNK_AUTH_TOKEN    ""


// Libraries
#include <ESP32Servo.h>                     // version used: 3.0.5
#include <BlynkSimpleEsp32.h>               // version used: 1.4.0


#define MOTOR_FORWARD       18
#define MOTOR_REVERSE       19
#define MOTOR_SPEED          5              // PWM pin for speed control (ENA)
#define SERVO_PIN           13              // Servo for to steer the front wheels
#define TRIG_PIN            12              // Ultrasonic sensor - trigger 
#define ECHO_PIN            14              // Ultrasonic sensor - echo

Servo steeringServo;

const int MOTOR_SPEED_VALUE = 200;          // PWM value: 0–255

// Wifi Credentials (e.g. mobile hotspot)
char ssid[] = "";       // hotspot name
char pass[] = "";       // hotspot password

// Blynk datastreams 
int carOn = 0;                      // a switch (V0)
int moveFwd = 0;                    // a button (V1)
int moveBack = 0;                   // a button (V2)
int carSpeed = 200;                 // a slider (V3), scale 0 to 255
int testSteering = 0;               // a button (V4)


void setup() {
    // Initialise Blynk connection
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    // Configure PWM for motor speed control
    ledcAttach(MOTOR_SPEED, 1000, 8);                // 1 kHz, 8-bit resolution 

    // Configure motor control pins
    pinMode(MOTOR_FORWARD, OUTPUT);
    pinMode(MOTOR_REVERSE, OUTPUT);

    // Configure ultrasonic sensor pins
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    // Init servo
    steeringServo.attach(SERVO_PIN, 500, 2500);
    
    // Put the car in idle (i.e. motor's off, steering centered)
    turnOffMotor();                                 
    ledcWrite(MOTOR_SPEED, 0);                      // init speed to 0 (disables the PWM signal)
    steeringServo.write(90);                        // starting angle of 90 degree (center)
}

// Receive virtual pin states from Blynk server. 
// The pin ids (Vi) should match the datastreams configured in the server.
BLYNK_WRITE(V0) {
    carOn = param.asInt();
}
BLYNK_WRITE(V1) {
    moveFwd = param.asInt();
}
BLYNK_WRITE(V2) {
    moveBack = param.asInt();
}
BLYNK_WRITE(V3) {
    carSpeed = param.asInt();
}
BLYNK_WRITE(V4) {
    testSteering = param.asInt();
}

// car halts
void turnOffMotor() {
    digitalWrite(MOTOR_FORWARD, LOW);
    digitalWrite(MOTOR_REVERSE, LOW);
}
// car drives forward
void goFwd() {
    digitalWrite(MOTOR_FORWARD, HIGH);
    digitalWrite(MOTOR_REVERSE, LOW);
}
// car reverses
void reverse() {
    digitalWrite(MOTOR_FORWARD, LOW);
    digitalWrite(MOTOR_REVERSE, HIGH);
}


void loop() {
    // Get incoming cloud messages and maintain the connection
    Blynk.run();

    // ------------------- Distance Measurement -------------------
    // TODO!
    // ------------------------------------------------------------

    if (carOn == 0) {           // car's turned off
        turnOffMotor();
        steeringServo.write(90);            // return steering to center
        ledcWrite(MOTOR_SPEED, 0);          // ensure PWM signal is fully disabled
    } 
    else {                    // car's turned on
        
        // move the car based on button press
        if (moveFwd == 1 && moveBack == 0) {
            goFwd();
            ledcWrite(MOTOR_SPEED, carSpeed);
        } else if (moveFwd == 0 && moveBack == 1) {
            reverse();
            ledcWrite(MOTOR_SPEED, carSpeed);
        } else {
            turnOffMotor();
            ledcWrite(MOTOR_SPEED, 0);
        }

        // diagnostics (remove later? if so, don't forget to update the pins of the datastreams!)
        if (testSteering == 1) {
            // --- SERVO TEST SEQUENCE ---
            // Steer Left (approx 45 degrees)
            steeringServo.write(45);
            delay(100);    
            // Steer Center (90 degrees)
            steeringServo.write(90);
            delay(100);
            // Steer Right (approx 135 degrees)
            steeringServo.write(135);
            delay(100);
            
            // Return to Center before moving
            steeringServo.write(90);
            delay(100);
            // ---------------------------
        }
    }

    // Wait 10ms before the next cycle
    delay(10);
}
