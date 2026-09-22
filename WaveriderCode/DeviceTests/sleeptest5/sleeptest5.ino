#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_NeoPixel.h>

#define NEOPIXEL_I2C_POWER 2
#define uS_TO_S 1000000ULL /* Conversion factor for microseconds to seconds */

#define READINGS 10    // Number of readings to average
#define SLEEPTIME 600  // Sleeptime in seconds - every 10 minutes
#define TESTTIME 60    // Test mode every 60 seconds (not including reboot time)

#define BME_SCK 13  // for BME 280
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10
#define WAKEUPTIME 30000  // 30 sec

#define PWMAXBOTIX_PIN 14

uint32_t Distancedelaytime = 1;  // in seconds.  Time delay between series of readings for averaging
uint64_t sleepinterval = 15;     // amount of time feather goes into sleep mode in sec.
//bool testmode; // Set to true if you want to see output on serial monitor on computer for testing, otherwise false
float temp = 0, press = 0, hum = 0, distance, distavg;
long pulseWidth;
float distanceInches;
float distanceCm;

Adafruit_BME280 bme;

void waitAsecond(float wait = 1) {
  int time;
  time = wait * 1000;
  delay(time);
}

void setup() {

  pinMode(NEOPIXEL_I2C_POWER, OUTPUT);  // Set pin to turn on and off on-board LED and I2C power - from Adafruit script
  waitAsecond(10);
  Serial.begin(9600);  //for computer monitoring during test mode

  Serial1.begin(9600);  // Communicate with Heltec.  Use pins 7 and 8 on Feather

  waitAsecond(15);

  // STEMMA QT setup using standard pins for I2C
  Wire.begin();  // SDA, SCL for Feather
  waitAsecond();

  pinMode(PWMAXBOTIX_PIN, INPUT);
  waitAsecond();

  digitalWrite(NEOPIXEL_I2C_POWER, HIGH);  // Turn on power to peripherals

  waitAsecond(10);  // wait for Heltec, bme280, and Maxbotix sensor to wake up

  bool bmestatus = bme.begin();  // Start up BME280 on default I2C address
  waitAsecond(10);               // wait 10 sec to start up

  Serial.println("BME280 startup on Feather ESP32 V2");
  if (!bmestatus) {
    Serial.println("Could not find BME280 sensor!");
  } else {
    Serial.println("BME280 initialized!");
  }
}

void loop() {

  temp = bme.readTemperature();
  press = bme.readPressure() / 100.0;  // hectapascals
  hum = bme.readHumidity();

  pulseWidth = pulseIn(PWMAXBOTIX_PIN, HIGH, 100000);

  // Send this info to computer serial monitor if it is in test mode


  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.println(" °C");

  Serial.print("Pressure: ");
  Serial.print(press);
  Serial.println(" hPa");

  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.println(" %");

  Serial.println("------------------");

  if (pulseWidth == 0) {
    Serial.println("No measurement received");
  } else {
    // MB1040 PW output:
    // approximately 147 microseconds per inch
    distanceInches = pulseWidth * 6.803e-03;  // 1/147
    distanceCm = distanceInches * 2.54;

    Serial.print("Pulse: ");
    Serial.print(pulseWidth);
    Serial.print(" us   Distance: ");
    Serial.print(distanceInches, 1);
    Serial.print(" in   ");
    Serial.print(distanceCm, 1);
    Serial.println(" cm");

    // Send this info to Heltec for Meshtastic text broadcast
    Serial1.println("Shark River Sensor 2");
    Serial1.print("Temp: ");
    Serial1.print(temp);
    Serial1.println(" °C; ");

    Serial1.print("Pressure: ");
    Serial1.print(press);
    Serial1.println(" hPa; ");

    Serial1.print("Humidity: ");
    Serial1.print(hum);
    Serial1.println(" %; ");

    Serial1.print("Distance: ");
    Serial1.print(round(distanceCm));
    Serial1.println(" cm");
  }

  waitAsecond(10);

  powerdown();  // turn off to save power until next reading
}


void powerdown() {

  // Now go to light sleep
  // sleepinterval  (in s) is defined above with operate or test
  Serial.println("Powering down");
  waitAsecond(5);
  Serial.println("Turning off neopixel and I2C");
  digitalWrite(NEOPIXEL_I2C_POWER, LOW);
  waitAsecond(5);
  Serial.println("Going to sleep for about 10 min");
  waitAsecond();
  esp_sleep_enable_timer_wakeup(SLEEPTIME * uS_TO_S);  // SLEEPTIME in sec.

  esp_deep_sleep_start();
}
