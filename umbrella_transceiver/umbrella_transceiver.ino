#include <Adafruit_NeoPixel.h>

//MAIN PROGRAM PARAMETERS

char ID = 'B'; //IDs should be all unique binary values from 0 to MAX_UMBRELLAS.

const int RING_PIN =  12; //ring pin
const int IDS_PER_SECOND = 750; //how many IDS to send out per second


const float PULSE_LENGTH = 2; //in seconds;
const float CLOSE_COLOR[] =  {255, 30, 30}; //Color of umbrella when close
const float MEDIUM_COLOR[] = {255, 60, 60}; //Color of umbrella when medium
const float FAR_COLOR[] = {255, 255, 255}; //Color of umbrella when far
const float CLOSE_INTENSITY = 20; //intensity of close color out of 100
const float MEDIUM_INTENSITY = 50; //intensity of medium color out of 100
const float FAR_INTENSITY = 100;//intensity of far color out of 100
const int MAX_UMBRELLAS = 100; //max amount of umbrellas NOTE: this code uses binary value of ASCII characters.
char FIRST_ID = 'A'; //the first ID all other IDS must have binary value greater than this and less than MAX_UMBRELLAS

const int SECONDS_TO_TEST = 3; //takes the average signal strength over this many seconds
const float TEST_ROUNDS = 3; //takes average signal strength over this many rounds per test

//global temp variables
float color[3];
bool up = true; //whether or not to increase intensity
float last_pulse = 0;
float max_intensity = 255; 
float intensity = 0;

float last_id_send = 0;
float id_delta = 1.0/IDS_PER_SECOND*1000.0;



Adafruit_NeoPixel ring = Adafruit_NeoPixel(16, RING_PIN);

void setup() {
 Serial.begin(9600);
 ring.begin(); //init ring
 ring.show(); //show nothing
 last_pulse = millis();
 last_id_send = millis();
}

char captureID(){
  String sample = "";
  float t = millis();
  while(millis()-t<1000.0*SECONDS_TO_TEST){
    doBackgroundStuff();
    char c = Serial.read();
    sample.concat(c);
  }
  int chars[MAX_UMBRELLAS];
  for (int i = (int)FIRST_ID;i<MAX_UMBRELLAS;i++){
    doBackgroundStuff();
    chars[i] = 0;
  }
  for (int i = 0; i<sample.length();i++){ //sort all ids
    doBackgroundStuff();
    int value = (int) sample.charAt(i);
    if (value<=MAX_UMBRELLAS){
         chars[value]++;
    }
  }
  int highest = 0;
  for (int i = (int)FIRST_ID;i<MAX_UMBRELLAS;i++){
    doBackgroundStuff();
    if (chars[i]>highest){
      highest = chars[i];
    }
  }
  Serial.println();
  Serial.println(highest);
  if (highest<50) return 'F';
  else if (highest<100) return 'M';
  return 'C';
}



void setLight(char level){
  switch (level){
    case 'F':
      max_intensity = CLOSE_INTENSITY;
      color[0] = FAR_COLOR[0];
      color[1] = FAR_COLOR[1];
      color[2] = FAR_COLOR[2];
      break;
    case 'M':
      max_intensity = MEDIUM_INTENSITY;
      color[0] = MEDIUM_COLOR[0];
      color[1] = MEDIUM_COLOR[1];
      color[2] = MEDIUM_COLOR[2];
      break;
    case 'C':
      max_intensity = FAR_INTENSITY;
      color[0] = CLOSE_COLOR[0];
      color[1] = CLOSE_COLOR[1];
      color[2] = CLOSE_COLOR[2];
      break;
  }
}

void doBackgroundStuff(){ 
  sendID();
  updateLight();
}

void sendID(){
    if ((millis()-last_id_send)>id_delta){ //Doesn't really matter how often I spam this function because it'll only actually
      Serial.print(ID);                    //go through if the delta condition is met
      last_id_send = millis();
    }
}
void updateLight(){
  if ((millis()-last_pulse)>((1/max_intensity)*1000*PULSE_LENGTH/2)){ //same logic as sendID()
    if (up&&intensity<max_intensity){
    intensity++;
    }else if (up&&intensity>=max_intensity){
      intensity--;
      up = false;
    }else if (!up&&intensity>0){
      intensity--;
    }else{
      intensity++;
      up = true;
    }
    last_pulse = millis();
  }
  for (int i = 0; i<16;i++){
    ring.setPixelColor(i,intensity/100*color[0], intensity/100*color[1], intensity/100*color[2]);
  }
  ring.show();
}

void loop() {
  setLight(captureID());
}
