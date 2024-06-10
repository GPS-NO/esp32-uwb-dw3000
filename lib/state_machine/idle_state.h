#ifndef IDLE_STATE_H
#define IDLE_STATE_H

#include <Arduino.h>

#include "state_machine.h"
#include "config_manager.h"
#include "mqtt_manager.h"
#include "ranging.h"

class IdleState : public State {
public:
  virtual void onEnter() override;
  virtual void onUpdate() override;
  virtual void onExit() override;

private:
  StateMachine *stateMachinePtr;
};

#endif