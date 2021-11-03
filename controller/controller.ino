#include <SPI.h>
#include "RF24.h"
#include "codesheet.h"
#include "math.h"

// Pins ************************************************************************
#define RADIOCE 9
#define RADIOCS 10
#define RADIO_INTERRUPT_PIN 2

#define SWITCH_PIN 3
#define PIN_MANUALOVERRIDE 3

#define PIN_ELMA_FRONT 4
#define PIN_ELMA_BACKL 5
#define PIN_ELMA_BACKR 6
#define PIN_PUMP_LEFT  7
#define PIN_PUMP_RIGHT 8

#define PIN_FORWARD 4
#define PIN_BACK    5
#define PIN_LEFT    6
#define PIN_RIGHT   7
#define PIN_IDLE    8

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

    pinMode(PIN_FORWARD, INPUT_PULLUP);
    pinMode(PIN_BACK, INPUT_PULLUP);
    pinMode(PIN_LEFT, INPUT_PULLUP);
    pinMode(PIN_RIGHT, INPUT_PULLUP);
    pinMode(PIN_IDLE, INPUT_PULLUP);

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

bool pinIsHigh (uint32_t pin) {
    return digitalRead(pin) == HIGH;
}

uint32_t composeMessage () {
    uint32_t message = 0;
    bool manual_override = digitalRead(PIN_MANUALOVERRIDE) == HIGH;
    if (manual_override) {
        message = CODE_MANUAL_OVERRIDE_bm
                |  (pinIsHigh(PIN_ELMA_FRONT) ? CODE_ELMA_FRONT_bm : 0)
                |  (pinIsHigh(PIN_ELMA_BACKL) ? CODE_ELMA_BACKL_bm : 0)
                |  (pinIsHigh(PIN_ELMA_BACKR) ? CODE_ELMA_BACKR_bm : 0)
                |  (pinIsHigh(PIN_PUMP_LEFT)  ? CODE_PUMP_LEFT_bm  : 0)
                |  (pinIsHigh(PIN_PUMP_RIGHT) ? CODE_PUMP_RIGHT_bm : 0)
                ;
    } else {
        message = CODE_MOVE_FORWARD_bm;
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
