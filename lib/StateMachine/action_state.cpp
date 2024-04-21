#include "action_state.h"

void ActionState::onEnter() {
  Serial.println("[*] Enter State: Action");

  configManager = ConfigManager::getInstance();
  mqttManager = MqttManager::getInstance();
  ranging = RangingSystem::getInstance();

  lastHeartbeat = 0;
  subState = IDLE;
  timeout = 10000;

  otherID[0] = (uint8_t)'A';
  otherID[1] = (uint8_t)'B';
  otherID[2] = (uint8_t)'C';
  otherID[3] = (uint8_t)'D';

  char topicBuffer[128];
  sprintf(topicBuffer, "%s/action", mqttManager->getBaseTopic().c_str());
  mqttManager->subscribe(topicBuffer, [&](const char *topic, const char *payload) {
    this->onAction(payload);
  });

  sprintf(topicBuffer, "broadcast/action");
  mqttManager->subscribe(topicBuffer, [&](const char *topic, const char *payload) {
    this->onAction(payload);
  });

  sprintf(topicBuffer, "broadcast/action/ranging/+/+");
  mqttManager->subscribe(topicBuffer, [&](const char *topic, const char *payload) {
    Serial.println("(ACTION_STATE) rangingCallback");

    String receivedTopic = String(topic);
    size_t baseTopicLength = String("broadcast/action/ranging").length();
    receivedTopic.remove(0, baseTopicLength + 1);

    String initiator = receivedTopic.substring(0, 4);
    String ranger = receivedTopic.substring(5);

    String myID = String((const char *)configManager->deviceConfig.rangingId).substring(0, 4);

    if (!initiator.equals(myID) && !ranger.equals(myID)) {
      Serial.println("(ACTION_STATE): no ranging for me: initiator: " + initiator + " ranger:" + ranger);
      return;
    }

    String otherID = initiator;
    String rangingType = "ranging";
    this->subState = RANING_RESPOND;
    if (initiator.equals(myID)) {
      otherID = ranger;
      rangingType = "init";
      this->subState = RANING_INIT;
    }

    Serial.println("(ACTION_STATE): Ranging operation received type:" + rangingType + " initiator:" + initiator + " ranger:" + ranger + " (myID:" + myID + " oID:" + otherID + ")");

    this->otherID[0] = (uint8_t)otherID.charAt(0);
    this->otherID[1] = (uint8_t)otherID.charAt(1);
    this->otherID[2] = (uint8_t)otherID.charAt(2);
    this->otherID[3] = (uint8_t)otherID.charAt(3);

    int timeout = String(payload).toInt();
    if (timeout > 0) this->timeout = timeout;
    else Serial.println("(ACTION_STATE): unknown timeout payload");
  });
}

void ActionState::onAction(const char *payload) {
  String strPayload = String(payload);
  strPayload.trim();
  Serial.println("(ACTION_STATE) onAction : " + strPayload);
  if (strPayload.equalsIgnoreCase("ping")) {
    mqttManager->registerDevice();
  } else if (strPayload.equalsIgnoreCase("restart")) {
    ESP.restart();
  }
}

void ActionState::onUpdate() {
  while (1) {
    if (Serial.available() > 0 || subState != IDLE) {
      int nextState = 0;
      if (subState != IDLE) nextState = subState;
      else if (Serial.available() > 0) nextState = Serial.parseInt();

      char baseRangingTopicBuffer[128];
      char topicBuffer[128];

      switch (nextState) {
        case RANING_INIT:
          {
            int16_t status = ranging->initiateRanging(otherID, timeout);
            sprintf(baseRangingTopicBuffer, "%s/ranging/%c%c%c%c", mqttManager->getBaseTopic().c_str(), otherID[0], otherID[1], otherID[2], otherID[3]);
            if (status > 0) {
              Serial.println("(ACTION_STATE): Ranging responded");
              //sprintf(topicBuffer, "%s/result", baseRangingTopicBuffer);
              //mqttManager->publish(topicBuffer, "true");
            } else {
              Serial.println("(ACTION_STATE): Ranging initiate timeout");
              sprintf(topicBuffer, "%s/initiate/timeout", baseRangingTopicBuffer);
              mqttManager->publish(topicBuffer, String(timeout).c_str());
            }
            this->subState = IDLE;
            break;
          }
        case RANING_RESPOND:
          {
            float distance = ranging->respondToRanging(otherID, timeout);
            sprintf(baseRangingTopicBuffer, "%s/ranging/%c%c%c%c", mqttManager->getBaseTopic().c_str(), otherID[0], otherID[1], otherID[2], otherID[3]);
            if (distance > 0.0f) {
              Serial.print("(ACTION_STATE): Ranging Success: ");
              Serial.print(distance, 2);
              Serial.println("cm");
              sprintf(topicBuffer, "%s/distance", baseRangingTopicBuffer);
              mqttManager->publish(topicBuffer, String(distance).c_str());
            } else {
              Serial.println("(ACTION_STATE): Ranging timeout");
              sprintf(topicBuffer, "%s/respond/timeout", baseRangingTopicBuffer);
              mqttManager->publish(topicBuffer, String(timeout).c_str());
            }
            this->subState = IDLE;
            break;
          }
        default:
          Serial.println("(ACTION_STATE): unknown state");
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
    delay(10);
  }
}

void ActionState::onExit() {
  mqttManager->unsubscribeAll();
}