/*
CDL Tide Sensor
This script runs on an Adafruit Feather Huzzah ESP32 V2 and periodically records a series of distance
 measurements from a Maxbotix ultrasonic sensor and reads a BME280 T, P, humidity sensor.  It sends
 the data through a serial connection to a Heltec meshtastic device which in turn broadcasts the data
 as a text message.  The feather switches to a deep sleep mode after broadcasting the data in order
 to save energy.

Much of this code came from Tidesensor5.ino - see the comments there for the evolution of the tide sensor
hardware and script

The BME280 TPH sensor is connected to the feather through QWIC I2C connectors.  The Maxbotix and Heltec radio board
are also powered through the I2C connector and this power is turned off before the Feather goes to sleep.
Time delays are built in to ensure the messages get out before sleeping.  

The software also will send the data through a USB serial port to a computer to troubleshoot. 

Distance readings are sampled READINGS times at MEASUREDELAY intervals.  They are averaged and the standard deviation calculated.  
SLEEPTIME (in seconds) defines how long the feather sleeps before restarting this program from the beginning.

The raw data is also sent as text - a problem getting this data received has been fixed by accumulating all
of the info and sending it at once.  Previous versions sent the data out at 1 second intervals, allowing other 
meshtastic devices (like IXR) to stomp on the messages.

*/

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
  Serial.print(temp, 0);
  Serial.println(" °C");

  Serial.print("Pressure: ");
  Serial.print(press, 0);
  Serial.println(" hPa");

  Serial.print("Humidity: ");
  Serial.print(hum, 0);
  Serial.println(" %");

  Serial.println("------------------");


  //Now gather and send Maxbotix sensor data
  Serial.print("Distances (cm):  ");

  distanceavg=0.0;
  for(i=0;i<READINGS;i++)
   {
      pulsewidth = pulseIn(PWMAXBOTIX_PIN, HIGH, 100000);
      distance[i] = pulsewidth*6.803e-03*2.54; //1/147 * 2.54 for cm
      distanceavg += distance[i];

      waitAsecond(MEASUREDELAY);

      Serial.print(i);
      Serial.print(" ");
      Serial.print(round(distance[i]));
      Serial.print(", ");
   }
  distanceavg=distanceavg/READINGS;

  Serial.println(" ");

  distancesd=0.0;
  for(i=0;i<READINGS;i++)
   {
      distancesd += (distance[i]-distanceavg)*(distance[i]-distanceavg); //variance calc
   }
  distancesd=sqrt(distancesd/READINGS);

  Serial.print("Average Distance: ");
  Serial.print(round(distanceavg), 0);
  Serial.print(" cm, SD: ");
  Serial.print(round(distancesd), 0);
  Serial.println(" cm");

  // Send this info to Heltec for Meshtastic text broadcast
  Serial1.println("Shark River Sensor 2");
  Serial1.print("T: ");
  Serial1.print(temp, 1);
  Serial1.println(" °C");

  Serial1.print("Press.: ");
  Serial1.print(press, 0);
  Serial1.println(" hPa");

  Serial1.print("Hum.: ");
  Serial1.print(hum, 0);
  Serial1.println(" %");

  Serial1.print("Average Distance: ");
  Serial1.print(round(distanceavg), 0);
  Serial1.print(" cm, SD: ");
  Serial1.print(round(distancesd), 0);
  Serial1.println(" cm; ");

  for(i=0;i<READINGS;i++) // send raw data
   {

      Serial1.print(i);
      Serial1.print(" ");
      Serial1.print(round(distance[i]), 0);
      if(i!=READINGS-1)
       {
          Serial1.print(", ");
       }
   }
  Serial1.println(" ");

  waitAsecond(15); // Wait for Heltec to communicate data

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
