/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
 
  This example code is in the public domain.
 */
#include <Console.h>
#include <SoftwareSerial.h>
#include <XBee.h>

SoftwareSerial mySerial(8, 9); // RX, TX
 
int ledPin = 3;
const int pinSensor = A1;       // pin del puerto analogico
unsigned int valorAnalogico = 0;         // variable para guardar el valor leido del sensor
unsigned int valorLuzPWM = 0;

XBee xbee = XBee();

// the setup routine runs once when you press reset:
void setup() {           
    mySerial.begin(9600);
    pinMode(ledPin, OUTPUT);
    
    Bridge.begin();
    Console.begin();

    xbee.setSerial(mySerial);
}

// the loop routine runs over and over again forever:
void loop() {
  lightsControlPhotoR();
  delay(1500);
}
void lightsControlPhotoR(){
  String out = "XB:1,";
              
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
    //transmit(valorLuzPWM); 

    out.concat("VA:");
    out.concat(valorAnalogico);
    out.concat(",PWM:");
    out.concat(valorLuzPWM);
    //out.concat("\r");
    
    /*Console.print("VA ");
    Console.print(valorAnalogico);
    Console.print(" PWM ");*/
    
    
    
    
    transmit(out);

}

void transmit(String out){
    int str_len = out.length() + 1; 
    char transmision [str_len];
    out.toCharArray(transmision,str_len);

    Console.println(str_len);
    Console.println(transmision);
    
    // Specify the address of the remote XBee (this is the SH + SL) 40F4EE73
    XBeeAddress64 Broadcast = XBeeAddress64(0x00000000, 0x0000ffff);
    
    // Create a TX Request
    ZBTxRequest zbTx = ZBTxRequest(Broadcast, (uint8_t *)transmision, sizeof(transmision));
    
    // Send your request
    xbee.send(zbTx);

}

