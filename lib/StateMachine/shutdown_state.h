#ifndef SHUTDOWN_STATE_H
#define SHUTDOWN_STATE_H

#include "state_machine_state.h"

class ShutdownState : public State {
public:
  virtual void onEnter() override;
  virtual void onUpdate() override;
  virtual void onExit() override;
};

#endif