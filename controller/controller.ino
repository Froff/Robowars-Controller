#include <SPI.h>
#include "RF24.h"
#include "codesheet.h"
#include "math.h"

// Pins ************************************************************************
#define RADIOCE 9
#define RADIOCS 10
#define RADIO_INTERRUPT_PIN 2
#define SWITCH_PIN 3

// Constants *******************************************************************
byte writeAdress[6] = "Pingu";
byte readAdress[6] = "Singu";

// Globals *********************************************************************
RF24 radio(RADIOCE, RADIOCS);

// Function declarations *******************************************************
void sendSignal(const uint32_t & value);
uint32_t composeAndSendMessage();

// Main ************************************************************************
void setup() {
    // Serial setup
    Serial.begin(115200);

    // Radio setup
    radio.begin();
    //radio.setPALevel(RF24_PA_HIGH);
    radio.enableAckPayload();                         // We will be using the Ack Payload feature, so please enable it
    radio.enableDynamicPayloads();                    // Ack payloads are dynamic payloads
    radio.openWritingPipe(writeAdress);
    radio.openReadingPipe(1, readAdress);

    // Settinginterrupts*/
    pinMode(RADIO_INTERRUPT_PIN, INPUT_PULLUP);
    delay(50);
    attachInterrupt(0, radioInterrupt, LOW);             // Attach interrupt handler to interrupt #0 (using pin 2) on BOTH the sender and receiver

    // Switch setup
    pinMode(SWITCH_PIN, INPUT_PULLUP);

    delay(50);
    Serial.println("Setup Done");
}

void loop() {
    uint32_t message = composeAndSendMessage();
    Serial.println(message, BIN);
    delay(100);
}

uint32_t composeAndSendMessage () {
    uint32_t message = composeMessage();
    sendSignal(message);
    return message;
}

uint32_t composeMessage () {
    uint32_t message = 0;
    if (digitalRead(SWITCH_PIN) == HIGH) {
        message = 1;
    }
    return message;
}

void sendSignal(const uint32_t & message_out) {
    radio.startWrite(&message_out, sizeof(uint32_t), 0);
}

void radioInterrupt (void) {
    Serial.println(F("hey"));
    bool tx, fail, rx; // tx: successful transmit. fail: failed transmit. rx: recieved message
    radio.whatHappened(tx, fail, rx); // What happened?

    if ( tx ) { Serial.println(F("sent!")); }

    if ( fail ) { Serial.println(F("FAIL!!")); }

    if ( rx || radio.available()) {
        uint32_t ack_message;
        radio.read(&ack_message, sizeof(ack_message));
        Serial.println(ack_message);
    }
}
