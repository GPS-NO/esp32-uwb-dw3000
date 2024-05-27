#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "state.h"

typedef int StateMachineSubState;

const StateMachineSubState IDLE = 0;

enum StationStateEnum {
  STATUS_RANGING,
  STATUS_IDLE,
  STATUS_READY,
  STATUS_ERROR,
  STATUS_RESTARTING,
  STATUS_SETUP,
  STATUS_OFFLINE
};

class StateMachine {
public:
  static StateMachine& getInstance();
  const char* getStatusString() const;
  void setStatus(StationStateEnum status);

  State* currentState;
  State* idleState;
  State* setupState;
  State* actionState;
  State* errorState;
  State* shutdownState;

private:
  StateMachine();
  StateMachine(const StateMachine&) = delete;
  StateMachine& operator=(const StateMachine&) = delete;

  StationStateEnum stationState;
};

#endif  // STATE_MACHINE_H
