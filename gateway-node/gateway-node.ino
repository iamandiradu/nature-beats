#include <ArduinoJson.h>

#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define soundTrigID "5af4a15759163605c19d76bf"

#include "UbidotsMicroESP8266.h"
#define TOKEN  "A1E-SyggO1V3EdZI5xZLEW5MQ3WIzboBNJ"  // Put here your Ubidots TOKEN
#define WIFISSID "Banana"
#define PASSWORD "hectorthefly"


Ubidots client(TOKEN);
byte soundTrig = 0;
SoftwareSerial swSerial(D1, D2, false, 128);

void setup() {
    pinMode(D1, INPUT);
    pinMode(D2, OUTPUT);

    Serial.begin(57600);
    swSerial.begin(57600);
    
    client.wifiConnection(WIFISSID, PASSWORD);
    client.setDataSourceName("Nature Beats");
    client.setDataSourceLabel("Nature Beats Prototype 2");
  
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
    sendData(DataString);
  }
  if(soundTrig == 1) {
    Serial.println(" Sound activated");
    swSerial.print(soundTrig);
  } else if(soundTrig == 0){
    Serial.println(" Sound deactivated");
    swSerial.print(soundTrig);
  }
  Serial.println(soundTrig);
  
//
  DataString = "";
  delay(1000);
  }

 void sendData(String data) {
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(data); 
    float temp = root["temperature"];
    float light = root["light"];
    float humidity = root["humidity"];
//    const soundTrigger = root["sound"];
    soundTrig = client.getValue(soundTrigID);
    client.add("Temperature", temp);
    client.add("Luminosity", light);
    client.add("Humidity", humidity);
    client.add("Sound", soundTrig);

    client.sendAll(true);
 }

