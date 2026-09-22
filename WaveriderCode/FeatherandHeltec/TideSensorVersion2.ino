#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_NeoPixel.h>

#define NEOPIXEL_I2C_POWER 2
#define uS_TO_S 1000000ULL /* Conversion factor for microseconds to seconds */

#define READINGS 10    // Number of Maxbotix readings to average
#define SLEEPTIME 900  // Sleeptime in seconds - every 15 minutes
#define TESTTIME 60    // Test mode every 60 seconds (not including reboot time)

#define BME_SCK 13  // for BME 280
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define PWMAXBOTIX_PIN 14
#define MEASUREDELAY 1 // in seconds.  Time delay between series of readings for averaging

float temp, press, hum, distance[READINGS], distanceavg, distancesd;
int i; 
long pulsewidth;
float distanceInches;
float distanceCm;

Adafruit_BME280 bme;

void waitAsecond(float wait = 1) {
  int time;
  time = wait * 1000; //convert to msec
  delay(time);
}

void setup() {

  pinMode(NEOPIXEL_I2C_POWER, OUTPUT);  // Set pin to turn on and off on-board LED and I2C power - from Adafruit script
  waitAsecond(); 
 
  pinMode(PWMAXBOTIX_PIN, INPUT); // sensor input

  digitalWrite(NEOPIXEL_I2C_POWER, HIGH);  // Turn on power to peripherals
  waitAsecond(10);  // wait for Heltec, bme280, and Maxbotix sensor to wake up
  
  Serial.begin(9600);  //for computer monitoring during test mode

  Serial1.begin(9600);  // Communicate with Heltec.  Use pins 7 and 8 on Feather and GPIO 43 and 44 on Heltec
  waitAsecond(10);  // Wait for Heltec to communicate with Feather through Serial1

  // STEMMA QT setup using standard pins for I2C
  Wire.begin();  // SDA, SCL for Feather
  waitAsecond();
  bool bmestatus = bme.begin();  // Start up BME280 on default I2C address
  waitAsecond(5);               // wait 5 sec to start up

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
  
  //Now gather and send Maxbotix sensor data
  Serial1.print("Distances (cm):  ");

  distanceavg=0.0;
  for(i=0;i<READINGS;i++)
   {
      pulsewidth = pulseIn(PWMAXBOTIX_PIN, HIGH, 100000);
      distance[i] = pulsewidth*6.803e-03*2.54; //1/147 * 2.54 for cm
      distanceavg += distance[i];

      waitAsecond(MEASUREDELAY);

      Serial1.print(i);
      Serial1.print(" ");
      Serial1.print(round(distance[i]));
      Serial1.print(", ");
   }
  distanceavg=distanceavg/READINGS;

  Serial1.println(" ");

  distancesd=0.0;
  for(i=0;i<READINGS;i++)
   {
      distancesd += (distance[i]-distanceavg)*(distance[i]-distanceavg); //variance calc
   }
  distancesd=sqrt(distancesd/READINGS);

  Serial1.print("Average: ");
  Serial1.print(round(distanceavg));
  Serial1.print(" cm, SD: ");
  Serial1.print(round(distancesd));
  Serial1.println(" cm");
  
  Serial.print("Average: ");
  Serial.print(round(distanceavg));
  Serial.print(" cm, SD: ");
  Serial.print(round(distancesd));
  Serial.println(" cm");
  
  waitAsecond(10); // Wait for Heltec to communicate data

  powerdown();  // turn off to save power until next reading
}

void powerdown() {

  // Now go to deep sleep
  // sleepinterval  (in s) is defined above with operate or test
  Serial.println("Powering down");
  Serial.println("Turning off neopixel and I2C");
  digitalWrite(NEOPIXEL_I2C_POWER, LOW);
  Serial.println("Going to sleep for about 15 min");
  waitAsecond();
  esp_sleep_enable_timer_wakeup(SLEEPTIME * uS_TO_S);  // SLEEPTIME in sec.
  esp_deep_sleep_start();
}
