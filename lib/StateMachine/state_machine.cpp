#include "state_machine.h"
#include "idle_state.h"
#include "setup_state.h"
#include "action_state.h"

StateMachine::StateMachine() {
  idleState = new IdleState();
  setupState = new SetupState();
  actionState = new ActionState();
  currentState = StateMachine::idleState;

  stationState = STATUS_SETUP;
}

StateMachine& StateMachine::getInstance() {
  static StateMachine instance;
  return instance;
}

void StateMachine::setStatus(StationStateEnum status) {
  stationState = status;
}

const char* StateMachine::getStatusString() const {
  switch (stationState) {
    case STATUS_RANGING:
      return "RANGING";
    case STATUS_IDLE:
      return "IDLE";
    case STATUS_READY:
      return "READY";
    case STATUS_ERROR:
      return "ERROR";
    case STATUS_RESTARTING:
      return "RESTARTING";
    case STATUS_SETUP:
      return "SETUP";
    case STATUS_OFFLINE:
      return "OFFLINE";
    default:
      return "INVALID";
  }
}