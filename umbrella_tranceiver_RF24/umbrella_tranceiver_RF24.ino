#include <SevenSeg.h>

#include <Adafruit_NeoPixel.h>
#include <SPI.h>

#include "nRF24L01.h"
#include "RF24.h"

//MAIN PROGRAM GLOBAL PARAMETERS

char ID = 'G'; //IDs should be all unique binary values from 0 to MAX_UMBRELLAS.

const int RING_PIN =  12; //ring pin
const int IDS_PER_SECOND = 1; //how many IDS to send out per second

const float PULSE_LENGTH = 2; //in seconds;
const float CLOSE_COLOR[] =  {255, 30, 30}; //Color of umbrella when close
const float MEDIUM_COLOR[] = {255, 60, 60}; //Color of umbrella when medium
const float FAR_COLOR[] = {255, 255, 255}; //Color of umbrella when far
const float CLOSE_INTENSITY = 20; //intensity of close color out of 100
const float MEDIUM_INTENSITY = 50; //intensity of medium color out of 100
const float FAR_INTENSITY = 100;//intensity of far color out of 100
const int MAX_UMBRELLAS = 100; //max amount of umbrellas NOTE: this code uses binary value of ASCII characters.
char FIRST_ID = 'A'; //the first ID all other IDS must have binary value greater than this and less than MAX_UMBRELLAS

const int NUMBER_OF_TESTS = 1; //takes the average signal strength over this many tests
const float TEST_LENGTH = 2; //length of each test in seconds

//global temp variables
float color[3];
bool up = true; //whether or not to increase intensity
float last_pulse = 0; //keep track of last light update

float max_intensity = 255; //max intensity
float intensity = 0; //current intensity
//basically mapping black (0,0,0) to each color


float last_id_send = 0; //last time we sent an ID
float id_delta = 1.0/IDS_PER_SECOND*1000.0; //how long to wait between sendind IDs


//LEDs for debugging.
const bool LED_MODE = true;

const int redPin = 2; //pin of red/far led
const int yellowPin = 3;//pin of yellow/medium led
const int greenPin = 4;//pin of green/close led


//RF24 Radio stuff

RF24 radio(7, 8); // uno
const uint8_t channel = 0x4c;
const uint64_t pipes[1] = { 0xF0F0F0F0E9LL}; //use this one channel for RX and TX

Adafruit_NeoPixel ring = Adafruit_NeoPixel(16, RING_PIN); //setup our 16 pixel ring


void setup() {
 Serial.begin(9600);
 ring.begin(); //init ring
 ring.show(); //show nothing
 last_pulse = millis(); //set to now
 last_id_send = millis(); //set to now

 //RF24 STUFF
 radio.begin(); //init the radio
 radio.setRetries(5,5); //try to send 100 times with 0 delay.  This doesn't seem to affect things too much.
 radio.setChannel(channel); //set the channel
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
  Serial.println("LISTENING");
  int totals[MAX_UMBRELLAS]; //Array to count instances of each ID
  for (int i = (int)FIRST_ID;i<MAX_UMBRELLAS;i++){
      doBackgroundStuff(); //Running this as much as possible.  see definition.
      totals[i] = 0; //Fill the array with 0s
  }
  for (int i = 0; i<NUMBER_OF_TESTS; i++){ //How many "tests" to run.  Average is taken of all the tests
    String sample = ""; //empty string
    float t = millis(); //keep track of when we started the test
    while(millis()-t<1000.0*TEST_LENGTH){ //while the time doesn't exceed our defined test length
      doBackgroundStuff(); //see definition
      char c;
      setReadMode();
      radio.read(&c, sizeof(char)); //Write to the address of our character
      sample.concat(c); //Add on the character to our string
    }
    for (int i = 0; i<sample.length();i++){ //Now we "sort" our string of IDS
      doBackgroundStuff(); //see definition
      int value = (int) sample.charAt(i); //get binary value of the ID.  We will use this as the index.  Basically a hash table where casting is our hash function
      if (value<=MAX_UMBRELLAS){
           totals[value]++; //increment the corresponding slot.
      }
    }
  }

  //we now know approx how far away everything is (RELATIVELY - AMOUNT OF DATA RECEIVED MIGHT CHANGE WHEN WE ADD MORE NODES)
  //we can do whatever we want with this data, but for now we'll just find the closest umbrella
  
  float highest = 0; //get the highest element in this array we built
    for (int i = (int)FIRST_ID;i<MAX_UMBRELLAS;i++){
      doBackgroundStuff();
      if (totals[i]>highest){
        highest = totals[i]; //it doesn't really matter the ID of the umbrella.  We just want the closest one
      }
    }
   highest/=NUMBER_OF_TESTS; //average out
   Serial.println(highest); //for debugging

   //gets sent to setLight()
   //this is hard coded in.  Would be nicer to have as global variables
   //Or even callibration depending on the total amount of signals we receive
   if (highest<4) return 'F';
   if (highest<7) return 'M';
   return 'C';
}



void setLight(char level){ //pretty straightforward.  Just setting the color/intensity for close/medium/far
  switch (level){
    case 'F': 
      max_intensity = CLOSE_INTENSITY;
      color[0] = FAR_COLOR[0];
      color[1] = FAR_COLOR[1];
      color[2] = FAR_COLOR[2];
      if (LED_MODE){ //only if on LED mode
        digitalWrite(redPin, HIGH);
        digitalWrite(yellowPin, LOW);
        digitalWrite(greenPin, LOW);
      }
      break;
    case 'M':
      max_intensity = MEDIUM_INTENSITY;
      color[0] = MEDIUM_COLOR[0];
      color[1] = MEDIUM_COLOR[1];
      color[2] = MEDIUM_COLOR[2];
      if (LED_MODE){
        digitalWrite(redPin, LOW);
        digitalWrite(yellowPin, HIGH);
        digitalWrite(greenPin, LOW);
      }
      break;
    case 'C':
      max_intensity = FAR_INTENSITY;
      color[0] = CLOSE_COLOR[0];
      color[1] = CLOSE_COLOR[1];
      color[2] = CLOSE_COLOR[2];
      if (LED_MODE){
        digitalWrite(redPin, LOW);
        digitalWrite(yellowPin, LOW);
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
      radio.write(&ID, sizeof(char)); //send out our ID                   
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
    ring.setPixelColor(i,intensity/100*color[0], intensity/100*color[1], intensity/100*color[2]);
  }
  ring.show(); //update our changes
}

void loop() {
  setLight(captureID()); //run this over and over
}
