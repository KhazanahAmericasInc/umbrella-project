
#include <FastLED.h>


#ifndef CONFIG_H
#define CONFIG_H
//MAIN PROGRAM GLOBAL PARAMETERS

//STRUCTS

struct color {
  int R;
  int G;
  int B;
};
color originalColor = {255, 255, 100}; //Unique color


const int DEBUG_MODE = 0;
const int RING_PIN = 5; //ring pin
const int CACHE_SIZE = 3;
int cache[CACHE_SIZE];
int cache_counter = 0;

const int RING_LEDS = 32;
int leds_amount = 32;
const float PULSE_LENGTH = 2; //in seconds;
const float CLOSE_INTENSITY = 100; //intensity of color out of 100
const float MEDIUM_INTENSITY = 50; //intensity of color out of 100
const float FAR_INTENSITY = 3;//intensity of color out of 100

float MEDIUM_SIGNAL_AMOUNT = -100;
float CLOSE_SIGNAL_AMOUNT = -60;

//GLOBAL VARIABLES USED FOR TEMP STORAGE

bool up = true; //whether or not to increase intensity
float last_pulse = 0; //keep track of last light update
int timesWithoutNeighbour = 0; //count how many times we were far away
int timesWithNeighbour = 0; //count how many times we have a close neighbour - used for random factor when close
float max_intensity = FAR_INTENSITY; //max intensity
float intensity = 0; //current intensity
int movementCounter = 0;
bool isMedium = false;

int current_dist = 2;
int streak = 0;
const int STREAK_MAX = 1; //how many times in a row the dist must be the same to register a change
int last_dist  = 2;

CRGB leds[RING_LEDS];

int intensityarr[] = 
{
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7, 7, 8, 9, 9, 10, 10, 11, 12, 12, 13, 14, 15, 16, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 36, 37, 38, 39, 40, 42, 43, 44, 46, 47, 49, 50, 51, 53, 54, 56, 57, 59, 60, 62, 64, 65, 67, 68, 70, 72, 73, 75, 77, 79, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 79, 77, 75, 73, 72, 70, 68, 67, 65, 64, 62, 60, 59, 57, 56, 54, 53, 51, 50, 49, 47, 46, 44, 43, 42, 40, 39, 38, 37, 36, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 16, 15, 14, 13, 12, 12, 11, 10, 10, 9, 9, 8, 7, 7, 6, 6, 5, 5, 4, 4, 4, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};



#endif


