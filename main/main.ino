
#include <Adafruit_NeoPixel.h>
#include <Adafruit_WS2801.h>
#include <SevenSeg.h>
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "config.h"

void setup() {

  Serial.begin(9600);
  ring.begin(); //init ring
  ring.show(); //show nothing
  bulb.begin();
  bulb.show();
  last_pulse = millis(); //set to now
  last_id_send = millis(); //set to now

  //accelerometer stuff
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  random_factor = random(MIN_RAND, MAX_RAND);

  //RF24 STUFF

  radio.begin(); //init the radio
  //radio.setDataRate(RF24_250KBPS);
  //radio.setPALevel(RF24_PA_MIN);
  radio.setRetries(5, 5); //try to send 100 times with 0 delay.  This doesn't seem to affect things too much.
  radio.setChannel(channel); //set the channel
  printf_begin();

  rgb_data[(int)mypacket.ID] = mypacket;

  for (int i = (int)FIRST_ID; i < MAX_UMBRELLAS; i++) {
    rgb_data[i].ID = (char) i;
    rgb_data[i].RGB.R = 0;
    rgb_data[i].RGB.G = 0;
    rgb_data[i].RGB.B = 0;
  }

  for (int i = 0; i < RING_LEDS; i++) {
    ring_colors[i] = mypacket.RGB;
  }

}

void setReadMode() { //start listening and open our reading pipe
  radio.openReadingPipe(1, pipes[1]);
  radio.startListening();
}

void setWriteMode() { //stop listening so we can talk.  Open our writing pipe
  radio.stopListening();
  radio.openWritingPipe(pipes[1]);
}


char captureID() { //todo separate this
  Serial.println("TRYING");
  if (DEBUG_MODE) radio.printDetails();
  int totals[MAX_UMBRELLAS]; //Array to count instances of each ID
  int past_test[MAX_UMBRELLAS]; //Past test to compare our current one to
  int current_test[MAX_UMBRELLAS]; //The current test

  for (int i = (int)FIRST_ID; i < MAX_UMBRELLAS; i++) {
    doBackgroundStuff(); //Running this as much as possible.  see definition.
    totals[i] = 0; //Fill the array with 0s
    past_test[i] = 0;
    current_test[i] = 0;
  }

  for (int i = 0; i < NUMBER_OF_TESTS; i++) { //How many "tests" to run.  Average is taken of all the tests
    String sample = ""; //empty string
    float t = millis(); //keep track of when we started the test

    while (millis() - t < 1000.0 * TEST_LENGTH) { //while the time doesn't exceed our defined test length
      doBackgroundStuff(); //see definition
      packet p;
      setReadMode();
      radio.read(&p, sizeof(packet)); //Write to the address of our character
      sample.concat(p.ID); //Add on the character to our string
      String rgb_string = "";
      rgb_data[(int)p.ID] = p;
    }
    for (int j = 0; j < sample.length(); j++) { //Now we "sort" our string of IDS
      doBackgroundStuff(); //see definition
      int value = (int) sample.charAt(j); //get binary value of the ID.  We will use this as the index.  Basically a hash table where casting is our hash function
      if (value <= MAX_UMBRELLAS) {
        current_test[value]++; //increment the corresponding slot.
      }
    }
    for (int j = (int)FIRST_ID; j < MAX_UMBRELLAS; j++) {
      doBackgroundStuff();
      float difference = current_test[j] - past_test[j];
      if (fabs(difference) > OUTLIER_CONSTANT * NUMBER_OF_TESTS) {
        //DEAL WITH THE OUTLIER
        if (DEBUG_MODE) {
          Serial.print("CHANGING FROM ");
          Serial.print(current_test[j]);
        }
        current_test[j] -= difference / 2; //fix the outlier by subtracting the outlier
        if (DEBUG_MODE) {
          Serial.print(" TO ");
          Serial.print(current_test[j]);
          Serial.print(" WHICH IS CLOSER TO ");
          Serial.println(past_test[j]);
        }
      }
      totals[j] += current_test[j];
      past_test[j] = current_test[j];
      current_test[j] = 0;
    }

  }

  //we now know approx how far away everything is (RELATIVELY: AMOUNT OF DATA RECEIVED MIGHT CHANGE WHEN WE ADD MORE NODES)
  //we can do whatever we want with this data, but for now we'll just find the closest umbrella

  float highest = 0; //get the highest element in this array we built
  int indexOfHighest = 0;
  for (int i = (int)FIRST_ID; i < MAX_UMBRELLAS; i++) {
    doBackgroundStuff();
    if (totals[i] > highest) {
      highest = totals[i];
      indexOfHighest = i;
    }
  }
  highest /= NUMBER_OF_TESTS; //average out
  Serial.println(indexOfHighest);
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

  timesWithoutNeighbour++;
  if (highest < MEDIUM_SIGNAL_AMOUNT) {
    if (timesWithoutNeighbour>=LONELY_CONSTANT){
      decay();
    }
    timesWithNeighbour = 0;
    return 'F';
  }
  if (highest < CLOSE_SIGNAL_AMOUNT) {
    if (timesWithoutNeighbour>=LONELY_CONSTANT){
      decay();
    }
    timesWithNeighbour = 0;
    return 'M';
  }
  timesWithoutNeighbour = 0;
  mixWithNeighbour(indexOfHighest);
  timesWithNeighbour++;
  return 'C';
}

void addColor(int indexOfHighest) {
  ring_colors[filledLEDS] = rgb_data[indexOfHighest].RGB;
  ring_colors[filledLEDS + 1] = rgb_data[indexOfHighest].RGB;
  ring_colors[filledLEDS + 2] = rgb_data[indexOfHighest].RGB;
  ring_colors[filledLEDS + 3] = rgb_data[indexOfHighest].RGB;
  filledLEDS += 4;
  if (filledLEDS >= RING_LEDS) filledLEDS = 0;
}

