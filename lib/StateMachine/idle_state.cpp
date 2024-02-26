#include "idle_state.h"

#include <Arduino.h>

void IdleState::onEnter() {
  Serial.println("[*] Enter State: Idle");

  /*delete ConfigManager::getInstance();
  delete MqttManager::getInstance();*/
  //RangingSystem::getInstance()->destroy();
}

void IdleState::onUpdate() {
  StateMachineState::currentState = StateMachineState::setupState;
  IdleState::onExit();
}

void IdleState::onExit() {
  //
}