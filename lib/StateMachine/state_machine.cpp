#include "state_machine.h"

StateMachine::StateMachine() {
    this->state = IDLE;
}

void StateMachine::setState(State state) {
    this->state = state;
}

State StateMachine::getState() const {
    return this->state;
}