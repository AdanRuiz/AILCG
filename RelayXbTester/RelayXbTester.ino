// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include <Arduino.h>
#include <Wire.h>
#include <XBee.h>
#include <SoftwareSerial.h>

XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
ZBRxResponse rx = ZBRxResponse();
SoftwareSerial nss(2, 3);


int overrideRelay1 = 0;
int overrideRelay2 = 0;
int overrideRelay3 = 0;
int pinRelay1 = 6;//agua
int pinRelay2 = 7;//aire
int pinRelay3 = 8;//calentador

/*
ROJO R1  AGUA 6
AZUL R2 AIRE 7
VERDE R3 CALENTADOR 8
CAFE R4
BLANCO VCC
NEGRO TIERRA
*/

String r1s ="";
String r2s ="";
String r3s ="";
String errors = "";

int msPump = 0;
boolean pumpOn = false;

float minWaterTemperature = 20.00f;
float waterTemperature = 0.00f;

unsigned int valorAnalogico = 0;         // variable para guardar el valor leido del sensor
unsigned int valorLuzPWM = 0;

void setup () {
  pinMode(pinRelay1, OUTPUT);
  pinMode(pinRelay2, OUTPUT);
  pinMode(pinRelay3, OUTPUT);

  //Apaga relevadores
  digitalWrite(pinRelay1, HIGH);
  digitalWrite(pinRelay2, HIGH);
  digitalWrite(pinRelay3, HIGH);
  
  Serial.begin(9600);
  nss.begin(9600);
  xbee.setSerial(nss);

  delay(1000);
}

void loop () {
	if(millis() % 3000 == 0 ){
		errors="";//reset variable each iteration

		String out="{XB:1,data:tester3}";


		
		transmit(out);
	}
	
    //readZb();
}



/* airPumpControl()
    Permite el control automatico y manual de la bomba de aire
    mediante el uso de overrideRelay2:
      0 Automatico
      1 On
      2 Off
    Cuando es automatico se enciende durante los primeros 29 minutos

*
void airPumpControl(){
  //aire
        if( now.minute() < 59 && overrideRelay2 == 0){
           digitalWrite(pinRelay2, LOW);
           r2s="on";
        }else if(overrideRelay2 == 1){
          digitalWrite(pinRelay2, LOW);
           r2s="on";
        }else{
          digitalWrite(pinRelay2, HIGH);
          r2s="off";
        }
}

/* waterControl()
		Permite el control automatico y manual de la temperatura del agua
		mediante el uso de overrideRelay3:
			0 Automatico
			1 On
			2 Off
		Cuando es automatico se toman en cuenta los umbrales de 
		temperatura minima.

*
void waterTemperatureControl(){
	//water temperature controller
        int cMinute = now.minute();
        int cSecond= now.second();
		    waterTemperature = getWaterTemperature();
        if(overrideRelay3 == 1){       
          digitalWrite(pinRelay3, LOW); 
          r3s="on";
        }else if(overrideRelay3 == 2){
          digitalWrite(pinRelay3, HIGH);  
          r3s="off";      
        }else if( cMinute % 5 == 0 && cSecond % 59 == 0 ){         
            if(waterTemperature > -10 && waterTemperature < minWaterTemperature){
               digitalWrite(pinRelay3, LOW);   
               r3s = "on" ;   
            }else{
              digitalWrite(pinRelay3, HIGH);  
              r3s="off";
            }  
        }    
      
}

/* waterPumpControl()
    Permite el control automatico y manual de la bomba de agua
    mediante el uso de overrideRelay1:
      0 Automatico
      1 On
      2 Off
    Cuando es automatico se enciende al minuto 35 
    con una duracion de 360ciclos de 10 segundos (3600 segundos).

*
void waterPumpControl(){
  //BOMBA AGUA
       if(overrideRelay1 == 2){
          digitalWrite(pinRelay1, HIGH); 
          r1s = "off";       
       }else if(overrideRelay1 == 1){       
          digitalWrite(pinRelay1, LOW); 
          r1s="on";
       }else{
            int cMinute = now.minute();
            if(  cMinute >= 0 &&  cMinute < 3 ){ //cada minuto 35 encender bomba agua
                           
             /* if( msPump < 360){
                Serial.print(" P: on");
                digitalWrite(pinRelay1, LOW);
                r1s="on";
              }else if(msPump >= 360 ){
                  Serial.print(" P: off");
                  digitalWrite(pinRelay1, HIGH);
                  r1s="off";
              }
              msPump=msPump+1;*
              digitalWrite(pinRelay1, LOW);
              r1s="on";
            }else{ 
              pumpOn=false;
              digitalWrite(pinRelay1, HIGH);
              r1s="off";
              msPump=0;
            }
            
       }
}


/* float getTemp()
    Este metodo devuelve el valor de la temperatura
*
float getWaterTemperature(){
    byte data[12];
    byte addr[8];
    
    if ( !ds.search(addr)) {
        //no more sensors on chain, reset search
        ds.reset_search();
        return -1000;
    }
    
    if ( OneWire::crc8( addr, 7) != addr[7]) {
    errors.concat("CRC is not valid!");
    return -1000;
    }
    
    if ( addr[0] != 0x10 && addr[0] != 0x28) {
    errors.concat("Device is not recognized");
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

}//getTemp()*


void readZb() {
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
}//readZB



void handleXbeeRxMessage(uint8_t *data, uint8_t length){
    char myCharArray[length];
    for (int i = 0; i < length; i++){
      myCharArray[i]=data[i];
      //Serial.write(data[i]);
    }
    String myString (myCharArray);
    Serial.println(myString);
    Serial.println(); 
    
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
}*/

void transmit(String out){
    int str_len = out.length() + 1; 
    char transmision [str_len];
    out.toCharArray(transmision,str_len);

    Serial.println(str_len);
    Serial.println(transmision);
    
    // Specify the address of the remote XBee (this is the SH + SL) 40F4EE73
    XBeeAddress64 Broadcast = XBeeAddress64(0x00000000, 0x0000ffff);
    
    // Create a TX Request
    ZBTxRequest zbTx = ZBTxRequest(Broadcast, (uint8_t *)transmision, sizeof(transmision));
    
    // Send your request
    xbee.send(zbTx);

}
