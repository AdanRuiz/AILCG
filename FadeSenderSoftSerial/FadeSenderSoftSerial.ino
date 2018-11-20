#include <SoftwareSerial.h>

#define ssRX 2
#define ssTX 3
SoftwareSerial nss(ssRX, ssTX);/*
 Fade

 This example shows how to fade an LED on pin 9
 using the analogWrite() function.

 This example code is in the public domain.
 */

int led = 13;           // the pin that the LED is attached to
int brightness = 0;    // how bright the LED is
int fadeAmount = 5;    // how many points to fade the LED by
int delayTime = 1000;
// the setup routine runs once when you press reset:
void setup() {
  // declare pin 9 to be an output:
  pinMode(led, OUTPUT);
  nss.begin(9600);
}

// the loop routine runs over and over again forever:
void loop() {
  // set the brightness of pin 9:
 

  // reverse the direction of the fading at the ends of the fade:
 
    analogWrite(led, brightness);
    int i = 0;
    while(i < delayTime){
      nss.println(brightness);
      i++;
    }
    //delay(delayTime);
    brightness = 255;
    analogWrite(led, brightness);
    i=0;
    while(i < delayTime){
      nss.println(brightness);
      i++;
    }
    brightness = 1;
  
  
}

