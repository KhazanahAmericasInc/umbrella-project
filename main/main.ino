#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include "nRF24L01.h" 
#include "RF24.h"
#include "config.h"

//Somehow including this ruins everything
#include "I2C.h" 

RF24 radio(6, 7); // uno


void getChipData(){
  I2c.read(MPU_addr, 0x3B, 6);
  cache[cache_counter].ax=(int)(I2c.receive()<<8|I2c.receive());  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
  cache[cache_counter].ay=(int)(I2c.receive()<<8|I2c.receive());
  cache[cache_counter].az=(int)(I2c.receive()<<8|I2c.receive());
  
  cache_counter++;
  if (cache_counter>CACHE_SIZE-1){
    cache_counter = 0;
  }
}

void setup() {

  I2c.begin();
  I2c.write(MPU_addr, 0x6B, 0);

  Serial.begin(9600);
  ring.begin(); //init ring
  ring.show(); //show nothing
  last_pulse = millis(); //set to now
  last_id_send = millis(); //set to now

  //RF24 STUFF

  radio.begin(); //init the radio
  //radio.setDataRate(RF24_250KBPS);
  //radio.setPALevel(RF24_PA_MIN);
  radio.setRetries(5, 5); //try to send 100 times with 0 delay.  This doesn't seem to affect things too much.
  radio.setChannel(channel); //set the channel
//  printf_begin();

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
    char ids[2000]; //empty string
    int index = 0;
    float t = millis(); //keep track of when we started the test

    while (millis() - t < 1000.0 * TEST_LENGTH) { //while the time doesn't exceed our defined test length
      doBackgroundStuff(); //see definition
      packet p;
      setReadMode();
      if (radio.available()){
        radio.read(&p, sizeof(packet)); //Write to the address of our packet
        //Serial.println((char)(p.ID-37));
        if ((char)(p.ID-32)==mypacket.ID){
          //BEACON FOUND
          originalColor = p.RGB;
          mypacket.RGB = originalColor;
        }
        ids[index] = p.ID; //Add on the character to our string
        index++;
        String rgb_string = "";
        Serial.print(p.ID);
        rgb_data[(int)p.ID] = p;
      }
    }
    for (int j = 0; j <= index; j++) { //Now we "sort" our string of IDS
      doBackgroundStuff(); //see definition
      int value = (int) ids[j]; //get binary value of the ID.  We will use this as the index.  Basically a hash table where casting is our hash function
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
  Serial.println(highest); //for debugging
  Serial.print("COLOR IS ");
  Serial.print(mypacket.RGB.R);
  Serial.print(" ");
  Serial.print(mypacket.RGB.G);
  Serial.print(" ");
  Serial.println(mypacket.RGB.B);
  

  //gets sent to setLight()

  
  if (highest < MEDIUM_SIGNAL_AMOUNT) {
    if (timesWithoutNeighbour>=LONELY_CONSTANT){
      decay();
      timesWithoutNeighbour++;
    }
    isMedium = false;
    return 'F';
  }
  if (highest < CLOSE_SIGNAL_AMOUNT) {
    if (!isMedium){
      mixWithNeighbour(indexOfHighest, 20);
      isMedium = true;
    }
    return 'M';
  }
  timesWithoutNeighbour = 0;
  mixWithNeighbour(indexOfHighest, 50);
  isMedium = false;
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


void mixWithNeighbour(int index, float mixaAmount) { //mixes the unit's color with the nearest unit's color mixAmount from 0-100 - percentage of first color
  doBackgroundStuff();
  mypacket.RGB = mixColors(rgb_data[index].RGB, mypacket.RGB, mixAmount/100.0);
}

void decay(){
  doBackgroundStuff();
  mypacket.RGB = mixColors(mypacket.RGB, originalColor);
}

color mixColors (color c1, color c2, float percentageFirst) { //simple color mixing algo
  color mixed;
  mixed.R = (c1.R*percentageFirst)+(c2.R*(1.0-percentageFirst));
  mixed.G = (c1.G*percentageFirst)+(c2.G*(1.0-percentageFirst));
  mixed.B = (c1.B*percentageFirst)+(c2.B*(1.0-percentageFirst));
  return mixed;
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
  }
}

void doBackgroundStuff() {
  //This method is called as much as possible since we don't have multi-threading
  //HOWEVER, since we track the time using millis(), and since we have defined the incrememnt of how often we should send IDs and update the light,
  //the functions only actually enter the block IF the defined incrememnt of time has occured.

  //so basically, we're spamming these functions everywhere but they don't do anything unless it's time for them to.
  sendPacket();
  updateLight();
  getChipData();
  
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

  

  chipData devs = getStandardDeviations();

  mcache[mcache_counter] = ((float)devs.ax/9000.0 + (float)devs.ax/9000.0 + (float)devs.ax/9000.0)/3.0;
  mcache_counter++;
  if (mcache_counter>=CACHE_SIZE-1) mcache_counter = 0;
  

  float multiplier = 0;
  for (int i = 0; i<CACHE_SIZE; i++){
    multiplier+=mcache[i];
  }
  multiplier/=CACHE_SIZE;
  color opposite = {
    255,0,0
  };

  r = (opposite.R*multiplier)+(r*(1.0-multiplier));
  g = (opposite.G*multiplier)+(g*(1.0-multiplier));
  b = (opposite.B*multiplier)+(b*(1.0-multiplier));
  
  for (int i = 0; i < 16; i++) {
    ring.setPixelColor(i,r,g,b); //gamma correction
  }
  ring.show(); //update our changes
  //Serial.print("INTENSITY IS ");
  //Serial.println(intensity);
}

void loop() {
  setLight(captureID()); //run this over and over
}

chipData getStandardDeviations (){
  
  long meanX = 0;
  long meanY = 0;
  long meanZ = 0;

  for (int i = 0; i<CACHE_SIZE; i++){
    meanX+=(long)cache[i].ax;
    meanY+=(long)cache[i].ay; 
    meanZ+=(long)cache[i].az; 
  }
  

  meanX/=CACHE_SIZE;
  
  meanY/=CACHE_SIZE;

  meanZ/=CACHE_SIZE;

  long sqDevSumX = 0;
  long sqDevSumY = 0;
  long sqDevSumZ = 0;

  for (int i = 0; i<CACHE_SIZE;i++){
    sqDevSumX+= ((cache[i].ax-meanX)*(cache[i].ax-meanX));
    sqDevSumY+= ((cache[i].ay-meanY)*(cache[i].ay-meanY));
    sqDevSumZ+= ((cache[i].az-meanZ)*(cache[i].az-meanZ));
  }

  return  
  { sqrt(abs(sqDevSumX/CACHE_SIZE)),
    sqrt(abs(sqDevSumY/CACHE_SIZE)),
    sqrt(abs(sqDevSumZ/CACHE_SIZE))
  };
}

