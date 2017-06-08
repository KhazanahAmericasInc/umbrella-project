#include <SevenSeg.h>

#include <Adafruit_NeoPixel.h>
#include <SPI.h>

#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

//MAIN PROGRAM GLOBAL PARAMETERS

struct color{
  int R;
  int G;
  int B;
};

struct packet{
  char ID;
  color RGB;
};

color thisColor = {0,0,255};  //Unique color
packet mypacket = {'C', thisColor};

const int RING_PIN =  5; //ring pin
const int RING_LEDS = 16;
const int IDS_PER_SECOND = 100; //how many IDS to send out per second
const float PULSE_LENGTH = 2; //in seconds;
const float CLOSE_INTENSITY = 20; //intensity of color out of 100
const float MEDIUM_INTENSITY = 50; //intensity of color out of 100
const float FAR_INTENSITY = 100;//intensity of color out of 100
const int MAX_UMBRELLAS = 100; //max amount of umbrellas NOTE: this code uses binary value of ASCII characters.
char FIRST_ID = 'A'; //the first ID all other IDS must have binary value greater than this and less than MAX_UMBRELLAS

const int NUMBER_OF_TESTS = 20; //takes the average signal strength over this many tests
const float TEST_LENGTH = 0.1f; //length of each test in seconds

int MEDIUM_SIGNAL_AMOUNT = 1; //greater than this and less than CLOSE_SIGNAL_AMOUNT will be considered "medium"
int CLOSE_SIGNAL_AMOUNT = 2; //greater than this will be considered "close"
//less than MEDIUM_SIGNAL_AMOUNT will be considered "far"

float OUTLIER_CONSTANT = 0.2f; //a difference greater than this between tests signals an outlier

//global temp variables
bool up = true; //whether or not to increase intensity
float last_pulse = 0; //keep track of last light update

float max_intensity = 100; //max intensity
float intensity = 0; //current intensity
//basically mapping black (0,0,0) to each color


float last_id_send = 0; //last time we sent an ID
float id_delta = 1.0/IDS_PER_SECOND*1000.0; //how long to wait between sendind IDs


//LEDs for debugging.
const bool LED_MODE = false;
const bool DEBUG_MODE = false;

const int redPin = 2; //pin of red/far led
const int yellowPin = 3;//pin of yellow/medium led
const int greenPin = 4;//pin of green/close led


//RF24 Radio stuff

RF24 radio(7, 8); // uno
//RF24 radio(9, 10); //nano
const uint8_t channel = 0x4c;
const uint64_t pipes[1] = { 0xF0F0F0F0E9LL}; //use this one channel for RX and TX

Adafruit_NeoPixel ring = Adafruit_NeoPixel(RING_LEDS, RING_PIN); //setup our 16 pixel ring
packet rgb_data[MAX_UMBRELLAS]; //store the RGB of each umbrella
color ring_colors[RING_LEDS]; //store the RGB of each LED


void setup() {

  
 Serial.begin(9600);
 ring.begin(); //init ring
 ring.show(); //show nothing
 last_pulse = millis(); //set to now
 last_id_send = millis(); //set to now

 //RF24 STUFF

 radio.begin(); //init the radio
 //radio.setDataRate(RF24_250KBPS);
 //radio.setPALevel(RF24_PA_MIN);
 radio.setRetries(5,5); //try to send 100 times with 0 delay.  This doesn't seem to affect things too much.
 radio.setChannel(channel); //set the channel
 printf_begin();

 for (int i = (int)FIRST_ID; i<MAX_UMBRELLAS;i++){
  rgb_data[i].ID = (char) i;
  rgb_data[i].RGB.R = 0;
  rgb_data[i].RGB.G = 0;
  rgb_data[i].RGB.B = 0;
 }
}

void setReadMode(){ //start listening and open our reading pipe
  radio.openReadingPipe(1, pipes[1]);
  radio.startListening();
}

void setWriteMode(){ //stop listening so we can talk.  Open our writing pipe
  radio.stopListening();
  radio.openWritingPipe(pipes[1]);
}


