#include "idle_state.h"

void IdleState::onEnter() {
  Serial.println("[*] Enter State: Idle");
  stateMachinePtr = &StateMachine::getInstance();

  stateMachinePtr->setStatus(STATUS_IDLE);
  Serial.println("[*] Station Status: " + String(stateMachinePtr->getStatusString()));

  ConfigManager::destroy();
  MqttManager::destroy();
  RangingSystem::destroy();
}

void IdleState::onUpdate() {
  StateMachine *stateMachinePtr = &StateMachine::getInstance();
  stateMachinePtr->currentState = stateMachinePtr->setupState;

  IdleState::onExit();
}

void IdleState::onExit() {
  //
}