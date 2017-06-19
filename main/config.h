#ifndef CONFIG_H
#define CONFIG_H

//MAIN PROGRAM GLOBAL PARAMETERS

struct color {
  int R;
  int G;
  int B;
};

struct packet {
  char ID;
  color RGB;
};

struct chipData {
  float ax;
  float ay;
};

const color originalColor = {0, 255, 0}; //Unique color
color thisColor = originalColor;
packet mypacket = {'G', thisColor};


chipData current_data;
const int CACHE_SIZE = 0; //how many accelerometeer samples to average out;
const int RING_PIN =  5; //ring pin
const int RING_LEDS = 16;
const int IDS_PER_SECOND = 100; //how many IDS to send out per second
const float PULSE_LENGTH = 2; //in seconds;
const float CLOSE_INTENSITY = 100; //intensity of color out of 100
const float MEDIUM_INTENSITY = 60; //intensity of color out of 100
const float FAR_INTENSITY = 40;//intensity of color out of 100
const int MAX_UMBRELLAS = 100; //max amount of umbrellas NOTE: this code uses binary value of ASCII characters.
char FIRST_ID = 'A'; //the first ID all other IDS must have binary value greater than this and less than MAX_UMBRELLAS


const int NUMBER_OF_TESTS = 5; //takes the average signal strength over this many tests
const float TEST_LENGTH = 0.1f; //length of each test in seconds

int MEDIUM_SIGNAL_AMOUNT = 1.3; //greater than this and less than CLOSE_SIGNAL_AMOUNT will be considered "medium"
int CLOSE_SIGNAL_AMOUNT = 2.4; //greater than this will be considered "close"
//less than MEDIUM_SIGNAL_AMOUNT will be considered "far"

float OUTLIER_CONSTANT = 0.2f; //a difference greater than this between tests signals an outlier
int LONELY_CONSTANT = 2; //how many scans to do before becoming lonely and turning back to original color
int random_factor = 0;
const int MIN_RAND = 2;
const int MAX_RAND = 6;

//LEDs for debugging.
const bool LED_MODE = false;
const bool DEBUG_MODE = false;

const int redPin = 2; //pin of red/far led
const int yellowPin = 3;//pin of yellow/medium led
const int greenPin = 4;//pin of green/close led
const int MIC_PIN = 6;
const int INTERRUPT_PIN = 2;
const uint8_t channel = 0x4c;
const uint64_t pipes[1] = { 0xF0F0F0F0E9LL}; //use this one channel for RX and TX

/*uint16_t packetSize;
Quaternion q;
VectorInt16 acc;
VectorInt16 accReal;
VectorFloat gravity;
uint8_t fifoBuffer[64];
/*
//GLOBAL VARIABLES USED FOR TEMP STORAGE

bool up = true; //whether or not to increase intensity
float last_pulse = 0; //keep track of last light update
int filledLEDS = 0; //track how many LEDs have other unit's colors
int timesWithoutNeighbour = 0; //count how many times we were far away
int timesWithNeighbour = 0; //count how many times we have a close neighbour - used for random factor when close
float max_intensity = 100; //max intensity
float intensity = 0; //current intensity
float last_id_send = 0; //last time we sent an ID
float id_delta = 1.0 / IDS_PER_SECOND * 1000.0; //how long to wait between sendind IDs


Adafruit_NeoPixel ring = Adafruit_NeoPixel(RING_LEDS, RING_PIN); //setup our 16 pixel ring
packet rgb_data[MAX_UMBRELLAS]; //store the RGB of each umbrella
color ring_colors[RING_LEDS];
RF24 radio(7, 8); // uno
Adafruit_WS2801 bulb = Adafruit_WS2801(4, 9, 10);

//gamma correction

#endif
