#include <painlessMesh.h>
#define   LED             D8       // GPIO number of connected LED, ON ESP-12 IS GPIO2

#define   BLINK_PERIOD    3000 // milliseconds until cycle repeat
#define   BLINK_DURATION  100  // milliseconds LED is on for

#define   MESH_SSID       "NatureBeats"
#define   MESH_PASSWORD   "naturebeatscompany"
#define   MESH_PORT       5555

// Prototypes
//void sendMessage(); 
void receivedCallback(uint32_t from, String & msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback(); 
void nodeTimeAdjustedCallback(int32_t offset); 
void delayReceivedCallback(uint32_t from, int32_t delay);

Scheduler     userScheduler; // to control your personal task
painlessMesh  mesh;

bool calc_delay = false;
SimpleList<uint32_t> nodes;

//void sendMessage() ; // Prototype
//Task taskSendMessage( TASK_SECOND * 1, TASK_FOREVER, &sendMessage ); // start with a one second interval

// Task to blink the number of nodes
Task blinkNoNodes;
bool onFlag = false;


int frequency = 500;
//#define buzz D8
#define light  D0
#define temp D1
#define humid D2
#define readPin A0

void setup() {
  Serial.begin(115200);
  
  pinMode(temp, OUTPUT);
  pinMode(light, OUTPUT);
  pinMode(humid, OUTPUT);

  pinMode(LED, OUTPUT);
//  mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);

  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  mesh.onNodeDelayReceived(&delayReceivedCallback);

//  userScheduler.addTask( taskSendMessage );
//  taskSendMessage.enable();

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
void tone(uint8_t _pin, unsigned int frequency, unsigned long duration) {
  pinMode (_pin, OUTPUT );
  analogWriteFreq(frequency);
  analogWrite(_pin,500);
}
void analogReadSensor() {
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& paramsJSON = jsonBuffer.createObject();
    
    digitalWrite(temp, HIGH);
    digitalWrite(light, LOW);
    digitalWrite(humid, LOW);
    paramsJSON["temperature"] = analogRead(readPin);
    
    digitalWrite(temp, LOW);
    digitalWrite(light, HIGH);
    digitalWrite(humid, LOW);
    paramsJSON["light"] = analogRead(readPin);
    
    digitalWrite(temp, LOW);
    digitalWrite(light, LOW);
    digitalWrite(humid, HIGH);
    paramsJSON["humidity"] = analogRead(readPin);
    
    String jsonStr = "";
    Serial.print("Send values: ");
    paramsJSON.printTo(jsonStr);
    mesh.sendBroadcast(jsonStr);
    
    Serial.print(jsonStr);
    Serial.println();    
  }
void loop() {
    userScheduler.execute(); // it will run mesh scheduler as well
  mesh.update();
  digitalWrite(LED, !onFlag);
  analogReadSensor();
  delay(2000);
//    tone(buzz, 500, 1000);
}

void receivedCallback(uint32_t from, String & msg) {
  Serial.printf("%u: %s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
  // Reset blink task
  onFlag = false;
  blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
  blinkNoNodes.enableDelayed(BLINK_PERIOD - (mesh.getNodeTime() % (BLINK_PERIOD*1000))/1000);
 
//  Serial.printf("New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
//  Serial.printf("Changed connections %s\n", mesh.subConnectionJson().c_str());
  // Reset blink task
  onFlag = false;
  blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
  blinkNoNodes.enableDelayed(BLINK_PERIOD - (mesh.getNodeTime() % (BLINK_PERIOD*1000))/1000);
 
  nodes = mesh.getNodeList();

  Serial.printf("Num nodes: %d\n", nodes.size());
//  Serial.printf("Connection list:");

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
//void sendMessage() {
//  String msg = "Hello from node no. ";
//  msg += mesh.getNodeId();
//  mesh.sendBroadcast(msg);
//
//  if (calc_delay) {
//    SimpleList<uint32_t>::iterator node = nodes.begin();
//    while (node != nodes.end()) {
//      mesh.startDelayMeas(*node);
//      node++;
//    }
//    calc_delay = false;
//  }
//
//  Serial.printf("Send message: %s\n", msg.c_str());
//  
//  taskSendMessage.setInterval( random(TASK_SECOND * 1, TASK_SECOND * 5));  // between 1 and 5 seconds
//}
