#include <XBee.h>

const int ledPin = 9; // the pin that the LED is attached to
int incomingByte;      // a variable to read incoming serial data into
XBee xbee  = XBee();;


void setup() {
  // initialize serial communication:
  Serial.begin(9600);
  xbee.setSerial(Serial);
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
}

void loop() {
    xbee.readPacket();
    if (xbee.getResponse().isAvailable())
    {
    int f = Serial.parseInt();  
    /*if (f > 0)
    {*/
    //Serial.println(f);
    analogWrite(ledPin, f);
    //} 
    } 
  

}
