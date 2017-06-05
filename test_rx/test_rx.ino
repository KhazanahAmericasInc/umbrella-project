#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

RF24 radio(7, 8);

const uint8_t channel = 0x4c;
const uint64_t pipes[1] = { 0xF0F0F0F0E9LL};


void setup(void)
{
    Serial.begin(57600);

    printf_begin();
    radio.begin();

    radio.setRetries(15,15);
    radio.setChannel(channel);

    radio.openReadingPipe(1,pipes[1]);
    radio.startListening();
    radio.printDetails();

}

void loop()
{
    if (radio.available())
    {
        int numget = -1;
        radio.read( &numget, sizeof(int) );

        Serial.print("RX: ");
        Serial.println(numget);
    }

    delay(100);
}
