#include "action_state.h"

void ActionState::onEnter() {
  Serial.println("[*] Enter State: Action");

  mqttManager = MqttManager::getInstance();
  ranging = RangingSystem::getInstance();

  lastHeartbeat = 0;

  char topicBuffer[128];
  sprintf(topicBuffer, "%s/ranging/+/+", mqttManager->getBaseTopic().c_str());
  mqttManager->subscribe(topicBuffer, [](const char *topic, const char *payload) {
    Serial.println("(SETUP_STATE): " + String(payload));
    String receivedTopic = String(topic);
    size_t baseTopicLength = (MqttManager::getInstance()->getBaseTopic() + "/ranging").length();
    receivedTopic.remove(0, baseTopicLength + 1);

    String otherID = receivedTopic.substring(0, 4);
    String rangingType = receivedTopic.substring(5);

    Serial.println("(SETUP_STATE): " + receivedTopic + " " + rangingType + " " + otherID);

    if (rangingType.equalsIgnoreCase("init")) {
      // 1
    } else if (rangingType.equalsIgnoreCase("ranging")) {
      // 2
    }
  });
}

void ActionState::onUpdate() {
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
            ActionState::onExit();
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
            ActionState::onExit();
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

    // LOOP
    // if x/y ÄNDER
    if (millis() - lastHeartbeat >= 30 * 1000) {
      mqttManager->sendHeartbeat();
      lastHeartbeat = millis();
    }

    mqttManager->loop();
    delay(100);
  }
}

void ActionState::onExit() {
  mqttManager->unsubscribeAll();
}