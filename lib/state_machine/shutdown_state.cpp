#include "shutdown_state.h"

void ShutdownState::onEnter() {
  Serial.println("[*] Enter State: Shutdown");
  stateMachinePtr = StateMachine::getInstance();
  stateMachinePtr->setStatus(STATUS_SHUTDOWN);

  configManager = ConfigManager::getInstance();
  mqttManager = MqttManager::getInstance();
  ranging = RangingSystem::getInstance();
}

void ShutdownState::onUpdate() {
  stateMachinePtr->currentState = stateMachinePtr->idleState;
  this->onExit();
}

void ShutdownState::onExit() {
  mqttManager->unsubscribeAll();

  ConfigManager::destroy();
  MqttManager::destroy();
  RangingSystem::destroy();
}