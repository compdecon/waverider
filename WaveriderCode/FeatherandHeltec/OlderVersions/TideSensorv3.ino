/*
Tide Sensor
This script runs on an Adafruit Feather Huzzah ESP32 and periodically records a distance
 measurement VL53L1X lidar sensor and a BME280 T, P, humidity sensor.  It sends
 the data through a serial connection to a meshtastic device which in turn broadcasts the data
 as a text message.  The feather switches to sleep mode after broadcasting the data in order
 to save energy - important when this prototype setup is deployed for the CDL Tide Sensor project.

The TPH and the distance sensors are connected to the feather through QWIC I2C connectors.  The
 software also will send the data through a USB serial port to a computer to troubleshoot.  To 
 enable the USB serial connection, set USBSerial to true.  Turn it off when not connected
 to a computer or the progam may not run.

Distance readings are sampled READINGS times and averaged.  SLEEPTIME in seconds defines how long the feather sleeps 
before restarting

Much of the code taken from Adafruit examples for these sensors and ESP32

GPW and MD, April 2026
Modified 6/24/26  to include a test mode.  A GPIO pin is checked to see if in test mode or operate mode 
  without having to modify the software.  The ESP32 will either sleep for 10 seconds or 5 minutes
  depening on whether GPIO13 sees a high or ground, respectively. 
  USBSerial is now defined by this GPIO as well.   if in test mode (high),  USBSerial = TRUE, meaning a 
  computer hooked up to the ESP32 USBC can read the data

Modified 6/26/26  to use M5Stack I2C ultrasonic sensor instead of the lidar sensor
  Having trouble getting the lidar to work in the pipe.  Try this sensor instead, which I bought from Digikey 
  in January 2026. I don't know if the BME280 will work now though. 
  Using Arduino_GroveI2C_Ultrasonic library

*/

#include <Wire.h>
//#include <SPI.h>
//#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "Arduino_GroveI2C_Ultrasonic.h"

#define SEALEVELPRESSURE_HPA 1013.25
#define uS_TO_S 1000000ULL /* Conversion factor for micro seconds to seconds */

#define READINGS 10 /* Number of readings to average */
#define SLEEPTIME 300 /* Sleeptime in seconds - every 5 minutes*/
#define TESTTIME 10   

#define BME_SCK 13 // for BME
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

uint32_t Distdelaytime=5000; // in milliseconds
int16_t distance;
uint64_t sleepinterval;
bool USBSerial; /* Set to true if you want to see output on computer, otherwise false */
float temp, press, hum, distavg;

Adafruit_BME280 bme;

Arduino_GroveI2C_Ultrasonic m5sensor;

void setup() {
  
  Serial.begin(9600);
  Serial1.begin(9600);
  delay(1000);

  // Use correct STEMMA QT pins
  Wire.begin(22, 20);  // SDA, SCL for Feather
  delay(1000);

  bool bmestatus = bme.begin(); // Start up BME280 on default I2C address
  delay(1000);
  
  // 6/24/2026 added io pin to detect whether 0 or V  to change time interval between measurements 
  // set pin 13 to ground to put system in measurement mode with long intervals between measurements
  // defined by SLEEPTIME
  // set pin 13 to +V  to put system in test mode with short interval between measurments definced by TESTTIME
  pinMode(13,INPUT);
  if(digitalRead(13))
   {
    sleepinterval = TESTTIME;
    USBSerial = true;
   }
   else
   {
    sleepinterval = SLEEPTIME;
    USBSerial = false;
   }
  
  m5sensor.begin();
  bool m5status = m5sensor.checkConnection();
  delay(1000);

  if(USBSerial)
  {
    Serial.println("BME280 & M5 test on Feather ESP32 V2");
    if (!bmestatus) 
    {
      Serial.println("Could not find BME280 sensor!");
    }
    else 
    {
      Serial.println("BME280 initialized!");
    }
    if (!m5status) 
    {
      Serial.print(F("Error on init of Ultrasonic sensor: "));
    }
    else 
    {
      Serial.println(F("Ultrasonic sensor initialized!"));
    }
    
  }

}

void loop() 
{
  temp = bme.readTemperature();
  press = bme.readPressure() / 100.0;
  hum = bme.readHumidity();

  distavg = 0.0;
  for(int i = 0; i < READINGS; i++)
  {
    
    // New code for I2C ultrasonic sensor
    m5sensor.update();
    distance = m5sensor.getDistance();
    distavg += distance;
    delay(Distdelaytime); // Wait time ms between readings

  }
  distavg = distavg/READINGS;

  // Send this info to computer if in test mode
  if(USBSerial)
  {
    Serial.print("Temp: ");
    Serial.print(temp);
    Serial.println(" °C");

    Serial.print("Pressure: ");
    Serial.print(press);
    Serial.println(" hPa");

    Serial.print("Humidity: ");
    Serial.print(hum);
    Serial.println(" %");

    Serial.print(distavg);
    Serial.println(" mm");
    Serial.println("------------------");
  }

  // Send this info to the Meshtastic radio
  Serial1.print("Shark River Sensor 1");
  Serial1.print("Temp: ");
  Serial1.print(temp);
  Serial1.print(" °C; ");

  Serial1.print("Pressure: ");
  Serial1.print(press);
  Serial1.print(" hPa; ");

  Serial1.print("Humidity: ");
  Serial1.print(hum);
  Serial1.print(" %; ");

  Serial1.print("Distance: ");
  Serial1.print(distavg);
  Serial1.println(" mm");

  // Now go to deep sleep - feather will restart so this loop only acts once
  // sleepinterval  (in s) is defined above with operate or test durations
 
  esp_sleep_enable_timer_wakeup(sleepinterval * uS_TO_S);
  
  esp_deep_sleep_start();
  
}
/*  from grove example - other function includes getTravelTime
void loop() {
  sonar.update();
  Serial.print("Distance [mm]: ");
  Serial.print(sonar.getDistance());
  Serial.print("\t");
  Serial.print("travel time [us]: ");
  Serial.println(sonar.getTravelTime());
}*/