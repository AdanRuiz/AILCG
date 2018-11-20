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
#include <LinkedList.h>
 int xbeeNumber=4;
LinkedList<String> xbeeList[4];
//LinkedList[] houses = InitializeArray<LinkedList>(200);

//LinkedList<String> myList = LinkedList<String>();

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
int wpc=0;
int wpt=5;
int wpd=10;

String xbeeMessages[][2]={
  {"11","12"},
  {"21","22"},
  {"31","32"},
  {"41","42"}
};


int lc = 0;
int lpctled = 100;



String getJsonValue(String search, String json){
    Console.print("Search");
    Console.print(search);
    Console.print(",JSON:");
    Console.println(json);
    int lastIndexSearch = json.lastIndexOf(search);
    String afterIndex = json.substring(lastIndexSearch);
    Console.println(afterIndex);
    int valueIndex = afterIndex.indexOf(':');
    Console.println(valueIndex);
    /*String valueWithTrash = afterIndex.substring(0,valueIndex);
    Serial.print(valueWithTrash);Serial.print('=');*/
    int commaIndex = afterIndex.indexOf(',');
    Console.println(commaIndex);
    String wpValue = afterIndex.substring(valueIndex+1,commaIndex);

    if(wpValue==""){
        Console.println("Vacio");  
        wpValue="99";
    }
       
    return wpValue;
}


void printMatrixXbee(){
  return;
  Console.println("--- Matrix ---");
  for(int i=0;i<4;i++){
    for(int j=0;j<2;j++){
      Console.print(xbeeMessages[i][j]);
      Console.print(" ");
    }//j
    Console.println(",");
  }//i
  Console.println("--- ---");
}
/*
void publishXbeesInfo(){
	for(int i=0;i<4;i++){
		String labelTmp ="XB";
		labelTmp.concat(i);
    String pub = xbeeMessages[i][0];
    pub.concat(xbeeMessages[i][1]);
    Console.print("Publish:");
    Console.print("XB");
    Console.print(i);
    Console.print(" ");
    Console.println(pub);
		Bridge.put(labelTmp,pub);
	}
}*/

void initXbeeList(){
  for(int i=0;i< xbeeNumber;i++){
    xbeeList[i] =  LinkedList<String>();
    //String lblTmp2= String(i);
    //lblTmp2.concat("1,");
    xbeeList[i].add("");
    //String lblTmp3=String(i);
    //lblTmp3.concat("2,");
    xbeeList[i].add("");
  }
}
void publishXbeesInfo(){
  for(int i=0;i< xbeeNumber;i++){
      int listSize = xbeeList[i].size();
      String labelTmp ="XB";
      labelTmp.concat(i);
      String xbValues="";
      for (int h = 0; h < listSize; h++) {
        String val = xbeeList[i].get(h);
        xbValues.concat(val);
        
      }
      Console.println(xbValues);
      Bridge.put(labelTmp,xbValues);
  }
}
void getXbeeList(){
  return;
  for(int i=0;i< xbeeNumber;i++){
      int listSize = xbeeList[i].size();
      for (int h = 0; h < listSize; h++) {
        String val = xbeeList[i].get(h);
        Console.print(val);
        Console.print(" ");
      }
      Console.println();
  }
}

void setup() { 
    
    Bridge.begin();
    Console.begin();
    nss.begin(9600);
    xbee.setSerial(nss);
  
    Console.println("starting up");

    Process date;
    date.runShellCommand("ntpd -qn -p 10.0.5.220");
    Console.println("Updated Time");
    Bridge.put("WPC", String(wpc));
    Bridge.put("WPD", String(wpd));
    Bridge.put("WPT", String(wpt));
    Bridge.put("LC", String(lc));
    Bridge.put("LPCTLED", String(lpctled));
    publishXbeesInfo();

    initXbeeList();
    getXbeeList();
    notify();
    
    Console.println("End Setup");
}

long lastNotify=0;


int getIntBridge(char * searchKey, int digits){
      char wptChar[digits];
      Bridge.get(searchKey,wptChar,digits);
      String temp (wptChar);
      return temp.toInt();
}

void notify(){
        lastNotify = millis();
        String date = GetDateAndTime();
        String hourStr = GetHour() ;
        int hourInt = hourStr.toInt();

        
        int ledStatus = 0;
    		if(lc == 1){
    			ledStatus = lpctled *255 /100;
    		}else if(lc == 2){
    			lpctled = 0;
    		}else{
    			ledStatus = 255;
    		}
        
        String out= "{'XB':0,'PWM':";
        out.concat(ledStatus);
        out.concat(",'LC':");
        out.concat(lc);
        out.concat(",'WPC':");
        out.concat(wpc);
        out.concat(",'WPT':");
        out.concat(wpt);
        out.concat(",'WPD':");
        out.concat(wpd);
        out.concat("}");

        xbeeList[0].set(0,out);
        
        transmit(out);
}


void loop() {

    if(millis()%15000==0){
      wpc = getIntBridge("WPC",2);
      wpd = getIntBridge("WPD",2);
      wpt = getIntBridge("WPT",2);
      lc = getIntBridge("LC",2);
      lpctled = getIntBridge("LPCTLED",3);
      
    }

    if(millis() % 2000 == 0){
        publishXbeesInfo();
        getXbeeList();
    }
    
    if(millis() % 59000 == 0){
        notify();
    }

    /*String minuteStr = GetMinute() ;
    int minuteInt = minuteStr.toInt();
   
    if( minuteInt % 30 == 0 ){
        Process date;
        date.runShellCommand("ntpd -qn -p 10.0.5.220");
    }*/

    
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
      
    }else if (xbee.getResponse().isError()) {
      // some kind of error happened, I put the stars in so
      // it could easily be found
      Console.print("***error code:");
      Console.println(xbee.getResponse().getErrorCode(),DEC);
    }else {
      // I hate else statements that don't have some kind
      // ending.  This is where you handle other things
    }
    
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
    int cXbee = 99;
  	cXbee = getJsonValue("XB", myString).toInt();
    int cXbeeMsgPart = 0;
    cXbeeMsgPart = getJsonValue("Part", myString).toInt();
    
    Console.print("xb=");
    Console.print(cXbee);
    Console.print(",p=");
    Console.print(cXbeeMsgPart);
    Console.print(".");
    if(cXbee!=99){
        if(cXbeeMsgPart==99)
            cXbeeMsgPart=0;
        else 
            cXbeeMsgPart-=1;

        xbeeList[cXbee].set(cXbeeMsgPart,myString);
        Console.print("[");
        Console.print(cXbee);
        Console.print("][");
        Console.print(cXbeeMsgPart);
        Console.println("]");
        //xbeeMessages[cXbee][cXbeeMsgPart] = myString;
    }

    printMatrixXbee();


	send_request(myString);
	wait_response();
	read_response();
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



String GetMinute (void) {
  Process time;
  String junkString = "";
  time.runShellCommand("date +\"%T\"");
  while(time.available()) {
    char c = time.read();
    junkString += c;
  }
  junkString = junkString.substring(3,junkString.length()-4);
  return(junkString);
} 
