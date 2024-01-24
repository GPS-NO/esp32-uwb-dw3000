#include <Arduino.h>
#include <Wire.h>
#include "state_machine.h"

StateMachine stateMachine;

const int flashBtnPin = 0;

void setup(){
    pinMode(flashBtnPin, INPUT_PULLUP);
}

void loop(){
    State currentState = stateMachine.getState();

    switch(currentState){
        case IDLE:
            printf("IDLE\n");
             if (digitalRead(flashBtnPin) == LOW) {
                printf("PRESSED\n");
            }
            break;
        case SETUP:
            //
            break;
        case RUNNING:
            //
            break;
        case STOPPED:
            //
            break;
        case ERROR:
            //
            break;
        default:
            stateMachine.setState(IDLE);
            break;
    }
    delay(1000);
}