/*
CDL Tide Sensor
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
  Using Arduino_GroveI2C_Ultrasonic library, no, M5 library.  
  OK - ultrasonic sensor works inside stilling pipe!  

7/6/26  removing BME280 code - ultrasonic sensor not responding consistently
  Hopefully simplifying code will help troubleshoot

7/15/2026
Getting inconsistent results from ultrasonic sensor.  It may be a problem with the M5 I2C implementation (saw some
posts about a problem).  Taking BME280 out of the circuit didn't help. So, back to an ultrasonic sensor using 
gpio pins for control.  Using HC-SR04 sensor with 5V power, and 2 10k ohm resistors to divide the sensor output 
signal to 2.5V to be compatible with 3.3 V Feather.  Code for BME280 sensor is back too.  
Added code to turn off LED and I2C power during sleep.
This time for sure!

8/3/2026
I forgot to set pinMode for HC-SR04 sensor!  That's why I never got a reading! Works fine now in test mode
with complete setup including battery, solar panel, and stilling pipe.  New sensor fixture Matt printed fits well too.
I think it's ready for prime time now
*/

#include <Wire.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA 1013.25
#define uS_TO_S 1000000ULL /* Conversion factor for microseconds to seconds */

#define READINGS 10   // Number of readings to average 
#define SLEEPTIME 600 // Sleeptime in seconds - every 10 minutes
#define TESTTIME 30   // Test mode every 30 seconds (not including reboot time)

#define BME_SCK 13 // for BME
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define TRIG_PIN 14 // for HC-SR04
#define ECHO_PIN 15

uint32_t Distancedelaytime=5000; // in milliseconds.  Time delay between series of readings for averaging
uint64_t sleepinterval;  // amount of time feather goes into deep sleep mode in sec.
bool USBSerial; /* Set to true if you want to see output on serial monitor on computer, otherwise false */
float temp, press, hum, duration, distance, distavg;

Adafruit_BME280 bme;

void setup() {
  
  // pinMode(NEOPIXEL_I2C_POWER, OUTPUT); // Turn on LED and I2C powr for light sleep mode if needed
  // digitalWrite(NEOPIXEL_I2C_POWER, HIGH);

  Serial.begin(9600);
  Serial1.begin(9600);
  delay(1000);

  // Use correct STEMMA QT pins
  Wire.begin();  // SDA, SCL for Feather
  delay(1000);
  
  bool bmestatus = bme.begin(); // Start up BME280 on default I2C address
  delay(1000);
  
  // 6/24/2026 added io pin to detect whether 0 or V  to change time interval between measurements 
  // set pin 13 to ground to put system in measurement mode with long intervals between measurements
  // defined by SLEEPTIME
  // set pin 13 to +V  to put system in test mode with short interval between measurements defined by TESTTIME
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
  // Set up HC-SR04 Digital IO pins
 
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  if(USBSerial)
  {
    Serial.println("BME280 & HC-SR04 test on Feather ESP32 V2");
    if (!bmestatus) 
    {
      Serial.println("Could not find BME280 sensor!");
    }
    else 
    {
      Serial.println("BME280 initialized!");
    } 

  }

}

void loop() 
{
  
  temp = bme.readTemperature();
  press = bme.readPressure() / 100.0; // hectapascals
  hum = bme.readHumidity();

  // Send this info to computer serial monitor if it is in test mode
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

    Serial.println("USBSerial Port High, Test Mode");
    Serial.print(distavg);
    Serial.println(" mm");
    Serial.println("------------------");
  }

  // Send this info to the Meshtastic radio

  Serial1.print("Shark River Sensor 1");
  Serial1.print("Temp: ");
  Serial1.print(temp);
  Serial1.println(" °C; ");

  Serial1.print("Pressure: ");
  Serial1.print(press);
  Serial1.println(" hPa; ");

  Serial1.print("Humidity: ");
  Serial1.print(hum);
  Serial1.println(" %; ");

  distavg = 0.0;
  for(int i = 0; i < READINGS; i++)
  {
    // Get distance info from HC-SR04 old fashion way, using GPIO pins, not I2C - 7/15/2026
    
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2); // ?
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10); // hold signal high for 10 us to trigger sound pulse
    digitalWrite(TRIG_PIN, LOW);
    duration = pulseIn(ECHO_PIN, HIGH);  // time interval for echo is length of this pulse
    distance = duration*0.5*0.343;  // inverse of 2.91
    distavg += distance;

    Serial1.print("Distance: ");
    Serial1.print(distance);
    Serial1.println(" mm");

    if(USBSerial)
    {
      Serial.print("Distance: ");
      Serial.print(distance);
      Serial.println(" mm");
    }
    delay(Distancedelaytime); // Wait time ms between readings

  }

  distavg = distavg/READINGS;
  Serial1.print("Average Distance: ");
  Serial1.print(distavg);
  Serial1.println(" mm");

  if(USBSerial)
  {
    Serial.print("Average Distance: ");
    Serial.print(distavg);
    Serial.println(" mm");
  }

  // Now go to deep sleep - feather will restart so this loop only executes once
  // sleepinterval  (in s) is defined above with operate or test durations
  
  pinMode(NEOPIXEL_I2C_POWER, OUTPUT);  // Turn off on-board led and I2C power - from Adafruit script
  digitalWrite(NEOPIXEL_I2C_POWER, LOW);

  esp_sleep_enable_timer_wakeup(sleepinterval * uS_TO_S);
  
  esp_deep_sleep_start();
  
}
