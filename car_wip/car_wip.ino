/*
 Autonmous Car project in progress
*/

// credentials needed to connect to the BLYNK cloud server
#define BLYNK_TEMPLATE_ID   "TMPL4KiGQUURr"
#define BLYNK_TEMPLATE_NAME "Autonomous Car"
#define BLYNK_AUTH_TOKEN    "gZRYmLaNVw15s1G7R_Es9JNCc5wzFa1b"


#include <ESP32Servo.h>
#include <BlynkSimpleEsp32.h>

#define MOTOR_FORWARD   18
#define MOTOR_REVERSE   19
#define MOTOR_SPEED      5                  // PWM pin for speed control (ENA)
#define SERVO_PIN       13                  // Servo for to steer the front wheels
#define TRIG_PIN        12                  // Ultrasonic sensor - trigger 
#define ECHO_PIN        14                  // Ultrasonic sensor - echo

Servo steeringServo;

const int MOTOR_SPEED_VALUE = 200;                  // PWM value: 0–255

// Wifi Credentials (mobile hotspot)
char ssid[] = "IR - Pixel 8";       // hotspot name
char pass[] = "abc123def456";       // hotspot password

int carOn = 0;

void setup() {
    // Initialise Blynk connection
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    // Configure motor control pins
    pinMode(MOTOR_FORWARD, OUTPUT);
    pinMode(MOTOR_REVERSE, OUTPUT);

    // Configure PWM for motor speed control
    // ledcAttach(MOTOR_SPEED, 1000, 8);                // 1 kHz, 8-bit resolution (leave this line commented out for now)

    // Init servo
    steeringServo.attach(SERVO_PIN, 500, 2500);
    steeringServo.write(90);                         // starting angle of 90 degree (center)

    // Stop the motor at startup
    digitalWrite(MOTOR_FORWARD, LOW);
    digitalWrite(MOTOR_REVERSE, LOW);
    ledcWrite(MOTOR_SPEED, 0);
}

// Receive virtual pin V0 state from Blynk app switch
BLYNK_WRITE(V0) {
    carOn = param.asInt();
}

void loop() {

    Blynk.run();

    // check value onOff datastream and set the car
    if (carOn == 0) {
        // turn off motors
        digitalWrite(MOTOR_FORWARD, 0);
        digitalWrite(MOTOR_REVERSE, 0);
        ledcWrite(MOTOR_SPEED, 0);
        // return steering to center
        steeringServo.write(90);
    } else {
        // --- SERVO TEST SEQUENCE ---
        // Steer Left (approx 45 degrees)
        steeringServo.write(45);
        delay(1000);    
        // Steer Center (90 degrees)
        steeringServo.write(90);
        delay(1000);
        // Steer Right (approx 135 degrees)
        steeringServo.write(135);
        delay(1000);
        
        // Return to Center before moving
        steeringServo.write(90);
        delay(1000);
        // ---------------------------
    }

    

    // Move the motor forward for 10 seconds
    // digitalWrite(MOTOR_FORWARD, HIGH);
    // digitalWrite(MOTOR_REVERSE, LOW);
    // ledcWrite(MOTOR_SPEED, MOTOR_SPEED_VALUE);
    // delay(10000);

    // // Move the motor in reverse for 3 seconds
    // digitalWrite(MOTOR_FORWARD, LOW);
    // digitalWrite(MOTOR_REVERSE, HIGH);
    // ledcWrite(MOTOR_SPEED, MOTOR_SPEED_VALUE);
    // delay(3000);

    // // Stop the motor
    // digitalWrite(MOTOR_FORWARD, LOW);
    // digitalWrite(MOTOR_REVERSE, LOW);
    // ledcWrite(MOTOR_SPEED, 0);

    // Wait 1 second before the next cycle
    delay(1000);
}