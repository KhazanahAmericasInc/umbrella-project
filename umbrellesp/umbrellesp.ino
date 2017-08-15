//************************************************************
// this is a simple example that uses the easyMesh library
//
// 1. blinks led once for every node on the mesh
// 2. blink cycle repeats every BLINK_PERIOD
// 3. sends a silly message to every node on the mesh at a random time betweew 1 and 5 seconds
// 4. prints anything it recieves to Serial.print
//
//
//************************************************************
#include <painlessMesh.h>
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include "config.h"

// some gpio pin that is connected to an LED...
// on my rig, this is 5, change to the right number of your LED.
#define   LED             2       // GPIO number of connected LED, ON ESP-12 IS GPIO2

#define   PRINT_PERIOD    2000000 // microseconds until cycle repeat
#define   BLINK_DURATION  500000  // microseconds LED is on for

#define   MESH_SSID       "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

painlessMesh  mesh;
bool calc_delay = false;
SimpleList<uint32_t> nodes;
int highest = 0;
int sent = 0;

void sendMessage() ; // Prototype
Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage ); // start with a one second interval

void setup() {
  digitalWrite(16, HIGH);
  delay(200);
  pinMode(16, OUTPUT);
  
  Serial.begin(115200);

  pinMode(LED, OUTPUT);
  

  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes(ERROR | DEBUG);  // set before init() so that you can see startup messages

  mesh.init(MESH_SSID, MESH_PASSWORD, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  //mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  mesh.onNodeDelayReceived(&delayReceivedCallback);

  //mesh.scheduler.addTask( taskSendMessage );
  //taskSendMessage.enable() ;

//  randomSeed(analogRead(A0));
  ring.begin(); //init ring
  ring.show(); //show nothing
  last_pulse = mesh.getNodeTime(); //set to now
  
}

void loop() {
  mesh.update();
  doBackgroundStuff();

  uint32_t cycleTime = mesh.getNodeTime() % PRINT_PERIOD;
  SimpleList<uint32_t> nodes = mesh.getNodeList();
  uint32_t onTime = BLINK_DURATION;
   
  if (onTime>cycleTime && sent < 1){
    SimpleList<uint32_t>::iterator node = nodes.begin();
    int counter = 0;
    while (node != nodes.end()) {
       mesh.startDelayMeas(*node);
       node++;
       counter++;
    }
    sent++;
    if (counter==0){
      delayReceivedCallback(0, MEDIUM_SIGNAL_AMOUNT+1);
    }
  }else if (onTime<cycleTime){
    sent = 0;
  }
  
}

void doBackgroundStuff() { //merge of updateLight

  int intensity = 0;
  if (current_dist==0&&streak>=STREAK_MAX){
    intensity = intensityarr[int (((mesh.getNodeTime() % 5000000) / 25000))];
  }else{
    intensity = max_intensity;
  }
  /*if ((mesh.getNodeTime() - last_pulse) > ((1 / max_intensity) * 1000000 * PULSE_LENGTH / 2)) { //same logic as sendID()
    //Simple logic to linearly fade in and out.
    if (up && intensity < max_intensity) {
      intensity++;
    } else if (up && intensity >= max_intensity) {
      intensity--;
      up = false;
    } else if (!up && intensity > 0) {
      intensity--;
    } else {
      intensity++;
      up = true;
    }
    last_pulse = mesh.getNodeTime(); //track this too
  }*/

  int r = originalColor.R*(intensity/max_intensity);
  int g = originalColor.G*(intensity/max_intensity);
  int b = originalColor.B*(intensity/max_intensity);
  

  r *= 0.6;
  g *= 0.6;
  b *= 0.6;
  
  for (int i = 0; i < RING_LEDS; i++) {
    ring.setPixelColor(i, r, g, b); //gamma correction
  }
  ring.show(); //update our changes
}

void sendMessage() {
  String msg = "Hello from node ";
  msg += mesh.getNodeId();
  bool error = mesh.sendBroadcast(msg + " myFreeMemory: " + String(ESP.getFreeHeap()));

  if (calc_delay) {
    SimpleList<uint32_t>::iterator node = nodes.begin();
    
    while (node != nodes.end()) {
        mesh.startDelayMeas(*node);
        node++;
    }
    calc_delay = false;
  }

  taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 5 ));  // between 1 and 5 seconds
}


void receivedCallback(uint32_t from, String & msg) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections %s\n", mesh.subConnectionJson().c_str());

  nodes = mesh.getNodeList();

  Serial.printf("Num nodes: %d\n", nodes.size());
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
  
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void delayReceivedCallback(uint32_t from, int32_t delay) {
  Serial.print(current_dist);
   Serial.printf("Delay to node %u is %d us\n", from, delay);
  int dist = 0;
  if (delay<CLOSE_SIGNAL_AMOUNT){
    dist = 0;
  }else if (delay<MEDIUM_SIGNAL_AMOUNT){
    dist = 1;
  }else if (delay==NULL){
    dist = 2;
  }else{
    dist = 2;
  }
  checkStreak(dist);
}

void checkStreak(int dist){
  if (dist!=current_dist){
    streak = 0;
    current_dist = dist;
  }else{
    streak++;
    if (streak>=STREAK_MAX){
      adjustLight(dist);
      if (current_dist==2){
        digitalWrite(16, LOW);
      }
    }
  }
}

void adjustLight(int dist){
  current_dist = dist;
  if (dist==0){
    max_intensity = CLOSE_INTENSITY;
  }else if (dist==1){
    max_intensity = MEDIUM_INTENSITY;
  }else{
    max_intensity = FAR_INTENSITY;
  }
}




