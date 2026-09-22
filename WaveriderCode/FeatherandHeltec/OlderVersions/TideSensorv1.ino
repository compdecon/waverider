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

*/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_VL53L1X.h>

#define USBSerial false /* Set to true if you want to see output on computer, otherwise false */
#define SEALEVELPRESSURE_HPA 1013.25
#define uS_TO_S 1000000ULL /* Conversion factor for micro seconds to seconds */
#define IRQ_PIN 2
#define XSHUT_PIN 3
#define READINGS 10
#define SLEEPTIME 600 /* Sleeptime in seconds */

uint32_t Distdelaytime=5000; // in milliseconds
int16_t distance;

bool bmestatus;

float temp, press, hum, distavg;

Adafruit_BME280 bme;
Adafruit_VL53L1X vl53 = Adafruit_VL53L1X(XSHUT_PIN, IRQ_PIN);

void setup() {
  
  Serial.begin(115200);
  delay(1000);

  // Use correct STEMMA QT pins
  Wire.begin(22, 20);  // SDA, SCL for Feather
  delay(1000);

  bool bmestatus = bme.begin(0x76); // Start up BME280 on default I2C address
  delay(1000);
  bool vl53status = vl53.begin(0x29, &Wire);
  delay(1000);
  bool vl53statusrange = vl53.startRanging();
  delay(1000);
  // Valid timing budgets: 15, 20, 33, 50, 100, 200 and 500ms!
  vl53.setTimingBudget(500);


  Serial1.begin(9600); // ,SERIAL_8N1,7,8,false,20000UL,120); // Start up communications with Meshtastic device
  while(!Serial1); // Wait for communications to begin

  if(USBSerial)
  {
    Serial.println("BME280 & VL53L1X test on Feather ESP32 V2");
    if (!bmestatus) 
    {
      Serial.println("Could not find BME280 sensor!");
    }
    else 
    {
      Serial.println("BME280 initialized!");
    }
    if (! vl53status) 
    {
      Serial.print(F("Error on init of VL sensor: "));
      Serial.println(vl53.vl_status);
    }
    else 
    {
      Serial.print(F("VL sensor initialized!"));
    }
    if (! vl53statusrange) {
      Serial.print(F("Couldn't start ranging: "));
      Serial.println(vl53.vl_status);
    }
    else
    {
        Serial.print(F("VL53 is ranging"));
    }
  
  }

}

void loop() 
{
  temp = bme.readTemperature();
  press = bme.readPressure() / 100.0;
  hum = bme.readHumidity();
  
  for(int i = 0; i < READINGS; i++)
  {
    if (vl53.dataReady()) 
    {
      distance = vl53.distance();
    }
    else
    {
      distance = 0;
    }
    distavg += distance;
    delay(Distdelaytime);

  }
  distavg = distavg/READINGS;

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
    
  Serial1.print("Temp: ");
  Serial1.print(temp);
  Serial1.print(" °C; ");

  Serial1.print("Pressure: ");
  Serial1.print(press);
  Serial1.print(" hPa; ");

  Serial1.print("Humidity: ");
  Serial1.print(hum);
  Serial1.print(" %; ");

  Serial1.print(distavg);
  Serial1.println(" mm");

  // Now go to deep sleep - feather will restart so this loop only acts once
  // From TimerWakeUp example
  esp_sleep_enable_timer_wakeup(SLEEPTIME * uS_TO_S);
  
  esp_deep_sleep_start();

}