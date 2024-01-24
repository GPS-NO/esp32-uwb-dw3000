#ifndef STATEMACHINE_H
#define STATEMACHINE_H

enum State {
    IDLE,
    SETUP,
    RUNNING,
    STOPPED,
    ERROR
};

class StateMachine {
    private:
        State state;

    public:
        StateMachine();
        State getState() const;
        void setState(State state);
};

#endif // STATEMACHINE_H
