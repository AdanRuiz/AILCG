#include <SoftwareSerial.h>

//#define ssRX 2
//#define ssTX 3
//SoftwareSerial nss(ssRX, ssTX);

const int ledPin = 13; // the pin that the LED is attached to
int incomingByte;      // a variable to read incoming serial data into

void setup() {
  // initialize serial communication:
  Serial.begin(9600);
  Serial3.begin(9600);
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);

  Serial.println("Hola");
}

void loop() {
 
   if (Serial3.available())
  {
   int f = Serial3.parseInt();  
   /*if (f > 0)
   {*/
     Serial.println(f);
       analogWrite(ledPin, f);
   //} 
  } 
  

}
