#include <Adafruit_NeoPixel.h>

char ID = 'B';
const int RING_PIN =  12;
const float IDS_PER_SECOND = 1000;
float max_intensity;
float intensity = 0;
float color[3] = {255,255,255};
const float PULSE_LENGTH = 2; //in seconds;
const float CLOSE_COLOR[] =  {255, 30, 30};
const float MEDIUM_COLOR[] = {255, 60, 60};
const float FAR_COLOR[] = {255, 255, 255};
const float CLOSE_INTENSITY = 20;
const float MEDIUM_INTENSITY = 50;
const float FAR_INTENSITY = 100;
bool up = true; //whether or not to increase intensity
float last_pulse = 0;
Adafruit_NeoPixel ring = Adafruit_NeoPixel(16, RING_PIN);


void setup() {
 Serial.begin(9600);
 ring.begin(); //init ring
 ring.show(); //show nothing
 last_pulse = millis();
}

void sendID(){
  float d = 1.0/IDS_PER_SECOND*1000.0; //calculate delay in ms
  for (int i = 0; i<IDS_PER_SECOND;i++){
    Serial.print(ID);
    delay(d);
    char c = Serial.read();
    setLight(c);
    updateLight();
  }
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

void updateLight(){
  if ((millis()-last_pulse)<((1/max_intensity)*1000*PULSE_LENGTH/2)) return;
  last_pulse = millis();
  
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
  for (int i = 0; i<16;i++){
    ring.setPixelColor(i,intensity/100*color[0], intensity/100*color[1], intensity/100*color[2]);
  }
  ring.show();
}

void loop() {
  sendID();
}
