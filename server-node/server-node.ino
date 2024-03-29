#include <painlessMesh.h>
#include <WiFiClient.h>
#include <SoftwareSerial.h>

// some gpio pin that is connected to an LED...
// on my rig, this is 5, change to the right number of your LED.
#define   LED             D2       // GPIO number of connected LED, ON ESP-12 IS GPIO2

#define   BLINK_PERIOD    3000 // milliseconds until cycle repeat
#define   BLINK_DURATION  100  // milliseconds LED is on for

#define   MESH_SSID       "NatureBeats"
#define   MESH_PASSWORD   "naturebeatscompany"
#define   MESH_PORT       5555

// Prototypes
void sendMessage(); 
void receivedCallback(uint32_t from, String & msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback(); 
void nodeTimeAdjustedCallback(int32_t offset); 
void delayReceivedCallback(uint32_t from, int32_t delay);

SoftwareSerial swSerial ( D1, D2, false , 128 );

Scheduler     userScheduler; // to control your personal task
painlessMesh  mesh;

bool calc_delay = false;
SimpleList<uint32_t> nodes;

void sendMessage() ; // Prototype
Task taskSendMessage( TASK_SECOND * 1, TASK_FOREVER, &sendMessage ); // start with a one second interval

// Task to blink the number of nodes
Task blinkNoNodes;
bool onFlag = false;

void setup() {
  Serial.begin(57600);
  swSerial.begin(57600) ;
  
  pinMode(D1, INPUT ) ;
  pinMode(D2, OUTPUT ) ;
  pinMode(LED, OUTPUT);

//  mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  //mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION | COMMUNICATION);  // set before init() so that you can see startup messages
//  mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);  // set before init() so that you can see startup messages

  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  mesh.onNodeDelayReceived(&delayReceivedCallback);

  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();

  blinkNoNodes.set(BLINK_PERIOD, (mesh.getNodeList().size() + 1) * 2, []() {
      // If on, switch off, else switch on
      if (onFlag)
        onFlag = false;
      else
        onFlag = true;
      blinkNoNodes.delay(BLINK_DURATION);

      if (blinkNoNodes.isLastIteration()) {
        // Finished blinking. Reset task for next run 
        // blink number of nodes (including this node) times
        blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
        // Calculate delay based on current mesh time and BLINK_PERIOD
        // This results in blinks between nodes being synced
        blinkNoNodes.enableDelayed(BLINK_PERIOD - 
            (mesh.getNodeTime() % (BLINK_PERIOD*1000))/1000);
      }
  });
  userScheduler.addTask(blinkNoNodes);
  blinkNoNodes.enable();

  randomSeed(analogRead(A0));
}
String DataString;
String data;
void loop() {
  userScheduler.execute(); // it will run mesh scheduler as well
  mesh.update();
  if(swSerial.available() > 0) {
    sendDataToGateway(data);

     char c = swSerial.read();  //gets one byte from serial buffer
      DataString += c;
  }
  if (DataString  != "") {
    Serial.println(DataString);
    if(DataString == "1") {
      //signal activate
      activateSound();
    } else if(DataString == "0") {
      //signal activate
      deactivateSound();
    }
    
  }
//  Serial.print(data);
  digitalWrite(LED, !onFlag);
  DataString = "";
  delay(1000);
}

void receivedCallback(uint32_t from, String & msg) {
//  Serial.printf("Received: Node no. %u: %s\n", from, msg.c_str());
  data = msg.c_str();
}

void sendDataToGateway (String data) {
//    Serial.print("Send to server: ");
    swSerial.print(data);
    Serial.println(data);
//    Serial.println();
//  while (swSerial.available() > 0) {
////    char c = swSerial.read() ; // lấy một byte từ serial bộ đệm
//  Serial.print(swSerial.read());
////    DataString += c; // makes the string DataString
//  }
}
void activateSound() {
  String activation = "on";
  mesh.sendBroadcast(activation);
  Serial.print(activation);
}
void deactivateSound() {
  String deactivation = "off";
  mesh.sendBroadcast(deactivation);
  Serial.print(deactivation);
}
void sendMessage() {
  String msg = "Central node ";
//  msg += mesh.getNodeId();
  msg += "on duty! ";
//  msg += " freeMemory: " + String(ESP.getFreeHeap());
//  mesh.sendBroadcast(msg);

//  if (calc_delay) {
//    SimpleList<uint32_t>::iterator node = nodes.begin();
//    while (node != nodes.end()) {
//      mesh.startDelayMeas(*node);
//      node++;
//    }
//    calc_delay = false;
//  }

//  Serial.printf("Send: %s\n", msg.c_str());
  
//  taskSendMessage.setInterval(TASK_SECOND * 2);  // 5 seconds
}

void newConnectionCallback(uint32_t nodeId) {
  // Reset blink task
  onFlag = false;
  blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
  blinkNoNodes.enableDelayed(BLINK_PERIOD - (mesh.getNodeTime() % (BLINK_PERIOD*1000))/1000);
 
  Serial.printf("New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections %s\n", mesh.subConnectionJson().c_str());
  // Reset blink task
  onFlag = false;
  blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
  blinkNoNodes.enableDelayed(BLINK_PERIOD - (mesh.getNodeTime() % (BLINK_PERIOD*1000))/1000);
 
  nodes = mesh.getNodeList();

  Serial.printf("No. of nodes: %d\n", nodes.size());
  Serial.printf("Connection list:");

  SimpleList<uint32_t>::iterator node = nodes.begin();
  while (node != nodes.end()) {
    Serial.printf(" %u", *node);
    node++;
  }
  Serial.println();
  calc_delay = true;
}

void nodeTimeAdjustedCallback(int32_t offset) {
//  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void delayReceivedCallback(uint32_t from, int32_t delay) {
//  Serial.printf("Delay to node %u is %d us\n", from, delay);
}
