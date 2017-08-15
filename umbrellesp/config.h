#ifndef CONFIG_H
#define CONFIG_H
//MAIN PROGRAM GLOBAL PARAMETERS

//STRUCTS

struct color {
  int R;
  int G;
  int B;
};
color originalColor = {255, 255, 255}; //Unique color
 

const int RING_PIN =  5; //ring pin

const int RING_LEDS = 12;
const float PULSE_LENGTH = 2; //in seconds;
const float CLOSE_INTENSITY = 100; //intensity of color out of 100
const float MEDIUM_INTENSITY = 50; //intensity of color out of 100
const float FAR_INTENSITY = 5;//intensity of color out of 100

float MEDIUM_SIGNAL_AMOUNT = 200000;
float CLOSE_SIGNAL_AMOUNT = 70000;

//GLOBAL VARIABLES USED FOR TEMP STORAGE

bool up = true; //whether or not to increase intensity
float last_pulse = 0; //keep track of last light update
int timesWithoutNeighbour = 0; //count how many times we were far away
int timesWithNeighbour = 0; //count how many times we have a close neighbour - used for random factor when close
float max_intensity = 100; //max intensity
float intensity = 0; //current intensity
int movementCounter = 0;
bool isMedium = false;

int current_dist = 2;
int streak = 0;
const int STREAK_MAX = 2; //how many times in a row the dist must be the same to register a change


Adafruit_NeoPixel ring = Adafruit_NeoPixel(RING_LEDS, RING_PIN); //setup our 16 pixel ring

int intensityarr[] = 
{
3, 3, 3, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70,
71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93,
94, 95, 96, 97, 97, 97, 97, 97, 97, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84,
83, 82, 81, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61,
60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38,
37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15,
14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 3, 3,
};


#endif

