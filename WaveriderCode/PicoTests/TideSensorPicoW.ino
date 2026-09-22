// ---------------------------------------------------------------------------
// NewPing library sketch that interfaces with all but the SRF06 sensor using
// only one Arduino pin. You can also interface with the SRF06 using one pin
// if you install a 0.1uf capacitor on the trigger and echo pins of the sensor
// then tie the trigger pin to the Arduino pin (doesn't work with Teensy).
// ---------------------------------------------------------------------------

#include <NewPing.h>

#define PING_PIN  15  // Arduino pin tied to both trigger and echo pins on the ultrasonic sensor.
#define MAX_DISTANCE 3000 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

NewPing sonar(14, 15, MAX_DISTANCE); // NewPing setup of pin and maximum distance.

void setup() {
  Serial.begin(115200); // Open serial monitor at 115200 baud to see ping results.
}

void loop() {
  delay(500
  );                     // Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
  Serial.print("Ping: ");
  Serial.print(sonar.ping_cm()/2.54); // Send ping, get distance in cm and print result (0 = outside set distance range)
  Serial.println(" in");
}