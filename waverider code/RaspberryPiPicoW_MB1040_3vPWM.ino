// Raspberry Pi Pico W + MaxBotix MB1040
// Arduino framework
//
// MB1040:
//   V+  -> Pico 3V3
//   GND -> Pico GND
//   PW  -> Pico GPIO 0

const int PW_PIN = 0;

void setup() {
  Serial.begin(115200);

  pinMode(PW_PIN, INPUT);

  // Give the sensor time to start up
  delay(500);

  Serial.println("MB1040 Ultrasonic Sensor");
  Serial.println("-----------------------");
}

void loop() {
  // Wait for the HIGH pulse from the MB1040.
  // pulseIn() returns the pulse width in microseconds.
  unsigned long pulseWidth = pulseIn(PW_PIN, HIGH, 100000);

  if (pulseWidth == 0) {
    Serial.println("No measurement received");
  } else {
    // MB1040 PW output:
    // approximately 147 microseconds per inch
    float distanceInches = pulseWidth / 147.0;

    float distanceCm = distanceInches * 2.54;

    Serial.print("Pulse: ");
    Serial.print(pulseWidth);
    Serial.print(" us   Distance: ");
    Serial.print(distanceInches, 1);
    Serial.print(" in   ");
    Serial.print(distanceCm, 1);
    Serial.println(" cm");
  }

  // MB1040 operates at approximately 20 readings/second.
  delay(500);
}