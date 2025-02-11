#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "state.h"
#include "mqtt_manager.h"

typedef int StateMachineSubState;

const StateMachineSubState IDLE = 0;

enum StationStateEnum {
  STATUS_RANGING,
  STATUS_IDLE,
  STATUS_READY,
  STATUS_ERROR,
  STATUS_SETUP,
  STATUS_SHUTDOWN,
  STATUS_RESTARTING,
  STATUS_OFFLINE
};

class StateMachine {
public:
  State* currentState;
  State* idleState;
  State* setupState;
  State* actionState;
  State* errorState;
  State* shutdownState;

  static StateMachine* getInstance();
  ~StateMachine();
  const char* getStationStateString() const;
  void setStatus(StationStateEnum status);

private:
  static StateMachine* instance;
  StateMachine();
  StationStateEnum stationState;
  MqttManager* mqttManager;
};

#endif  // STATE_MACHINE_H
