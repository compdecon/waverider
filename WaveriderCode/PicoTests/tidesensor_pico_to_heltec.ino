
#include <HCSR04.h>

byte triggerPin = 11;
byte echoPin = 10;

#define LED_PIN LED_BUILTIN

void setup () {
  HCSR04.begin(triggerPin, echoPin);

  pinMode(LED_PIN, OUTPUT);


  Serial.begin(9600);

  Serial2.setTX(8);   // GPIO 8 as TX pin 11
  Serial2.setRX(9);   // GPIO 9 as RX pin 12

  Serial2.begin(9600);  // Start UART1 at 9600 baud

}

void loop () {
  digitalWrite(LED_PIN, HIGH);
  double* distances = HCSR04.measureDistanceCm();
  
  //Serial1.println(distances[0]);
  //Serial2.println("hello");
  //delay(2000);
  

  Serial.print("Distance Sensor 1: ");
  Serial.print(distances[0]);
  Serial.println(" cm");
  
  //Serial2.print("1: ");
  Serial2.print(distances[0]);
  //Serial2.println(" cm");
  
  Serial.println("---");
  //delay(250);
  //digitalWrite(LED_PIN, LOW);
  delay(2000);
  
}