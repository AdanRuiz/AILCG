/**
UFCommonTrayXbeeRxTx

Libray to control a Urban Farming Test Bed

Dependencies
      Andrew Rapp XBEE Library
	  Arduinos Default SoftwareSerial 
	  OneWire
	  RTC Lib

Things to Control :
	Lights
Things to Log : 
	Date
	Temperature
	Humidity
	LED Duty Cycle (pwm)
	
	Autor: Adan Ruiz
	Email: aruiz@cinvestav.mx
	       ardn0001@hotmail.com
*/
 
#include <XBee.h>
#include <SoftwareSerial.h>
#include "DHT.h"
#include <OneWire.h>
#include "RTClib.h"

OneWire  ds(5);

#define DHTPIN 4     // what digital pin we're connected to
#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE); //required by newer versions of DHT

RTC_Millis RTC;  // Real Time Clock library (needed to turn on the water pump)
DateTime now;


XBee xbee = XBee(); //Communications Library
XBeeResponse response = XBeeResponse();
// create reusable response objects for responses we expect to handle
ZBRxResponse rx = ZBRxResponse();

/* Define NewSoftSerial TX/RX pins
// Connect Arduino pin 2 to Tx and 3 to Rx of the XBee
// I know this sounds weird, but remember that default arduino Rx/Tx is
// used also to program Arduino Uno, this is a conflict because those instructions
// alter the Xbee behaviour, so it is better to isolate the communications pins 
// between the Arduino and the Xbee shield.
*/
#define ssRX 2
#define ssTX 3
SoftwareSerial nss(ssRX, ssTX);

int ledPin = 9;     //Indicates the output pin of LED bars 
int atxPin = 14;    //Turns on/off the source power of LED bars

int pwmLight = 0;   //Indicates the duty cicle of LED lights

String lucesStatus="off";  //Debug var to indicate the led status on/off

/*
//  When coordinator goes off or unavailable, systems becomes autonomous 
//  so leds should be off, for that behaviour lastMsgLightMinute is used
*/
int lastMsgLightMinute=0;  //Register that indicates when was received the last instruction to turn on/off the lights from the xbee coordinator
int lMinute=0;             // Indicates the current minute
long lastNotifyValue =0, timeNow=0; //Vars used to repeat the basic functionality every 8seconds

void setup() { 
    //RTC.adjust(DateTime(2016, 1, 1, 10, 10, 0));
    pinMode(ledPin, OUTPUT);
    pinMode(atxPin, OUTPUT);
    // start serial
    Serial.begin(9600);
    nss.begin(9600);
    xbee.setSerial(nss);
  
    Serial.println("starting up");
  
    dht.begin();
    lastNotifyValue = millis();
}

