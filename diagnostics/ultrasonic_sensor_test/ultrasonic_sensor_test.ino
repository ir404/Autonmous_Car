/*
  Diagnostics: Ultrasonic Sensor Test

  File: diagnostics/ultrasonic_test.ino
  
  Description: Isolated test script to verify HC-SR04 distance readings via Serial Monitor.
*/

#define TRIG_PIN 12                  // Ultrasonic sensor - trigger
#define ECHO_PIN 14                  // Ultrasonic sensor - echo

void setup() {
    Serial.begin(115200);
    delay(1000); // Allow time for Serial Monitor to establish connection

    Serial.println("\n=== Ultrasonic Sensor Diagnostic Starting ===");

    // Configure ultrasonic sensor pins
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
}

// Function to measure and calculate distance in centimetres
float getDistanceCM() {
    // Ensure trigger pin is LOW before pulse
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);

    // Trigger the sensor with a 10us HIGH pulse
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // Read echo pulse duration (30ms timeout prevents hanging if out of range)
    long duration = pulseIn(ECHO_PIN, HIGH, 30000);

    // Calculate distance (speed of sound is ~0.0343 cm/us, divided by 2 for round trip)
    if (duration == 0) {
        return -1.0; // Out of range or no echo received
    }
    return (duration * 0.0343) / 2.0;
}

void loop() {
    float distance = getDistanceCM();

    if (distance < 0) {
        Serial.println("Distance: Out of range / Timeout");
    } else {
        Serial.print("Distance: ");
        Serial.print(distance, 1);
        Serial.println(" cm");
    }

    // 250ms sampling rate for comfortable viewing in Serial Monitor
    delay(250); 
}