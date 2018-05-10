#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Banana";
const char* password = "hectorthefly";

SoftwareSerial swSerial(D1, D2, false, 128);

void setup() {
    pinMode(D1, INPUT);
    pinMode(D2, OUTPUT);

    Serial.begin(57600);
    swSerial.begin(57600);
    
  
  
}
String DataString;

void loop() {
  while(swSerial.available() > 0)
  {
//    Serial.println("Connection open");
    char c = swSerial.read();  //gets one byte from serial buffer
    DataString += c; //makes the string DataString    
  } 
  if (DataString  != "") {
    Serial.println(DataString);
  }
  
 
//
  DataString = "";
  delay(1000);
  }
