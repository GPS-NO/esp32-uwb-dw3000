#include <Arduino.h>

#include "boarddefines.h"
#include "ranging.h"

uint8_t otherID[] = {'5', '6', '7', '8'};

RangingSystem rangingSystem(otherID);

void setup() {
    Serial.begin(MON_SPEED);

    Serial.println("setup: init responder");
    int8_t initResult = rangingSystem.init(PIN_IRQ, PIN_RST, PIN_SS);
    if (initResult == 0)
        Serial.println("setup: Ranging system initialized successfully.");
    else
        Serial.println("setup: Failed to initialize ranging system.");
}

void loop() {
    Serial.println("loop: running in initiator.");
    while (true) {
        int16_t result = rangingSystem.initiateRanging(7500);
        if (result > 0) {
            Serial.println("loop: Ranging initiated successfully.");
            break;
        } else {
            Serial.println("loop: Failed to initiate ranging.");
        }
    }

    Serial.println("loop: running in responder.");
    while (true) {
        float distance = rangingSystem.respondToRanging(7500);
        if (distance > 0.0f) {
            Serial.println("loop: Ranging response sent successfully.");
            Serial.print("loop: Distance: ");
            Serial.print(distance);
            Serial.println(" cm");
            break;
        } else {
            Serial.println("loop: Failed to respond to ranging.");
        }
    }
}
