#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

RF24 radio(7, 8); // uno

const uint8_t channel = 0x4c;
const uint64_t pipes[1] = { 0xF0F0F0F0E9LL};
int testnum = 1;

void setup(void)
{
  Serial.begin(57600);
  printf_begin();
  radio.begin();
  radio.setPALevel(RF24_PA_MAX);

  radio.setRetries(15,15);
  radio.setChannel(channel);

  radio.stopListening();
  radio.printDetails();

  radio.openWritingPipe(pipes[1]);
}

void loop()
{
    testnum++;
    bool status = radio.write(&testnum, sizeof(int));
    delay(50);
}
