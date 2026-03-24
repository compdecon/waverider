/* This program reads temp, press, and humidity and  a serial port to a pc, and
   finally to a pico W MEshtastic device that sends the temperature out as a text message over the mesh
   gpw & md  3/2026
*/

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

/*  3/18/2026  gpw and md.   Using Serial 2 to connect pico W that controls bme temp sensor 
   with pico W that controls meshtastic LoRa shield.  TX for Serial2 is set to GPIO pin 8 (physical pin 11)
   on sending pico W, which is wired to GPIO pin 9 (physical pin 12) on receiving pico W.   RX is set to GPIO pin 9
   (physical pin 12) on sending pico and GPIO 8 on receiving unit.
   The receiving pico W is programmed with meshtastic software to send the incoming serial data out across the network as a text message.

   This demo uses the bme280 T,Hum,P sensor to broadcast the temperature 


// gpw & md 3/2026
// 
*/
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
// #include <Bonezegei_LCD1602_I2C.h>  // LCD display

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C
// Bonezegei_LCD1602_I2C lcd(0x27); // LCD display with I2C address 0x27

unsigned long delayTime=10000;  // every 10 seconds
unsigned status;
String str1, str2;

void setup() {
//   Serial.begin(9600);
   Serial2.setTX(8);
   Serial2.setRX(9);
   Serial2.begin(9600); // 
 //  while(!Serial);    // time to get serial running
   while(!Serial2);   
   

   status = bme.begin();  
//   Serial.println(F("BME280 test"));
   
//   lcd.begin(); // start up display
   delay(500);  
//   lcd.setPosition(0, 0); //param1 = X   param2 = Y
//   lcd.print("CDL Test");

    
}


void loop() { 

  //Serial.print(CtoF(bme.readTemperature()));
  //Serial.println(" F");
  Serial2.print(CtoF(bme.readTemperature()));
  Serial2.println(" F");
   
  /* lcd.setPosition(0, 1);
  str1 = String(CtoF(bme.readTemperature()),1); // temperature converted to F with 1 sig figure
  str1 += "F  "; 
  str1 += String(bme.readHumidity(),0); // humidity in % with 1 sig figure
  str1 += " %";
  lcd.print(str1.c_str());
  */
  //printValues();
  //delay(1000);

   delay(delayTime);
}


void printValues() {
    Serial.print("Temperature = ");
    Serial.print(CtoF(bme.readTemperature()));
    Serial.println(" F");

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
 *  				Using both Rothfusz and Steadman's equations
 *					(http://www.wpc.ncep.noaa.gov/html/heatindex_equation.shtml)
 *  @param  T
 *          temperature in  degrees Fahrenheit
 *  @param  RH
 *          humidity in percent
 *  
 *	@return float heat index
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