char captureID(){
  Serial.println("TRYING");
  if (DEBUG_MODE) radio.printDetails();
  int totals[MAX_UMBRELLAS]; //Array to count instances of each ID
  int past_test[MAX_UMBRELLAS]; //Past test to compare our current one to
  int current_test[MAX_UMBRELLAS]; //The current test

  for (int i = (int)FIRST_ID;i<MAX_UMBRELLAS;i++){
      doBackgroundStuff(); //Running this as much as possible.  see definition.
      totals[i] = 0; //Fill the array with 0s
      past_test[i] = 0;
      current_test[i] = 0;
  }
  
  for (int i = 0; i<NUMBER_OF_TESTS; i++){ //How many "tests" to run.  Average is taken of all the tests
    String sample = ""; //empty string
    float t = millis(); //keep track of when we started the test
    
    while(millis()-t<1000.0*TEST_LENGTH){ //while the time doesn't exceed our defined test length
      doBackgroundStuff(); //see definition
      packet p;
      setReadMode();
      radio.read(&p, sizeof(packet)); //Write to the address of our character
      sample.concat(p.ID); //Add on the character to our string
      String rgb_string = "";
      rgb_data[(int)p.ID] = p;
    }
    for (int j = 0; j<sample.length();j++){ //Now we "sort" our string of IDS
      doBackgroundStuff(); //see definition
      int value = (int) sample.charAt(j); //get binary value of the ID.  We will use this as the index.  Basically a hash table where casting is our hash function
      if (value<=MAX_UMBRELLAS){
           current_test[value]++; //increment the corresponding slot.
      }
    }
    for (int j = (int)FIRST_ID; j<MAX_UMBRELLAS;j++){
      doBackgroundStuff();
      float difference = current_test[j]-past_test[j];
      if (fabs(difference)>OUTLIER_CONSTANT*NUMBER_OF_TESTS){
        //DEAL WITH THE OUTLIER
        if (DEBUG_MODE){
          Serial.print("CHANGING FROM ");
          Serial.print(current_test[j]);
        }
        current_test[j] -= difference/2; //fix the outlier by subtracting the outlier
        if (DEBUG_MODE){
          Serial.print(" TO ");
          Serial.print(current_test[j]);
          Serial.print(" WHICH IS CLOSER TO ");
          Serial.println(past_test[j]);
        }
      }
      totals[j]+=current_test[j];
      past_test[j]=current_test[j];
      current_test[j] = 0;
    }

  }

  //we now know approx how far away everything is (RELATIVELY: AMOUNT OF DATA RECEIVED MIGHT CHANGE WHEN WE ADD MORE NODES)
  //we can do whatever we want with this data, but for now we'll just find the closest umbrella
  
  float highest = 0; //get the highest element in this array we built
  int indexOfHighest = 0;
    for (int i = (int)FIRST_ID;i<MAX_UMBRELLAS;i++){
      doBackgroundStuff();
      if (totals[i]>highest){
        highest = totals[i];
        indexOfHighest = i;
      }
    }
   highest/=NUMBER_OF_TESTS; //average out
   Serial.println(highest); //for debugging
   Serial.print("COLOR IS ");
   Serial.print(mypacket.RGB.R);
   Serial.print(" ");
   Serial.print(mypacket.RGB.G);
   Serial.print(" ");
   Serial.println(mypacket.RGB.B);

   //gets sent to setLight()
   //this is hard coded in.  Would be nicer to have as global variables
   //Or even callibration depending on the total amount of signals we receive
   
   if (highest<MEDIUM_SIGNAL_AMOUNT) return 'F';
   if (highest<CLOSE_SIGNAL_AMOUNT) return 'M';
   addColor(indexOfHighest);
   return 'C';
}

void addColor(int index){
  doBackgroundStuff();
  int r1 = mypacket.RGB.R;
  int g1 = mypacket.RGB.G;
  int b1 = mypacket.RGB.B;

  int r2 = rgb_data[index].RGB.R;
  int g2 = rgb_data[index].RGB.G;
  int b2 = rgb_data[index].RGB.B;

  int rd = r1-r2;
  int gd = g1-g2;
  int bd = b1-b2;

  r1-=rd/2;
  g1-=gd/2;
  b1-=bd/2;

  mypacket.RGB.R = r1;
  mypacket.RGB.G = g1;
  mypacket.RGB.B = b1;
}

String RGBFormat(int input){ //makes sure it's 3 chars long
  doBackgroundStuff();
  if (input/100>0) return String(input);
  if (input/10>0){
    String temp = "0";
    temp.concat(String(input));
    return temp;
  }
  String temp = "00";
  temp.concat(String(input));
  return temp;
}

void setLight(char level){ //pretty straightforward.  Just setting the color/intensity for close/medium/far
  switch (level){
    case 'F': 
      max_intensity = FAR_INTENSITY;
      if (LED_MODE){ //only if on LED mode
        digitalWrite(redPin, HIGH);
        digitalWrite(yellowPin, LOW);
        digitalWrite(greenPin, LOW);
      }
      break;
    case 'M':
      max_intensity = MEDIUM_INTENSITY;
      if (LED_MODE){
        digitalWrite(redPin, LOW);
        digitalWrite(yellowPin, HIGH);
        digitalWrite(greenPin, LOW);
      }
      break;
    case 'C':
      max_intensity = CLOSE_INTENSITY;
      if (LED_MODE){
        digitalWrite(redPin, LOW);
        digitalWrite(yellowPin, LOW);
        digitalWrite(greenPin, HIGH);
      }
      break;
     case 'R': //R for really close
       if (LED_MODE){
          digitalWrite(redPin, HIGH);
          digitalWrite(yellowPin, HIGH);
          digitalWrite(greenPin, HIGH);
        }
     break;
  }
}

void doBackgroundStuff(){ 
  //This method is called as much as possible since we don't have multi-threading
  //HOWEVER, since we track the time using millis(), and since we have defined the incrememnt of how often we should send IDs and update the light, 
  //the functions only actually enter the block IF the defined incrememnt of time has occured.

  //so basically, we're spamming these functions everywhere but they don't do anything unless it's time for them to.
  sendID();
  updateLight();
}

void sendID(){
    if ((millis()-last_id_send)>id_delta){ //Logic for timing things right.
      setWriteMode(); //get into write mode               
      radio.write(&mypacket, sizeof(packet)); //send out our ID                   
      last_id_send = millis(); //track when we sent it
    }
}
void updateLight(){
  if ((millis()-last_pulse)>((1/max_intensity)*1000*PULSE_LENGTH/2)){ //same logic as sendID()
    //Simple logic to linearly fade in and out.
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
    last_pulse = millis(); //track this too
  }
  for (int i = 0; i<16;i++){
    ring.setPixelColor(i,intensity/100*mypacket.RGB.R, intensity/100*mypacket.RGB.G, intensity/100*mypacket.RGB.B);
  }
  ring.show(); //update our changes
  //Serial.print("INTENSITY IS ");
  //Serial.println(intensity);
}

void loop() {
  setLight(captureID()); //run this over and over
}

