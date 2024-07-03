#include "setup_state.h"

void SetupState::onEnter() {
  Serial.println("[*] Enter State: Setup");
  stateMachinePtr = StateMachine::getInstance();
  stateMachinePtr->setStatus(STATUS_SETUP);

  configManager = ConfigManager::getInstance();
  mqttManager = MqttManager::getInstance();
  ranging = RangingSystem::getInstance();
  rangingInitResult = ranging->init(PIN_IRQ, PIN_RST, PIN_SS);

  while (!mqttManager->isConfigAvailable()) {
    Serial.println("[*] No configuration found. Entering configuration mode.");
    delay(2500);
  }
}

void SetupState::onUpdate() {
  if (healthCheck()) {
    mqttManager->sendHeartbeat();
    mqttManager->registerDevice();

    stateMachinePtr->currentState = stateMachinePtr->actionState;
  }
  delay(2500);
  //Should it step back to the initial state if the health check fails?
}

bool SetupState::healthCheck() {
  if (!mqttManager->isWifiConnected()) {
    Serial.println("[ERROR] WiFi connection is not established.");
    return false;
  }

  if (!mqttManager->isConnected()) {
    Serial.println("[ERROR] MQTT connection is not established.");
    return false;
  }

  if (rangingInitResult != 0) {
    Serial.println("[ERROR] Failed to initialize ranging system.");
    return false;
  }

  return true;
}

void SetupState::onExit() {}