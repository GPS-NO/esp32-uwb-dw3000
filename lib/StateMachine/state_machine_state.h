#ifndef STATE_MACHINE_STATE_H
#define STATE_MACHINE_STATE_H

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
    static State* rangingState;
};


#endif // STATE_MACHINE_STATE_H
