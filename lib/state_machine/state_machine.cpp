#include "state_machine.h"
#include "idle_state.h"
#include "setup_state.h"
#include "action_state.h"
#include "error_state.h"
#include "shutdown_state.h"

StateMachine* StateMachine::instance = nullptr;

StateMachine::StateMachine() {
  idleState = new IdleState();
  setupState = new SetupState();
  actionState = new ActionState();
  errorState = new ErrorState();
  shutdownState = new ShutdownState();
  currentState = StateMachine::idleState;

  mqttManager = MqttManager::getInstance();
}

StateMachine* StateMachine::getInstance() {
  if (instance == NULL) {
    instance = new StateMachine();
  }

  return instance;
}

void StateMachine::setStatus(StationStateEnum status) {
  stationState = status;

  const char* stationStateString = getStationStateString();
  if (mqttManager != NULL) {
    mqttManager->updateStationStatus(stationStateString);
  }
}

const char* StateMachine::getStationStateString() const {
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
    case STATUS_SHUTDOWN:
      return "SHUTDOWN";
    default: return "UNKNOWN";
  }
}