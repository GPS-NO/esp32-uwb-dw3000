#include "action_state.h"

void ActionState::onEnter() {
  Serial.println("[*] Enter State: Action");
  stateMachinePtr = StateMachine::getInstance();
  stateMachinePtr->setStatus(STATUS_READY);

  configManager = ConfigManager::getInstance();
  mqttManager = MqttManager::getInstance();
  ranging = RangingSystem::getInstance();

  lastHeartbeat = 0;
  subState = IDLE;
  timeout = 10000;

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
    String responder = receivedTopic.substring(5);

    String myID = String((const char *)configManager->deviceConfig.rangingId).substring(0, 4);

    if (!initiator.equals(myID) && !responder.equals(myID)) {
      Serial.println("(ACTION_STATE): no ranging for me: initiator: " + initiator + " responder:" + responder);
      return;
    }

    String rangingType = "responder";
    this->subState = RANING_RESPONDER;
    if (initiator.equals(myID)) {
      rangingType = "init";
      this->subState = RANING_INIT;
    }

    Serial.println("(ACTION_STATE): Ranging operation received type:" + rangingType + " initiator:" + initiator + " responder:" + responder + " myID:" + myID);

    this->initiatorId[0] = (uint8_t)initiator.charAt(0);
    this->initiatorId[1] = (uint8_t)initiator.charAt(1);
    this->initiatorId[2] = (uint8_t)initiator.charAt(2);
    this->initiatorId[3] = (uint8_t)initiator.charAt(3);

    this->responderId[0] = (uint8_t)responder.charAt(0);
    this->responderId[1] = (uint8_t)responder.charAt(1);
    this->responderId[2] = (uint8_t)responder.charAt(2);
    this->responderId[3] = (uint8_t)responder.charAt(3);

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
    const char *status = stateMachinePtr->getStationStateString();
    mqttManager->updateStationStatus(status);
    mqttManager->registerDevice();
  } else if (strPayload.equalsIgnoreCase("restart")) {
    stateMachinePtr->setStatus(STATUS_RESTARTING);
    delay(10);
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

      sprintf(baseRangingTopicBuffer, "broadcast/action/ranging/%c%c%c%c/%c%c%c%c", this->initiatorId[0], this->initiatorId[1], this->initiatorId[2], this->initiatorId[3], this->responderId[0], this->responderId[1], this->responderId[2], this->responderId[3]);

      switch (nextState) {
        case RANING_INIT:
          {
            ranging->setInitiatorId(this->initiatorId);
            ranging->setResponderId(this->responderId);
            int16_t status = ranging->initiateRanging(timeout);
            if (status > 0) {
              Serial.println("(ACTION_STATE): Ranging responded");
              sprintf(topicBuffer, "%s/initiate", baseRangingTopicBuffer);
              mqttManager->publish(topicBuffer, "true");
            } else {
              Serial.printf("(ACTION_STATE): Ranging initiate timeout %d\r\n", status);
              sprintf(topicBuffer, "%s/initiate/timeout", baseRangingTopicBuffer);
              mqttManager->publish(topicBuffer, String(timeout).c_str());
            }
            this->subState = IDLE;
            break;
          }
        case RANING_RESPONDER:
          {
            ranging->setInitiatorId(this->initiatorId);
            ranging->setResponderId(this->responderId);
            float distance = ranging->respondToRanging(timeout);
            if (distance > 0.0f) {
              Serial.print("(ACTION_STATE): Ranging Success: ");
              Serial.print(distance, 2);
              Serial.println("cm");
              sprintf(topicBuffer, "%s/distance", baseRangingTopicBuffer);
              mqttManager->publish(topicBuffer, String(distance).c_str());
            } else {
              Serial.printf("(ACTION_STATE): Ranging timeout %.0f\r\n", distance);
              sprintf(topicBuffer, "%s/responder/timeout", baseRangingTopicBuffer);
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