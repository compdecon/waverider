// This script reads temp, press, humidity data from I2C connected BME280 sensor and
// displays it on a I2C connected 1602 LCD display.  Libraries from Adafruit and Bonezegei are used
// for the BME280 and LCD, respectively
// A Raspberry Pi Pico W is used to control everything
// gpw and md 3/2026

// This script operates the QAPASS I2C connected 1602 LCD display from Matt's electronics kit
// The adafruit LCD libraries do not work with this display, but the Bonezegei library does work
// The I2C address appears to be 0x27 (not 0x20 for Adafruit library default)
// The display is connected to a Rasberry Pi Pico W  
// Connections:
//  Display GND -> Pico Pin 38 (GND)
//  Display VCC -> Pico Pin 40 (VBUS)
//  Display SDA -> Pico Pin 6 (GP4, I2C0 SDA)
//  Display SCL -> Pico Pin 7 (GP5, I2C0 SCL)
//
// gpw & md 3/2026
// 

/***************************************************************************
  This is a library for the BME280 humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BME280 Breakout
  ----> http://www.adafruit.com/products/2652

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface. The device's I2C address is either 0x76 or 0x77.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
  See the LICENSE file for details.
 ***************************************************************************/
// Connections for BME280
// GND to pico pin 38 (GND)
// Vin to pico pin 40 (VBUS)
// SCK to pico pin 7 (GP4, I2C0 SDA)
// SD1 to pico pin 6 (GP4, I2C0 SDA)
// 

#include <Bonezegei_LCD1602_I2C.h>  // LCD display

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>  // Adafruit sensor

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // Adafruit I2C Temp, Pressure and humidity sensor
Bonezegei_LCD1602_I2C lcd(0x27); // LCD display with I2C address 0x27

unsigned long delayTime;
String str1, str2;

void setup() {
  lcd.begin(); // start up display
  delay(500);  
  lcd.setPosition(0, 0); //param1 = X   param2 = Y
  lcd.print("CDL Test");

  bme.begin(); //; start up sensor   
  delay(2000);

  Serial.begin(9600);  // Set up serial communications to monitor info on laptop
  while(!Serial);    // time to get serial running
  Serial.println(F("BME280 test"));
  delayTime = 1000;
  Serial.println();
}

void loop() {
  lcd.setPosition(0, 1);
  
  str1 = String(CtoF(bme.readTemperature()),1); // temperature converted to F with 1 sig figure
  str1 += "F  "; 
  str1 += String(bme.readPressure(),0); // humidity in % with 1 sig figure
  str1 += "pa";
  lcd.print(str1.c_str());
  printValues();
  delay(1000);  
}


void printValues() {
    Serial.print("Temperature = ");
    Serial.print(CtoF(bme.readTemperature()));
    Serial.println(" °F");

    Serial.print("Pressure = ");

    Serial.print(bme.readPressure() / 100.0F);
    Serial.println(" hPa");

    Serial.print("Approx. Altitude = ");
    Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.println(" m");

    Serial.print("Humidity = ");
    Serial.print(bme.readHumidity());
    Serial.println(" %");

    Serial.print("Feels Like Temperature = ");
    Serial.print(computeHeatIndex(CtoF(bme.readTemperature()), bme.readHumidity()));
    Serial.println(" °F");

    Serial.println();
}

float CtoF(float temperatureC)
 {
    return 1.8 * temperatureC + 32.0;
 }

float computeHeatIndex(float T, float RH)
 {
    /*!
 *  @brief  Compute Heat Index - mooched from Adafruit DHT library and modified gpw 11/25
 *             Using both Rothfusz and Steadman's equations
 *             (http://www.wpc.ncep.noaa.gov/html/heatindex_equation.shtml)
 *  @param  T
 *          temperature in  degrees Fahrenheit
 *  @param  RH
 *          humidity in percent
 *  
 *  @return float heat index
 */

    float hi;

    /* Use simple formula first */ 

    hi = 0.5 * (T + 61.0 + ((T - 68.0) * 1.2) + (RH * 0.094));

    if (hi >= 80) 
     {
        hi = -42.379 + 2.04901523 * T + 10.14333127 * RH +
         -0.22475541 * T * RH +
         -0.00683783 * T * T +
         -0.05481717 * RH * RH +
         0.00122874 * T * T * RH +
         0.00085282 * T * RH * RH +
         -0.00000199 * T * T * RH * RH;
     }
    if ((RH < 13) && (T >= 80.0) && (T <= 112.0))
     {
        hi -= ((13.0 - RH) * 0.25) * sqrt((17.0 - abs(T - 95.0)) * 0.05882);
     }
    else if ((RH > 85.0) && (T >= 80.0) && (T <= 87.0))
     {
        hi += ((RH - 85.0) * 0.1) * ((87.0 - T) * 0.2);
     }
  return hi;
}
