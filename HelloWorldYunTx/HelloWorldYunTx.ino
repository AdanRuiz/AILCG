/*
XBee TX test for a Arduino Mega2560 using Serial3 as the XBee serial
input for a Series 2 XBee.  This is NOT based on the examples that come with
the Arduino XBee library.

See, the examples there and most other places on the web SUCK.  Andrew's
library is much easier to use than the illustrations would lead you to believe.

This is a HEAVILY commented example of how send a text packet using series 2
XBees.  Series 1 XBees are left as an exercise for the student.
*/

#include <XBee.h>
#include <Console.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(8, 9); // RX, TX

XBee xbee = XBee();
// This is the XBee broadcast address.  You can use the address
// of any device you have also.
XBeeAddress64 Broadcast = XBeeAddress64(0x00000000, 0x0000ffff);

char Hello[] = "Hello World";
char Buffer[128];  // this needs to be longer than your longest packet.

void setup() { 
  // start serial
  Bridge.begin();
  Console.begin();
  // and the software serial port
  mySerial.begin(9600);
  // now that they are started, hook the XBee into
  // Software Serial
  xbee.setSerial(mySerial);
  Console.println("Initialization all done!");
}

void loop() {
  ZBTxRequest zbtx = ZBTxRequest(Broadcast, (uint8_t *)Hello, strlen(Hello));
  xbee.send(zbtx);
  delay(2000);
  strcpy(Buffer,"I saw what you did last night.");
  zbtx = ZBTxRequest(Broadcast, (uint8_t *)Buffer, strlen(Buffer));
  xbee.send(zbtx);
  delay(2000);
}

