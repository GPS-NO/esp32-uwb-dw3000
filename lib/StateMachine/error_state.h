#ifndef ERROR_STATE_H
#define ERROR_STATE_H

#include "state_machine_state.h"

class ErrorState : public State {
public:
  virtual void onEnter() override;
  virtual void onUpdate() override;
  virtual void onExit() override;
};

#endif