#include "ranging_state.h"

void RangingState::onEnter() {
  Serial.println("[*] Enter State: Idle");
  ranging = RangingSystem::getInstance();
}

void RangingState::onUpdate() {
  Serial.println("================================");
  Serial.println("1: Wechsel in State: Initator");
  Serial.println("2: Wechsel in State: Ranger");
  Serial.println("3: Wechsel in State: Error");
  Serial.println("================================");

  while (1) {
    if (Serial.available() > 0) {
      int serialInput = Serial.parseInt();
      uint8_t otherID[] = { 'A', 'B', 'C', 'D' };
      uint32_t timeout = 10000;
      switch (serialInput) {
        case 1:
          {
            int16_t status = ranging->initiateRanging(otherID, timeout);
            if (status > 0)
              Serial.println("Ranging responded");
            else
              Serial.println("Ranging initiate timeout");
            StateMachineState::currentState = StateMachineState::setupState;
            RangingState::onExit();
            return;
          }
        case 2:
          {
            float distance = ranging->respondToRanging(otherID, timeout);
            if (distance > 0.0f) {
              Serial.print("Ranging Success: ");
              Serial.print(distance, 2);
              Serial.println("cm");
            } else
              Serial.println("Ranging timeout");
            StateMachineState::currentState = StateMachineState::setupState;
            RangingState::onExit();
            return;
          }
        case 3:
          Serial.println("Dieser State existiert noch nicht!");
          break;
        default:
          Serial.println("Ungültige Kombination.");
          break;
      }
    }
    delay(100);
  }
}

void RangingState::onExit() {
  //
}