#include "shutdown_state.h"

void ShutdownState::onEnter() {}

void ShutdownState::onUpdate() {
  StateMachineState::currentState = StateMachineState::idleState;
  ShutdownState::onExit();
}

void ShutdownState::onExit() {}