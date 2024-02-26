#ifndef STATE_MACHINE_STATE_H
#define STATE_MACHINE_STATE_H

typedef int StateMachineSubState;

const StateMachineSubState IDLE = 0;

class State {
public:
  virtual void onEnter() = 0;
  virtual void onUpdate() = 0;
  virtual void onExit() = 0;
  virtual ~State() {}
};

class StateMachineState {
public:
  static State* currentState;
  static State* idleState;
  static State* setupState;
  static State* actionState;
  static State* errorState;
  static State* shutdownState;
};


#endif  // STATE_MACHINE_STATE_H
