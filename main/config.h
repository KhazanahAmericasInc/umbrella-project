#ifndef CONFIG_H
#define CONFIG_H

//STRUCTS

struct color {
  int R;
  int G;
  int B;
};

struct HSV{
  float H;
  float S;
  float V;
};

struct packet {
  char ID;
  color RGB;
  int intensity;
};

struct chipData {
  int16_t ax;
  int16_t ay;
  int16_t az;
};

const int CACHE_SIZE = 2; //how many frames to store
int cache_counter = 0;
chipData cache[CACHE_SIZE];
float mcache[CACHE_SIZE];
int mcache_counter = 0;
int spinCount = 0;
const int SPIN_TO_CHANGE_AMOUNT = 9;

const int MPU_addr=0x68;
chipData current_data = {0,0,0};
color originalColor = {255, 255, 255}; //Unique color
color thisColor = originalColor;
packet mypacket = {'A', thisColor, -1};
 

const int RING_PIN =  5; //ring pin

const int RING_LEDS = 32;
const int IDS_PER_SECOND = 100; //how many IDS to send out per second
float pulse_length = 2; //in seconds;
const float PULSE_LENGTH_FAR = 3.5; //in seconds;
const float PULSE_LENGTH_MEDIUM = 2.5; //in seconds;
const float PULSE_LENGTH_CLOSE = 1.5; //in seconds;
const float CLOSE_INTENSITY = 100; //intensity of color out of 100
const float MEDIUM_INTENSITY = 50; //intensity of color out of 100
const float FAR_INTENSITY = 15;//intensity of color out of 100

const int MAX_UMBRELLAS = 91; //max amount of umbrellas NOTE: this code uses binary value of ASCII characters.
char FIRST_ID = 'A'; //the first ID all other IDS must have binary value greater than this and less than MAX_UMBRELLAS

const int NUMBER_OF_TESTS = 50; //takes the average signal strength over this many tests
const float TEST_LENGTH = 0.05f; //length of each test in seconds

float MEDIUM_SIGNAL_AMOUNT = 1.5; //greater than this and less than CLOSE_SIGNAL_AMOUNT will be considered "medium"
float CLOSE_SIGNAL_AMOUNT = 2.5; //greater than this will be considered "close"
//less than MEDIUM_SIGNAL_AMOUNT will be considered "far"

float OUTLIER_CONSTANT = 0.2f; //a difference greater than this between tests signals an outlier
int LONELY_CONSTANT = 4; //how many scans to do before becoming lonely and turning back to original color
int random_factor = 0;
const int MIN_RAND = 8;
const int MAX_RAND = 12;

//LEDs for debugging.
const bool LED_MODE = false;
const bool DEBUG_MODE = false;

const int redPin = 2; //pin of red/far led
const int yellowPin = 3;//pin of yellow/medium led
const int greenPin = 4;//pin of green/close led

const uint8_t channel = 0x4c;
const uint64_t pipes[1] = { 0xF0F0F0F0E9LL}; //use this one channel for RX and TX

bool intensity_flag = false;
int intensity_id = -1;



//GLOBAL VARIABLES USED FOR TEMP STORAGE

bool up = true; //whether or not to increase intensity
float last_pulse = 0; //keep track of last light update
int filledLEDS = 0; //track how many LEDs have other unit's colors
int timesWithoutNeighbour = 0; //count how many times we were far away
int timesWithNeighbour = 0; //count how many times we have a close neighbour - used for random factor when close
float max_intensity = FAR_INTENSITY; //max intensity
float intensity = 0; //current intensity
float last_id_send = 0; //last time we sent an ID
float id_delta = 1.0 / IDS_PER_SECOND * 1000.0; //how long to wait between sendind IDs
int movementCounter = 0;
boolean hueUp = true;
bool isMedium = false;
int colorTwist = 0;


Adafruit_NeoPixel ring = Adafruit_NeoPixel(RING_LEDS, RING_PIN); //setup our 16 pixel ring
packet packet_data[MAX_UMBRELLAS]; //store the RGB of each umbrella
color ring_colors[RING_LEDS];

//gamma correction

const uint8_t PROGMEM gamma8[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

#endif
