#include <SPI.h>
#include "RF24.h"
#include "codesheet.h"
#include <stdint.h>

// Pins
#define LED_PIN 3
#define RADIO_INTERRUPT_PIN 2

// Constants
byte writeAdress[6] = "Singu";
byte readAdress[6] = "Pingu";
const uint32_t ack = 1;

// Global variables
RF24 radio(9,10);
uint32_t lastmessage = 0;
bool message_in_queue = false;

// Main
void setup() {
    // Serial setup
    Serial.begin(115200);
    Serial.println(F("Finished Serial setup"));

    // Radio setup
    radio.begin();
    //radio.setPALevel(RF24_PA_HIGH);
    radio.enableAckPayload();
    radio.enableDynamicPayloads();
    radio.openWritingPipe(writeAdress);
    radio.openReadingPipe(1, readAdress);

    radio.startListening();
    radio.writeAckPayload(1, &ack, sizeof(uint32_t));
    Serial.println(F("Finished radio setup"));

    // radio interrupt
    pinMode(RADIO_INTERRUPT_PIN, INPUT_PULLUP);
    delay(50);
    attachInterrupt(0, catchMessage, LOW);

    // Pin
    pinMode(LED_PIN, OUTPUT);

    Serial.println(F("Finished setup"));

    digitalWrite(LED_PIN, 1);
    delay(500);
    digitalWrite(LED_PIN, 0);
}

void loop() {
    if (message_in_queue) {
        message_in_queue = false;
        handleMessage(lastmessage);
    }
}

void catchMessage() {
    Serial.println(F("got message!"));
    bool tx,fail,rx;
    radio.whatHappened(tx, fail, rx);
    radio.read(&lastmessage, sizeof(lastmessage));
    radio.writeAckPayload(1, &ack, sizeof(ack));
    message_in_queue = true;
}

// Handles the incoming message. Returns true iff signal was recognized
bool handleMessage(int32_t message_in) {
    Serial.print(F("Message: "));
    Serial.println(message_in);
    if (message_in == 1) {
        digitalWrite(LED_PIN, HIGH);
    } else {
        digitalWrite(LED_PIN, LOW);
    }
    return true;
}
