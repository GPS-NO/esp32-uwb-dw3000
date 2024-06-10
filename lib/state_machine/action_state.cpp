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

  sprintf(topicBuffer, "broadcast/action/ranging");
  mqttManager->subscribe(topicBuffer, [&](const char *topic, const char *payload) {
    StationMode mode = ranging->getStationMode();

    if (mode == STATION_MODE_UNSET) {
      sprintf(topicBuffer, "%s/dwt/mode", mqttManager->getBaseTopic().c_str());

      mqttManager->publish(topicBuffer, ranging->getStationModeChar());
      ranging->setStationMode(STATION_MODE_INITIATOR);
    }

    uint8_t status = ranging->init(STATION_MODE_INITIATOR, this->initiatorID);
    Serial.println("Ranging init status: " + String(status));
    mqttManager->publish(topicBuffer, ranging->getStationModeChar());
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
    this->subState = RANING_RESPOND;
    if (initiator.equals(myID)) {
      rangingType = "init";
      this->subState = RANING_INIT;
    }

    Serial.println("(ACTION_STATE): Ranging operation received type:" + rangingType + " initiator:" + initiator + " responder:" + responder + " myID:" + myID + "");

    initiatorID[0] = (uint8_t)initiator.charAt(0);
    initiatorID[1] = (uint8_t)initiator.charAt(1);
    initiatorID[2] = (uint8_t)initiator.charAt(2);
    initiatorID[3] = (uint8_t)initiator.charAt(3);

    responderID[0] = (uint8_t)responder.charAt(0);
    responderID[1] = (uint8_t)responder.charAt(1);
    responderID[2] = (uint8_t)responder.charAt(2);
    responderID[3] = (uint8_t)responder.charAt(3);

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
    const char *status = stateMachinePtr->getStationStateChar();
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

      switch (nextState) {
        case RANING_INIT:
          {
            ranging->setInitiatorId(initiatorID);
            ranging->setResponderId(responderID);

            ranging->init(STATION_MODE_INITIATOR, configManager->deviceConfig.rangingId);


            float distance = ranging->initiateRanging(timeout);
            if (distance >= 0.0f) {
              Serial.print("(ACTION_STATE): Ranging Success: ");
              Serial.print(distance, 2);
              Serial.println("m");
              // TODO: send distance
            } else {
              Serial.printf("(ACTION_STATE): Ranging initiate timeout %.2f\r\n", distance);
              // TODO: send timeout
            }
            this->subState = IDLE;
            break;
          }
        case RANING_RESPOND:
          {
            ranging->setInitiatorId(initiatorID);
            ranging->setResponderId(responderID);

            ranging->init(STATION_MODE_RESPONDER, configManager->deviceConfig.rangingId);


            int16_t status = ranging->respondToRanging(timeout);
            if (status > 0) {
              Serial.println("(ACTION_STATE): Ranging responded");
              // TODO: send okay
            } else {
              Serial.printf("(ACTION_STATE): Ranging respond timeout %d\r\n", status);
              // TODO: send timeout
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
