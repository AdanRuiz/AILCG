/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
 
  This example code is in the public domain.
 */
#include <Console.h>
#include <SoftwareSerial.h>
SoftwareSerial mySerial(8, 9); // RX, TX
 
int ledPin = 3;
const int pinSensor = A1;       // pin del puerto analogico
unsigned int valorAnalogico = 0;         // variable para guardar el valor leido del sensor
unsigned int valorLuzPWM = 0;


// the setup routine runs once when you press reset:
void setup() {                
    mySerial.begin(9600);
    pinMode(ledPin, OUTPUT);
    
    Bridge.begin();
    Console.begin();

}

// the loop routine runs over and over again forever:
void loop() {
  lightsControlPhotoR();
  delay(300);
}
void lightsControlPhotoR(){
  String out = "";
              
    valorAnalogico = analogRead(pinSensor);
 
    //254     800
    // ?    valorAnalogico
    if(valorAnalogico < 150)
     valorAnalogico=0;
    
    long  t1 =  valorAnalogico * 255L ;
    valorLuzPWM = int(  t1   / 1024L) ;
    //validaci�n en caso de que el sensor supere el minimo
    
    // valorLuzPWM = 255 - valorLuzPWM ;
    //enciende led
    analogWrite(ledPin, valorLuzPWM);
    
    //  manda el valor por el serial
    mySerial.println(valorLuzPWM); 
    
    Console.print("VA ");
    Console.print(valorAnalogico);
    Console.print(" PWM ");
    Console.println(valorLuzPWM);


}

