
/** 
I took Andrew Rapp's receive example and modified it to be completely
unrecognizable. 

I wanted to experiment with and better understand how to use his XBee
library for an actual project.  With the inclusion of support for SoftwareSerial
so that the XBee can be put on digital pins leaving the Arduino serial port
available for debugging and status, the library's usefullness shot way up.

This is a HEAVILY commented example of how to grab a receive packet off the
air and do something with it using series 2 XBees.  Series 1 XBees are left as
and exercise for the student.
*/ 
 
#include <XBee.h>
#include <SoftwareSerial.h>
#include <Console.h>
#include <YunClient.h>
#include <SPI.h>

//Zigbee Objects
XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
ZBRxResponse rx = ZBRxResponse();

// Define NewSoftSerial TX/RX pins
#define ssRX 8
#define ssTX 9
SoftwareSerial nss(ssRX, ssTX);

//
const char *server = "10.0.5.220";
YunClient client;
String xbeeMsgPublish[3]={"","",""};
String xbeeMessages[3][2]={
  {"",""},
  {"",""},
  {"",""}
};


void setup() { 
    Bridge.begin();
    Console.begin();
  
    
    nss.begin(9600);
    xbee.setSerial(nss);
  
    Console.println("starting up");

    Process date;
    date.runShellCommand("ntpd -qn -p 10.0.5.220");

    notify();
}

long lastNotify=0;

void notify(){
        lastNotify = millis();

        String date = GetDateAndTime();
        String hourStr = GetHour() ;
        int hourInt = hourStr.toInt()-5;

        int pctLed = 100;

        
        int ledStatus = 0;
        
        //if((hourInt >=6 && hourInt <=23))
          //ledStatus = 125;
          ledStatus = pctLed *255 /100;
        // else
         // ledStatus = 0;
        
        String out= "{'XB':0,'PWM':";
        out.concat(ledStatus);
        out.concat(",'WPC':0}");

        transmit(out);
    
}


void loop() {
    //Console.print(".");
    if(millis() % 2000 == 0 || millis() - lastNotify > 2000){
        notify();
    }
    if(millis() % 59000 == 0 ){
        Process date;
        date.runShellCommand("ntpd -qn -p 10.0.5.220");
    }

    
    // doing the read without a timer makes it non-blocking, so
    // you can do other stuff in loop() as well.
    xbee.readPacket();
    // so the read above will set the available up to
    // work when you check it.
    if (xbee.getResponse().isAvailable()) {
      // got something
      if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
        // got a zb rx packet, the kind this code is looking for
      
        // now that you know it's a receive packet
        // fill in the values
        xbee.getResponse().getZBRxResponse(rx);
      
        // this is how you get the 64 bit address out of
        // the incoming packet so you know which device
        // it came from
        Console.print("Got an rx packet from: ");
        XBeeAddress64 senderLongAddress = rx.getRemoteAddress64();
        print32Bits(senderLongAddress.getMsb());
        Console.print(" ");
        print32Bits(senderLongAddress.getLsb());
      
        // this is how to get the sender's
        // 16 bit address and show it
        uint16_t senderShortAddress = rx.getRemoteAddress16();
        Console.print(" (");
        print16Bits(senderShortAddress);
        
        Console.println(")");
      
        // The option byte is a bit field
        if (rx.getOption() & ZB_PACKET_ACKNOWLEDGED)
            // the sender got an ACK
          Console.println("packet acknowledged");
        if (rx.getOption() & ZB_BROADCAST_PACKET)
          // This was a broadcast packet
          Console.println("broadcast Packet");
        
        Console.print("checksum is ");
        Console.println(rx.getChecksum(), HEX);
      
        // this is the packet length
        Console.print("packet length is ");
        Console.print(rx.getPacketLength(), DEC);
      
        // this is the payload length, probably
        // what you actually want to use
        Console.print(", data payload length is ");
        Console.println(rx.getDataLength(),DEC);
      
        // this is the actual data you sent
        Console.println("Received Data: ");
        for (int i = 0; i < rx.getDataLength(); i++) {
          print8Bits(rx.getData()[i]);
          Console.print(' ');
        }
      
        // and an ascii representation for those of us
        // that send text through the XBee
        /*Serial.println();
        for (int i= 0; i < rx.getDataLength(); i++){
          Serial.write(' ');
          if (iscntrl(rx.getData()[i]))
            Serial.write(' ');
          else
            Serial.write(rx.getData()[i]);
          Serial.write(' ');
        }
        Serial.println();*/ //adan
      
        // So, for example, you could do something like this:
        handleXbeeRxMessage(rx.getData(), rx.getDataLength());
        
/*
        // I commented out the printing of the entire frame, but
        // left the code in place in case you want to see it for
        // debugging or something
        Serial.println("frame data:");
        for (int i = 0; i < xbee.getResponse().getFrameDataLength(); i++) {
          print8Bits(xbee.getResponse().getFrameData()[i]);
          Serial.print(' ');
        }
        Serial.println();
        for (int i= 0; i < xbee.getResponse().getFrameDataLength(); i++){
          Serial.write(' ');
          if (iscntrl(xbee.getResponse().getFrameData()[i]))
            Serial.write(' ');
          else
            Serial.write(xbee.getResponse().getFrameData()[i]);
          Serial.write(' ');
        }
        Serial.println();
*/
      }
    }
    else if (xbee.getResponse().isError()) {
      // some kind of error happened, I put the stars in so
      // it could easily be found
      Console.print("************************************* error code:");
      Console.println(xbee.getResponse().getErrorCode(),DEC);
    }
    else {
      // I hate else statements that don't have some kind
      // ending.  This is where you handle other things
    }
}

