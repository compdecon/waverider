/*
CDL Tide Sensor with relay
This script runs on an Adafruit Feather Huzzah ESP32 V2 and periodically records a distance
 measurement from a Maxbotix ultrasonic sensor and (eventually) reads a BME280 T, P, humidity sensor.  It sends
 the data through a serial connection to a  Heltec meshtastic device which in turn broadcasts the data
 as a text message.  The feather switches to sleep mode after broadcasting the data in order
 to save energy.

Much of this code came from Tidesensor5.ino - see the comments there for the evolution of the tide sensor
hardware and script

The TPH sensor is connected to the feather through QWIC I2C connectors.  The
 software also will send the data through a USB serial port to a computer to troubleshoot.  To 
 enable the USB serial connection, set testmode to true.  Turn it off when not connected
 to a computer.  testmode is set by looking at voltage on TESTMODE_PIN

Distance readings are sampled READINGS times and averaged.  SLEEPTIME in seconds defines how long the feather sleeps 
before restarting  Testmode is now defined by this GPIO as well.   if in test mode (TESTMODE_PIN is high),  testmode = TRUE, meaning a 
computer hooked up to the ESP32 USBC can read the data

Added a featherwing latching relay to turn off the Heltec meshtastic modules in order to save power
GPIO Pins 14 and 15 are used to control a featherwing latching relay.  14 enables, (NO to COM), 15 turns off
(NO open, NC to COM).   

*/

#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_NeoPixel.h>
#define NEOPIXEL_I2C_POWER 2
#define PIN_NEOPIXEL 0

#define uS_TO_S 1000000ULL /* Conversion factor for microseconds to seconds */

#define READINGS 10   // Number of readings to average 
#define SLEEPTIME 600 // Sleeptime in seconds - every 10 minutes
#define TESTTIME 60   // Test mode every 60 seconds (not including reboot time)

#define BME_SCK 13 // for BME 280 
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define DIST_PW_PIN 16 // For Pulse width monitoring - measured distance is scaled: 147 uS = 1 inch. Pin 2 on Maxbotix
//#define DIST_TX_PIN 17 // For Maxbotix serial write

#define RADIO_RX_PIN 7 // connect to Heltec TX
#define RADIO_TX_PIN 8 // connect to Heltec RX

#define TESTMODE_PIN 13 // Test mode - apply Vcc to Test mode, 0V for operation

#define LATCHON_PIN 14 // for relay featherwing
#define LATCHOFF_PIN 15
#define LATCHTIME 10
#define WAKEUPTIME 5000

uint32_t Distancedelaytime=1; // in seconds.  Time delay between series of readings for averaging
uint64_t sleepinterval;  // amount of time feather goes into sleep mode in sec.
bool testmode; // Set to true if you want to see output on serial monitor on computer for testing, otherwise false
float temp=0, press=0, hum=0, distance, distavg;

Adafruit_BME280 bme;

void waitAsecond(float wait=1)
 {
  int time;
  time = wait*1000;
  delay(time);
 }

void setup() 
 { 
  
  // set TESTMODE_PIN to ground to put system in measurement mode with long intervals between measurements defined by SLEEPTIME
  // set TESTMODE_PIN to +V  to put system in test mode with short interval between measurements defined by TESTTIME
  pinMode(TESTMODE_PIN,INPUT);
  if(digitalRead(TESTMODE_PIN))
   {
    sleepinterval = TESTTIME;
    testmode = true;
   }
   else
   {
    sleepinterval = SLEEPTIME;
    testmode = false;
   }

  pinMode(NEOPIXEL_I2C_POWER, OUTPUT);  // Set pin to turn on and off on-board LED and I2C power - from Adafruit script
  pinMode(LATCHOFF_PIN, OUTPUT); // Signal relay to power down Heltec and Maxbotix
  pinMode(LATCHON_PIN, OUTPUT); // Turn on relay to power up Heltec and Maxbotix

  Serial.begin(9600); //for computer monitoring during test mode
  Serial1.begin(9600);  // Communicate with Heltec
  //Serial1.begin(9600, SERIAL_8N1, RADIO_RX_PIN, RADIO_TX_PIN);  // Communicate with Heltec
  //Serial2.begin(9600, SERIAL_8N1, DIST_RX_PIN, DIST_TX_PIN); // Serial communication with sensor
  waitAsecond();

  // STEMMA QT setup using standard pins for I2C
  Wire.begin();  // SDA, SCL for Feather
  waitAsecond();

 }

void loop() 
 {
  powerup();
  
  temp = bme.readTemperature();
  press = bme.readPressure()/100.0; // hectapascals
  hum = bme.readHumidity();

  // Send this info to computer serial monitor if it is in test mode
  if(testmode)
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

    Serial.println("Testmode Port High, Test Mode");
    Serial.print(distavg);
    Serial.println(" mm");
    Serial.println("------------------");
  }

  // Send this info to the Meshtastic radio
  
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

  distavg = 0.0;
  for(int i = 0; i < READINGS; i++)
  {
    distance = round(pulseIn(DIST_PW_PIN, HIGH)*5.4043e-02); // result is cm

    Serial1.print("Distance: ");
    Serial1.print(distance);
    Serial1.println(" cm");

    if(testmode)
    {
      Serial.print("Distance: ");
      Serial.print(distance);
      Serial.println(" cm");
    }
    waitAsecond(Distancedelaytime); // Wait time between readings

  }

  distavg = round(distavg/READINGS);
  Serial1.print("Average Distance: ");
  Serial1.print(distavg);
  Serial1.println(" cm");

  if(testmode)
  {
    Serial.print("Average Distance: ");
    Serial.print(distavg);
    Serial.println(" cm");
  }

  powerdown();  // turn off to save power until next reading
  
 }

void powerup()
 {
      

  digitalWrite(NEOPIXEL_I2C_POWER, HIGH); 

  digitalWrite(LATCHON_PIN, HIGH);
  delay(LATCHTIME);
  digitalWrite(LATCHON_PIN, LOW);
  delay(WAKEUPTIME); // wait for Heltec and Maxbotix sensor to wake up, msec
 

  bool bmestatus = bme.begin(); // Start up BME280 on default I2C address
  delay(1000); // wait 1 sec to start up

  if(testmode)
  {
    Serial.println("BME280 & Maxbotix test on Feather ESP32 V2");
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


void powerdown()
 {
  
  // Now go to light sleep
  // sleepinterval  (in s) is defined above with operate or test
  
  digitalWrite(NEOPIXEL_I2C_POWER, LOW);

  digitalWrite(LATCHOFF_PIN, HIGH);
  delay(LATCHTIME);
  digitalWrite(LATCHOFF_PIN, LOW);

  esp_sleep_enable_timer_wakeup(sleepinterval * uS_TO_S); // sleepinterval in sec.
  
  esp_light_sleep_start();
  
 }


