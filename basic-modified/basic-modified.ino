#include <FastLED.h>
//************************************************************
// this is a simple example that uses the painlessMesh library
//
// 1. sends a silly message to every node on the mesh at a random time betweew 1 and 5 seconds
// 2. prints anything it recieves to Serial.print
//
//
//************************************************************
#include "painlessMesh.h"
#include <ESP8266WiFi.h>
#include "config.h"

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555


painlessMesh  mesh;
int highest = 0;
int sent = 0;
int c = 0;

void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections %s\n", mesh.subConnectionJson().c_str());
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void setup() {
  Serial.begin(115200);

  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION);  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  FastLED.addLeds<NEOPIXEL, RING_PIN>(leds, RING_LEDS);
  for (int i = 0; i < RING_LEDS; i++) {
    leds[i].r = 0;
    leds[i].g = 0;
    leds[i].b = 0;
  }
  for (int i = 0; i<CACHE_SIZE;i++){
    cache[i] = 2; //set to far
  }
}

void loop() {
  mesh.update();
  doBackgroundStuff();
  c++;
  if (c>=300){
      sortSignal(highestRSSI);
      c = 0;
      Serial.println("CHECKING");
      //setup();
  }
}


int toZigZag(int raw){
  if (raw<=100) return raw;
  else return 200-raw;
}

void doBackgroundStuff() { //merge of updateLight

  if (DEBUG_MODE==0) leds_amount = RING_LEDS;
  int intensity = 0;
  if (current_dist==0){
    //intensity = intensityarr[int (((mesh.getNodeTime() % 5000000) / 25000))];
    //intensity = toZigZag((mesh.getNodeTime() % 5000000) / 25000);
    int t = mesh.getNodeTime();
    intensity = intensityarr[(((t % 5000000) / 25000))];
  }else if (highestRSSI>=-101&&DEBUG_MODE==1){
    intensity = 100-(-1*highestRSSI);
    leds_amount = ((intensity/100.0)*32);
    Serial.print("INTENSITY IS NOW ");
    Serial.println(intensity);
    Serial.print("LEDS IS NOW ");
    Serial.println(leds_amount);
  }else{
    intensity = max_intensity;
  }

  

  int r = originalColor.R*(intensity/100.0);
  int g = originalColor.G*(intensity/100.0);
  int b = originalColor.B*(intensity/100.0);
  
  if (DEBUG_MODE==1){
    r = 1;
    g = 1;
    b = 100;
  }
  
  for (int i = 0; i < RING_LEDS; i++) {
    leds[i].r = r;
    leds[i].g = g;
    leds[i].b = b;
    if (i>leds_amount){
      leds[i].r = 100;
      leds[i].g = 1;
      leds[i].b = 1;
    }
  }
  FastLED.show();  //update our changes
}

void sortSignal(int sig){
  int dist = 0;
  if (sig>CLOSE_SIGNAL_AMOUNT){
    dist = 0;
  }else if (sig>MEDIUM_SIGNAL_AMOUNT){
    dist = 1;
  }else if (sig==NULL){
    dist = 2;
  }else{
    dist = 2;
  }
  if (DEBUG_MODE){
    checkStreak(dist);
    return;
  }


  for (int i = 1; i<CACHE_SIZE; i++){
    cache[i-1] = cache[i];
  }
  cache[CACHE_SIZE-1] = dist;

  Serial.print("[");
  for (int i = 0; i<CACHE_SIZE; i++){
    Serial.print(" ");
    Serial.print(cache[i]);
  }
  Serial.println("]");
  
  if (samecheck(cache, CACHE_SIZE)==1) return;
  adjustLight(dist);
  Serial.println("CONSISTENT");
}

int samecheck(const int a[], int n)
{   
    while(--n>0 && a[n]==a[0]);
    return n!=0;
}


void checkStreak(int dist){
  if (dist!=current_dist){
    streak = 0;
    last_dist = current_dist;
    current_dist = dist;
  }else{
    streak++;
    if (streak>=STREAK_MAX){
      adjustLight(dist);
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
