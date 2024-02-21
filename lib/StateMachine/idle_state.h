#ifndef IDLE_STATE_H
#define IDLE_STATE_H

#include "state_machine_state.h"

class IdleState : public State {
public:
  virtual void onEnter() override;
  virtual void onUpdate() override;
  virtual void onExit() override;
};

#endif