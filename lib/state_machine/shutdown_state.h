#ifndef SHUTDOWN_STATE_H
#define SHUTDOWN_STATE_H

#include <Arduino.h>

#include "state_machine.h"
#include "ranging.h"

class ShutdownState : public State {
public:
  virtual void onEnter() override;
  virtual void onUpdate() override;
  virtual void onExit() override;

private:
  StateMachine *stateMachinePtr;

  ConfigManager *configManager;
  MqttManager *mqttManager;
  RangingSystem *ranging;
};

#endif