#include "ranging_state.h"

void RangingState::onEnter() {
  Serial.println("[*] Enter State: Idle");
  mqttManager = MqttManager::getInstance();
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
      char baseRangingTopicBuffer[128];
      char topicBuffer[128];

      switch (serialInput) {
        case 1:
          {
            int16_t status = ranging->initiateRanging(otherID, timeout);
            sprintf(baseRangingTopicBuffer, "%s/ranging/%c%c%c%c/initiate", mqttManager->getBaseTopic(), otherID[0], otherID[1], otherID[2], otherID[3]);
            if (status > 0) {
              Serial.println("Ranging responded");
              sprintf(topicBuffer, "%s/result", baseRangingTopicBuffer);
              mqttManager->publish(topicBuffer, "true");
            } else {
              Serial.println("Ranging initiate timeout");
              sprintf(topicBuffer, "%s/timeout", baseRangingTopicBuffer);
              mqttManager->publish(topicBuffer, String(timeout).c_str());
            }
            StateMachineState::currentState = StateMachineState::setupState;
            RangingState::onExit();
            return;
          }
        case 2:
          {
            float distance = ranging->respondToRanging(otherID, timeout);
            sprintf(baseRangingTopicBuffer, "%s/ranging/%c%c%c%c/respond", mqttManager->getBaseTopic(), otherID[0], otherID[1], otherID[2], otherID[3]);
            if (distance > 0.0f) {
              Serial.print("Ranging Success: ");
              Serial.print(distance, 2);
              Serial.println("cm");
              sprintf(topicBuffer, "%s/result", baseRangingTopicBuffer);
              mqttManager->publish(topicBuffer, String(distance).c_str());
            } else {
              Serial.println("Ranging timeout");
              sprintf(topicBuffer, "%s/timeout", baseRangingTopicBuffer);
              mqttManager->publish(topicBuffer, String(timeout).c_str());
            }
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
  mqttManager->unsubscribeAll();
}