#include <FastLED.h>
#include "painlessMesh.h"
#include <ESP8266WiFi.h>
#include "config.h"
#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections %s\n", mesh.subConnectionJson().c_str());
} 

void setup() {
  Serial.begin(115200);
  
  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION);

  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT ); //initialize the mesh
  
  mesh.onNewConnection(&newConnectionCallback); //set callbacks
  mesh.onChangedConnections(&changedConnectionCallback);

  FastLED.addLeds<NEOPIXEL, RING_PIN>(leds, RING_LEDS); //Start everything at 0
  for (int i = 0; i < RING_LEDS; i++) {
    leds[i].r = 0;
    leds[i].g = 0;
    leds[i].b = 0;
  }
  //FastLED.show() will apply the changes
  
  for (int i = 0; i<CACHE_SIZE;i++){
    cache[i] = 2; //set to far to begin with
  }
}

void loop() {
  mesh.update(); //Mesh bg stuff
  doBackgroundStuff(); //My bg stuff
  c++; //counter once per loop
  if (c>=500){ //Only scan every 500s.  This should be close to the MS value of painlessMeshSTA.h
      sortSignal(highestRSSI); //Sort the highest RSSI between close, med, far
      c = 0; //reset to 0
      Serial.println("CHECKING");
      //setup();
  }
}

void doBackgroundStuff() { //merge of updateLight

  if (DEBUG_MODE==0) leds_amount = RING_LEDS; //Debug mode so not all LEDS blue
  int intensity = 0;
  
  if (current_dist==0&&DEBUG_MODE==0){ //When close, pulse
    int t = mesh.getNodeTime();
    intensity = intensityarr[(((t % (PULSE_LENGTH*1000000)) / (PULSE_LENGTH*5000)))]; //Logic to get pulse data
        
  }else if (highestRSSI>=-101&&DEBUG_MODE==1){ //If debug mode
    intensity = 100-(-1*highestRSSI); //intensity/100 - not really used for light intensity in debug mode
    leds_amount = ((intensity/100.0)*32); //amt of blue LEDs to show
  }else{
    intensity = max_intensity; //medium or far is constant color
  }

  //use intensities to get color
  int r = originalColor.R*(intensity/100.0);
  int g = originalColor.G*(intensity/100.0); 
  int b = originalColor.B*(intensity/100.0); 
  
  if (DEBUG_MODE==1){ //debug mode
    r = 1;
    g = 1;
    b = 100;
    //blue
  }
  
  for (int i = 0; i < RING_LEDS; i++) {
    // just set each pixel to its color
    leds[i].r = r;
    leds[i].g = g;
    leds[i].b = b;
    if (i>leds_amount){ //only applies in debug mode
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
    dist = 0; //close
  }else if (sig>MEDIUM_SIGNAL_AMOUNT){
    dist = 1; //medium
  }else if (sig==NULL){
    dist = 2; //far
  }else{
    dist = 2; //far
  }
  if (DEBUG_MODE==1){
    adjustLight(dist);
    //if debug mode, skip all the normal stuff
    return;
  }


  for (int i = 1; i<CACHE_SIZE; i++){
    cache[i-1] = cache[i]; //move the cache down
  }
  cache[CACHE_SIZE-1] = dist; //insert our latest data


  //just to know what's going on
  Serial.print("[");
  for (int i = 0; i<CACHE_SIZE; i++){
    Serial.print(" ");
    Serial.print(cache[i]);
  }
  Serial.println("]");

  if (samecheck(cache, CACHE_SIZE)==1) return; //Check if our array is all the same
  adjustLight(dist); //if it's consistent, then we adjust the light
  Serial.println("CONSISTENT");
}

int samecheck(const int a[], int n) //simple fn to check if array is same
{   
    while(--n>0 && a[n]==a[0]);
    return n!=0;
}

void adjustLight(int dist){ //Change intensity based on close/med/far
  current_dist = dist;
  if (dist==0){
    max_intensity = CLOSE_INTENSITY;
  }else if (dist==1){
    max_intensity = MEDIUM_INTENSITY;
  }else{
    max_intensity = FAR_INTENSITY;
  }
}