void loop() {
    
    timeNow = millis();
    if(timeNow % 8000 == 0 || timeNow - lastNotifyValue > 8000){ //used to repeat the basic functionality every 8seconds
        lastNotifyValue = timeNow;
        now = RTC.now();
        lMinute=now.minute();
        lightsControlZb();                                       //method to control the lights considering the XBee Communications messages
        
        
        // Read relative humidity (%)
        float h = dht.readHumidity();
        // Read temperature as Celsius (the default)
        float t = dht.readTemperature();
        
		// Building 1st Message using a Xbee output string (for log and multi-agent purposes)
        String out= "{'XB':3,'Part':1,";
        out.concat("'H':");
        out.concat(h);
        out.concat(",'T':");
        out.concat(t);
        out.concat("}");
        transmit(out); //transmits 1st msg using xbee
        
		// Building 2nd Message using a Xbee output string (for log and multi-agent purposes)        
        String out2= "{'XB':3,'Part':2,";
        out2.concat("'WT':");
        out2.concat(getTemp());
        out2.concat(",'PWM':");
        out2.concat(pwmLight);
        out2.concat("}");
        transmit(out2);//transmits 2nd msg using xbee
        
    }

    
    // doing the read without a timer makes it non-blocking, so
    // you can do other stuff in loop() as well.
    xbee.readPacket();
    // so the read above will set the available up to
    // work when you check it.
    if (xbee.getResponse().isAvailable()) {
      // got something
      Serial.println();
      Serial.print("Frame Type is ");
      // Andrew call the frame type ApiId, it's the first byte
      // of the frame specific data in the packet.
      Serial.println(xbee.getResponse().getApiId(), HEX);
    
      if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
        // got a zb rx packet, the kind this code is looking for
      
        // now that you know it's a receive packet
        // fill in the values
        xbee.getResponse().getZBRxResponse(rx);
      
        // this is how you get the 64 bit address out of
        // the incoming packet so you know which device
        // it came from
        Serial.print("Got an rx packet from: ");
        XBeeAddress64 senderLongAddress = rx.getRemoteAddress64();
        print32Bits(senderLongAddress.getMsb());
        Serial.print(" ");
        print32Bits(senderLongAddress.getLsb());
      
        // this is how to get the sender's
        // 16 bit address and show it
        uint16_t senderShortAddress = rx.getRemoteAddress16();
        Serial.print(" (");
        print16Bits(senderShortAddress);
        Serial.println(")");
      
        // The option byte is a bit field
        if (rx.getOption() & ZB_PACKET_ACKNOWLEDGED)
            // the sender got an ACK
          Serial.println("packet acknowledged");
        if (rx.getOption() & ZB_BROADCAST_PACKET)
          // This was a broadcast packet
          Serial.println("broadcast Packet");
        
        Serial.print("checksum is ");
        Serial.println(rx.getChecksum(), HEX);
      
        // this is the packet length
        Serial.print("packet length is ");
        Serial.print(rx.getPacketLength(), DEC);
      
        // this is the payload length, probably
        // what you actually want to use
        Serial.print(", data payload length is ");
        Serial.println(rx.getDataLength(),DEC);
      
        // this is the actual data you sent
        Serial.println("Received Data: ");
        for (int i = 0; i < rx.getDataLength(); i++) {
          print8Bits(rx.getData()[i]);
          Serial.print(' ');
        }
      
        // and an ascii representation for those of us
        // that send text through the XBee
        Serial.println();
        for (int i= 0; i < rx.getDataLength(); i++){
          Serial.write(' ');
          if (iscntrl(rx.getData()[i]))
            Serial.write(' ');
          else
            Serial.write(rx.getData()[i]);
          Serial.write(' ');
        }
        Serial.println();
      
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
      Serial.print("************************************* error code:");
      Serial.println(xbee.getResponse().getErrorCode(),DEC);
    }
    else {
      // I hate else statements that don't have some kind
      // ending.  This is where you handle other things
    }
}

// this is just a stub to show how to get the XBee messages
void handleXbeeRxMessage(uint8_t *data, uint8_t length){

    //1. Build a String from bytes
    char myCharArray[length];
    for (int i = 0; i < length; i++){
      myCharArray[i]=data[i];
      //Serial.write(data[i]);
    }
    String myString (myCharArray);
    Serial.println("******************REC DEBUG***********************");
    Serial.println(myString);
    Serial.println();
	//2. Parse Message
    myString.replace("{","");
    myString.replace("}","");
    int commaIndex = myString.indexOf(','); //PRIMERA COMA
    String firstParameter = myString.substring(0, commaIndex); // INICIO A LA PRIMERA COMA {XB:0
    int separatorIndex = myString.indexOf(':');
    String firstKey = myString.substring(0, separatorIndex); // INICIO AL 
    String firstValue = myString.substring(separatorIndex+1,commaIndex);
    Serial.print(firstKey);
    Serial.print('=');
    Serial.println(firstValue);
	//3. Identify var to read and assign value to current arduino variable
    if(firstKey.equals("'XB'") && firstValue.equals("0")){
        int pwmIndex = myString.lastIndexOf("'PWM'");
        String pwmNoLimit = myString.substring(pwmIndex);
        Serial.println(pwmNoLimit);
        int pwmKeyIndex = pwmNoLimit.indexOf(':');
        String pwmKey = pwmNoLimit.substring(0,pwmKeyIndex);
        Serial.print(pwmKey);Serial.print('=');
        int pwmCommaIndex = pwmNoLimit.indexOf(',');
        String pwmValue = pwmNoLimit.substring(pwmKeyIndex+1,pwmCommaIndex);
        pwmLight = pwmValue.toInt();
        Serial.println(pwmLight);


        lastMsgLightMinute=now.minute();
        
    }
   Serial.println("******************////////REC DEBUG***********************");
    
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
    Serial.write(nibble + 0x30);
  else
    Serial.write(nibble + 0x37);
      
  nibble = (uint8_t) (c & 0x0F);
  if (nibble <= 9)
    Serial.write(nibble + 0x30);
  else
    Serial.write(nibble + 0x37);
}

