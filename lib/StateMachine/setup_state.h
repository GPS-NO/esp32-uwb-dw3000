#ifndef SETUP_STATE_H
#define SETUP_STATE_H

#include "state_machine_state.h"

class SetupState : public State {
public:
    virtual void onEnter() override;
    virtual void onUpdate() override;
    virtual void onExit() override;
};

#endif