String getJsonValue(String search, String json){
    int lastIndexSearch = json.lastIndexOf(search);
        String afterIndex = json.substring(lastIndexSearch);
        Serial.println(afterIndex);
        int valueIndex = afterIndex.indexOf(':');
        /*String valueWithTrash = afterIndex.substring(0,valueIndex);
        Serial.print(valueWithTrash);Serial.print('=');*/
        int commaIndex = afterIndex.indexOf(',');
        String wpValue = afterIndex.substring(valueIndex+1,commaIndex);
        
    return wpValue;
}





void handleXbeeRxMessage(uint8_t *data, uint8_t length){
    // this is just a stub to show how to get the data,
    // and is where you put your code to do something with
    // it.
    
    char myCharArray[length];
    for (int i = 0; i < length; i++){
      myCharArray[i]=data[i];
      //Serial.write(data[i]);
    }
    String myString (myCharArray);
    Console.print("content: ");
    Console.println(myString);
    Console.println();

    int cXbee = getJsonValue("XB", myString).toInt();
    int cXbeeMsgPart = 0;
    cXbeeMsgPart = getJsonValue("Part", myString).toInt();
    xbeeMessages[cXbee-1][cXbeeMsgPart-1] = myString;

    printMatrixXbee();

  	send_request(myString);
  	wait_response();
  	read_response();
    
}


void printMatrixXbee(){
  Console.println("--- Matrix ---");
  for(int i=0;i<3;i++){
    for(int j=0;j<2;j++){
      xbeeMsgPublish[i].concat(xbeeMessages[i][j]);
      xbeeMsgPublish[i].concat(" ");
      Console.print(xbeeMessages[i][j]);
      Console.print(" ");
    }//j
    Console.println(",");
  }//i
  Console.println("--- ---");
}
// these routines are just to print the data with
// leading zeros and allow formatting such that it
// will be easy to read.
void print32Bits(uint32_t dw){
  print16Bits(dw >> 16);
  print16Bits(dw & 0xFFFF);
}

void print16Bits(uint16_t w){
  print8Bits(w >> 8);
  print8Bits(w & 0x00FF);
}

void print8Bits(byte c){
  uint8_t nibble = (c >> 4);
  if (nibble <= 9)
    Console.write(nibble + 0x30);
  else
    Console.write(nibble + 0x37);
      
  nibble = (uint8_t) (c & 0x0F);
  if (nibble <= 9)
    Console.write(nibble + 0x30);
  else
    Console.write(nibble + 0x37);
}




void transmit(String out){
    int str_len = out.length() + 1; 
    char transmision [str_len];
    out.toCharArray(transmision,str_len);
    Console.print("/* Length: ");
    Console.print(str_len);
    Console.print(", Data: ");
    Console.print(transmision);
    Console.println("*/");
    
    // Specify the address of the remote XBee (this is the SH + SL) 40F4EE73
    XBeeAddress64 Broadcast = XBeeAddress64(0x00000000, 0x0000ffff);
    
    // Create a TX Request
    ZBTxRequest zbTx = ZBTxRequest(Broadcast, (uint8_t *)transmision, sizeof(transmision));
    
    // Send your request
    xbee.send(zbTx);

	send_request(out);
	wait_response();
	read_response();

}








void send_request(String jsonStr){
  return;
  char bufferSv[64];
	Console.println("connecting");
	if (client.connect(server, 80)) {
		Console.print("log2server");
    Console.println(jsonStr);
		// POST URI
		client.println("POST /JADE/jadeParser.php HTTP/1.1");

		// Host header
		sprintf(bufferSv, "Host: %s", server);
		client.println(bufferSv);

		client.println("Connection: close");
		client.println("Content-Type: application/x-www-form-urlencoded");
		
		client.println("Content-Length: 108");
		// End of headers
		client.println();
		
		String petition = "json=";
		petition.concat(jsonStr);
		// Request body
		client.println(petition);

	} else {
		Console.println("connection failed"); 
	}
}



/* Wait for a response */
void wait_response(){
	while (!client.available()) {
		if (!client.connected()) {
			return;
		} 
	}
}

 /* Read the response and output to the serial monitor */
void read_response(){
	bool print = true;

	while (client.available()) {
		char c = client.read();
		// Print only until the first carriage return
		if (c == '\n')
		print = false;
		if (print)
		Console.print(c);
	}
}

void end_request(){
	client.stop();
}


String GetHour (void) {
  Process time;
  String junkString = "";
  time.runShellCommand("date +\"%T\"");
  while(time.available()) {
    char c = time.read();
    junkString += c;
  }
  junkString = junkString.substring(0,junkString.length()-1);
  return(junkString);
} 

 String GetDateAndTime (void) {
  Process time;
  String junkString = "";
  
  time.runShellCommand("date +\"%F %T\"");
  while(time.available()) {
    char c = time.read();
    junkString += c;
  }
  junkString = junkString.substring(0,junkString.length()-1);
  return(junkString);
} 