/* lightsControl()
    Control automatico de las luces en el pin del relay2:
    Se enciende a las 7am y apaga a las 9pm

*/
void lightsControlClock(){
  //LUZ
        if(now.hour()>=7 && now.hour()<=22 ){ 
          digitalWrite(atxPin, LOW);
          lucesStatus = "on";
        }else{
           digitalWrite(atxPin, HIGH);
           lucesStatus = "off";
        }
}

/* lightsControlZb()
    Control automatico de las luces en el pin del relay2 que considera 
	los mensajes del coordinador de zigbee

*/
void lightsControlZb(){
	//when was received the last message, if it is too old turn off the light
	if(lMinute > lastMsgLightMinute && millis()%1000){ //millis()%1000 instruction limited to be executed every second (just avoiding overhead of instructions to microcontroller)
	//1 case, when current minute bigger than the last xbee received msg,
     // lMinute  lastMsgLightMinute        DIFFERENCE
	// 30              29                      1
	// 31              29                      2
	// ...          ...                     ..
 	// 33              29                      4
 	// 34              29                      5         <--------- IF DIFFERENCE IS bigger THAN 5 TURN OFF
         if(lMinute - lastMsgLightMinute > 5 ){        //difference of 5 minutes! turn off
              pwmLight=0;
              Serial.print("YUN EXPIRED LIGHT1 MinuteNow ");       
              Serial.print(lMinute);
              Serial.print(" lastmsg:");
              Serial.println(lastMsgLightMinute);  
         }
	
    }else if(lMinute < lastMsgLightMinute && millis()%1000){//2nd case, when current minute less than the last xbee received msg. 
	//Imagine message was received 1 min ago at minute 59(lastMsgLightMinute) but now the current minute is 0(lminute) should be on not off
	// lMinute  lastMsgLightMinute        DIFFERENCE
	// 0              59                      59
	// 1              59                      58
	// 2              59                      57
	// ...          ...                     ..
 	// 4              59                      55
 	// 5              59                      54         <--------- IF DIFFERENCE IS 54 O LESS LED BAR IS OFF
         if(lastMsgLightMinute - lMinute < 55){// 5 minutes of tolerance so 59 -4 = 53
              pwmLight=0;
              Serial.print("YUN EXPIRED LIGHT2 MinuteNow ");       
              Serial.print(lMinute);
              Serial.print(" lastmsg:");
              Serial.println(lastMsgLightMinute);
         }
    }
	//ON/OFF CONTROL (I Forget to comment but still works uncommented si I leave it as is)
    if(pwmLight > 100){ 
        digitalWrite(atxPin, LOW);
        lucesStatus = "on";
    }else{
         digitalWrite(atxPin, HIGH);
         lucesStatus = "off";
    };
	
	
	//PWM CONTROL OF LIGHT
	analogWrite(ledPin,pwmLight);
}


/* float getTemp()
    Este metodo devuelve el valor de la temperatura ds18b20
*/
float getTemp(){
    byte data[12];
    byte addr[8];
    
    if ( !ds.search(addr)) {
        //no more sensors on chain, reset search
        ds.reset_search();
        return -1000;
    }
    
    if ( OneWire::crc8( addr, 7) != addr[7]) {
    Serial.println("CRC is not valid!");
    return -1000;
    }
    
    if ( addr[0] != 0x10 && addr[0] != 0x28) {
    Serial.print("Device is not recognized");
    return -1000;
    }
    
    ds.reset();
    ds.select(addr);
    ds.write(0x44,1); 
    
    byte present = ds.reset();
    ds.select(addr); 
    ds.write(0xBE); 
    
    
    for (int i = 0; i < 9; i++) { 
    data[i] = ds.read();
    }
    
    ds.reset_search();
    
    byte MSB = data[1];
    byte LSB = data[0];
    
    float TRead = ((MSB << 8) | LSB); 
    float Temperature = TRead / 16;
    
    return Temperature;

}//getTemp()




void transmit(String out){
    int str_len = out.length() + 1; 
    char transmision [str_len];
    out.toCharArray(transmision,str_len);

    Serial.println(str_len);
    Serial.println(transmision);
    
    // Specify the address of the remote XBee (this is the SH + SL) 40F4EE73
    //0013a200
    //40f4ee78
    XBeeAddress64 Broadcast = XBeeAddress64(0x0013A200, 0x40F4EE6F);
    
    // Create a TX Request
    ZBTxRequest zbTx = ZBTxRequest(Broadcast, (uint8_t *)transmision, sizeof(transmision));
    
    // Send your request
    xbee.send(zbTx);

}
