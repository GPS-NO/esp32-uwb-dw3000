#include "idle_state.h"

#include <Arduino.h>

void IdleState::onEnter() {
  Serial.println("[*] Enter State: Idle");

  ConfigManager::destroy();
  MqttManager::destroy();
  RangingSystem::destroy();
}

void IdleState::onUpdate() {
  StateMachineState::currentState = StateMachineState::setupState;
  IdleState::onExit();
}

void IdleState::onExit() {
  //
}