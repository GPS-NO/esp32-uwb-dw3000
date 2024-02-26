#include "setup_state.h"

void SetupState::onEnter() {
  Serial.println("[*] Enter State: Setup");

  configManager = ConfigManager::getInstance();
  mqttManager = MqttManager::getInstance();
  ranging = RangingSystem::getInstance();
  rangingInitResult = ranging->init(configManager->deviceConfig.rangingId, PIN_IRQ, PIN_RST, PIN_SS);

  if (!mqttManager->isConfigAvailable()) {
    Serial.println("[*] No configuration found. Entering configuration mode.");
    delay(2500);
    this->onEnter();
    //Welche Routine müsste man umsetzen, falls keine Config existiert?
  }

  mqttManager->connect();
}

void SetupState::onUpdate() {
  if (healthCheck()) {
    mqttManager->registerDevice();
    mqttManager->loop();

    StateMachineState::currentState = StateMachineState::actionState;
    SetupState::onExit();
    return;
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

void SetupState::onExit() {
  mqttManager->unsubscribeAll();
}