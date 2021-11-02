#include <SPI.h>
#include "RF24.h"
#include "codesheet.h"
#include <stdint.h>

// Pins
#define RADIOCE 9
#define RADIOCS 10
#define RADIO_INTERRUPT_PIN 2
#define LED_PIN 3

#define ELMA_FRONT 3
#define ELMA_BACKL 7
#define ELMA_BACKR 8
#define PUMP_LEFT  5
#define PUMP_RIGHT 6

// Abatper constants
#define PUMPDESE_PWM_MAX 128
#define ELMADESE_PWM_MAX 200

// Radio constants
byte writeAdress[6] = "Singu";
byte readAdress[6] = "Pingu";
const uint32_t ack = 1;

// Enums
typedef enum {
  SMARTSYSTEM3000,
  MANUALOVERRIDE
} InputMode;

typedef enum {
  IDLE,
  FORWARD,
  LEFT,
  RIGHT,
  BACK
} RopovState;

// Global variables
RF24 radio(RADIOCE, RADIOCS);
uint32_t lastmessage = 0;
bool message_in_queue = false;

InputMode inputMode = SMARTSYSTEM3000;
RopovState state = IDLE, nextState = IDLE;

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
    if (inputMode == SMARTSYSTEM3000) {
        analogWrite(ELMA_FRONT, ELMADESE_PWM_MAX);
        analogWrite(ELMA_BACKL, ELMADESE_PWM_MAX);
        analogWrite(ELMA_BACKR, ELMADESE_PWM_MAX);
        analogWrite(PUMP_LEFT,  PUMPDESE_PWM_MAX);
        analogWrite(PUMP_RIGHT, PUMPDESE_PWM_MAX);
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
    inputMode = message_in & CODE_MANUAL_OVERRIDE_bm ? MANUALOVERRIDE : SMARTSYSTEM3000;
    if (inputMode == SMARTSYSTEM3000) {
        Serial.println(F("SMART-SYSTEM-3000"));
        if (message_in & CODE_MOVE_FORWARD_bm) {
            state = FORWARD;
        } else if (message_in & CODE_MOVE_BACK_bm) {
            state = BACK;
        } else if (message_in & CODE_MOVE_LEFT_bm) {
            state = LEFT;
        } else if (message_in & CODE_MOVE_RIGHT_bm) {
            state = RIGHT;
        } else {
            state = IDLE;
        }
    } else {
        Serial.println(F("MANUAL OVERRIDE"));
        analogWrite(ELMA_FRONT, message_in & CODE_ELMA_FRONT_bm ? ELMADESE_PWM_MAX : 0);
        analogWrite(ELMA_BACKL, message_in & CODE_ELMA_BACKL_bm ? ELMADESE_PWM_MAX : 0);
        analogWrite(ELMA_BACKR, message_in & CODE_ELMA_BACKR_bm ? ELMADESE_PWM_MAX : 0);
        analogWrite(PUMP_LEFT,  message_in & CODE_PUMP_LEFT_bm  ? PUMPDESE_PWM_MAX : 0);
        analogWrite(PUMP_RIGHT, message_in & CODE_PUMP_RIGHT_bm ? PUMPDESE_PWM_MAX : 0);
    }
    Serial.println();
    return true;
}
