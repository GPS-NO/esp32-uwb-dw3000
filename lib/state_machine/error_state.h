#ifndef ERROR_STATE_H
#define ERROR_STATE_H

#include <Arduino.h>

#include "state_machine.h"


class ErrorState : public State {
public:
  virtual void onEnter() override;
  virtual void onUpdate() override;
  virtual void onExit() override;

private:
  StateMachine *stateMachinePtr;
};

#endif