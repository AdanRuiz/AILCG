const int ledPin = 9; // the pin that the LED is attached to
int incomingByte;      // a variable to read incoming serial data into



void setup() {
  // initialize serial communication:
  Serial.begin(9600);
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
}

void loop() {
 
   if (Serial.available())
  {
   int f = Serial.parseInt();  
   /*if (f > 0)
   {*/
     //Serial.println(f);
       analogWrite(ledPin, f);
   //} 
  } 
  

}