void decay(){
  doBackgroundStuff();
  mypacket.RGB = mixColors(mypacket.RGB, originalColor);
}

void mixWithNeighbour(int index) { //mixes the unit's color with the nearest unit's color 50/50
  doBackgroundStuff();
  mypacket.RGB = mixColors(rgb_data[index].RGB, mypacket.RGB);
}

color mixColors (color c1, color c2) { //simple color mixing algo
  color temp = c1;
  int rd = c1.R - c2.R;
  int gd = c1.G - c2.G;
  int bd = c1.B - c2.B;
  temp.R -= rd / 2;
  temp.G -= gd / 2;
  temp.B -= bd / 2;
  return temp;
}

void setLight(char level) { //pretty straightforward.  Just setting the color/intensity for close/medium/far
  switch (level) {
    case 'F':
      max_intensity = FAR_INTENSITY;
      if (LED_MODE) { //only if on LED mode
        digitalWrite(redPin, HIGH);
        digitalWrite(yellowPin, LOW);
        digitalWrite(greenPin, LOW);
      }
      break;
    case 'M':
      max_intensity = MEDIUM_INTENSITY;
      if (LED_MODE) {
        digitalWrite(redPin, LOW);
        digitalWrite(yellowPin, HIGH);
        digitalWrite(greenPin, LOW);
      }
      break;
    case 'C':
      max_intensity = CLOSE_INTENSITY;
      if (LED_MODE) {
        digitalWrite(redPin, LOW);
        digitalWrite(yellowPin, LOW);
        digitalWrite(greenPin, HIGH);
      }
      if (timesWithNeighbour>=random_factor){
        timesWithNeighbour = 0;
        random_factor = random(MIN_RAND, MAX_RAND);
        randomizeColor();
      }
      break;
    case 'R': //R for really close
      if (LED_MODE) {
        digitalWrite(redPin, HIGH);
        digitalWrite(yellowPin, HIGH);
        digitalWrite(greenPin, HIGH);
      }
      break;
  }
}

void doBackgroundStuff() {
  //This method is called as much as possible since we don't have multi-threading
  //HOWEVER, since we track the time using millis(), and since we have defined the incrememnt of how often we should send IDs and update the light,
  //the functions only actually enter the block IF the defined incrememnt of time has occured.

  //so basically, we're spamming these functions everywhere but they don't do anything unless it's time for them to.
  sendPacket();
  updateLight();
  //chip.readAccelData(chip.accelCount);
  //chip.getAres();
  //acc_cache[cache_counter].ax = (float)chip.accelCount[0]*chip.aRes;
  //acc_cache[cache_counter].ay = (float)chip.accelCount[1]*chip.aRes;

  //cache_counter++;
  //if (cache_counter>=CACHE_SIZE){
  //  cache_counter = 0;
  //}
}

void randomizeColor(){
  int r = mypacket.RGB.R;
  int g = mypacket.RGB.G;
  int b = mypacket.RGB.B;
  
  mypacket.RGB.R = random(r/2, r);
  mypacket.RGB.G = random(g/2, g);
  mypacket.RGB.B = random(b/2, b);
}

void sendPacket() {
  if ((millis() - last_id_send) > id_delta) { //Logic for timing things right.
    setWriteMode(); //get into write mode
    packet out = mypacket;
    color c1 = mixColors(ring_colors[0], ring_colors[4]);
    color c2 = mixColors(ring_colors[8], ring_colors[12]);
    out.RGB = mixColors(c1, c2); //mix 4 colors
    radio.write(&mypacket, sizeof(packet)); //send out our ID
    last_id_send = millis(); //track when we sent it
  }
}
void updateLight() {
  if ((millis() - last_pulse) > ((1 / max_intensity) * 1000 * PULSE_LENGTH / 2)) { //same logic as sendID()
    //Simple logic to linearly fade in and out.
    if (up && intensity < max_intensity) {
      intensity++;
    } else if (up && intensity >= max_intensity) {
      intensity--;
      up = false;
    } else if (!up && intensity > 0) {
      intensity--;
    } else {
      intensity++;
      up = true;
    }
    last_pulse = millis(); //track this too
  }

  int r = intensity / 100 * mypacket.RGB.R;
  int g = intensity / 100 * mypacket.RGB.G;
  int b = intensity / 100 * mypacket.RGB.B;

  color invertedColor = {
    ((255-mypacket.RGB.R)<50 ? 50:(255 - mypacket.RGB.R)),
    ((255-mypacket.RGB.G)<50 ? 50:(255 - mypacket.RGB.G)),
    ((255-mypacket.RGB.B)<50 ? 50:(255 - mypacket.RGB.B)),
  };
  //expecting value from 0 to 1;

  //r += (mypacket.RGB.R-invertedColor.R)*accAvg;
  //g += (mypacket.RGB.G-invertedColor.G)*accAvg;
  //b += (mypacket.RGB.G-invertedColor.B)*accAvg;

  // limit color
  if (r>=255) r = 255;
  if (g>=255) g= 255;
  if (b>=255) b = 255;
  
  for (int i = 0; i < 16; i++) {
    ring.setPixelColor(i,r,g,b); //gamma correction
  }
  for (int i = 0; i<8;i++){
    bulb.setPixelColor(i,r,g,b);
  }
  ring.show(); //update our changes
  bulb.show();
  //Serial.print("INTENSITY IS ");
  //Serial.println(intensity);
}

void loop() {
  setLight(captureID()); //run this over and over
}

chipData getChipData(){
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr,14,true);  // request a total of 14 registers
  current_data.ax =Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)     
  current_data.ay =Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  current_data.az = Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  
}

