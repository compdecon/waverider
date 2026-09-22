#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_NeoPixel.h>

#define NEOPIXEL_I2C_POWER 2
#define uS_TO_S 1000000ULL /* Conversion factor for microseconds to seconds */

#define READINGS 10   // Number of readings to average 
#define SLEEPTIME 600 // Sleeptime in seconds - every 10 minutes
#define TESTTIME 60   // Test mode every 60 seconds (not including reboot time)

#define BME_SCK 13 // for BME 280 
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10
#define WAKEUPTIME 30000 // 30 sec
uint32_t Distancedelaytime=1; // in seconds.  Time delay between series of readings for averaging
uint64_t sleepinterval=15;  // amount of time feather goes into sleep mode in sec.
bool testmode; // Set to true if you want to see output on serial monitor on computer for testing, otherwise false
float temp=0, press=0, hum=0, distance, distavg;

Adafruit_BME280 bme;

void waitAsecond(float wait=1)
 {
  int time;
  time = wait*1000;
  delay(time);
 }

void setup() {

  pinMode(NEOPIXEL_I2C_POWER, OUTPUT);  // Set pin to turn on and off on-board LED and I2C power - from Adafruit script

  Serial.begin(9600); //for computer monitoring during test mode
  //Serial1.begin(9600);  // Communicate with Heltec
  //Serial1.begin(9600, SERIAL_8N1, RADIO_RX_PIN, RADIO_TX_PIN);  // Communicate with Heltec
  //Serial2.begin(9600, SERIAL_8N1, DIST_RX_PIN, DIST_TX_PIN); // Serial communication with sensor
  waitAsecond();

  // STEMMA QT setup using standard pins for I2C
  Wire.begin();  // SDA, SCL for Feather
  waitAsecond();
  

}

void loop() {
  powerup();
  
  temp = bme.readTemperature();
  press = bme.readPressure()/100.0; // hectapascals
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


  powerdown();  // turn off to save power until next reading
  
 }

void powerup()
 {
      

  digitalWrite(NEOPIXEL_I2C_POWER, HIGH); 

  //delay(WAKEUPTIME); // wait for Heltec and Maxbotix sensor to wake up, msec
  waitAsecond();
  bool bmestatus = bme.begin(); // Start up BME280 on default I2C address
  waitAsecond(); // wait 1 sec to start up



    Serial.println("BME280 startup on Feather ESP32 V2");
    if (!bmestatus) 
    {
      Serial.println("Could not find BME280 sensor!");
    }
    else 
    {
      Serial.println("BME280 initialized!");
    } 



 }


void powerdown()
 {
  
  // Now go to light sleep
  // sleepinterval  (in s) is defined above with operate or test
  Serial.println("Powering down");
  waitAsecond(5);
  Serial.println("Turning off neopixel and I2C");

  digitalWrite(NEOPIXEL_I2C_POWER, LOW);
  waitAsecond(5);
  Serial.println("Going to sleep for 15 sec");
  waitAsecond();
  esp_sleep_enable_timer_wakeup(sleepinterval * uS_TO_S); // sleepinterval in sec.
  
  esp_light_sleep_start();
  
 }

