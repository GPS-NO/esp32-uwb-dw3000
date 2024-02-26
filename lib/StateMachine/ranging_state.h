#ifndef RANGING_STATE_H
#define RANGING_STATE_H

#include <Arduino.h>

#include "ranging.h"
#include "mqtt_manager.h"
#include "state_machine_state.h"

class RangingState : public State {
public:
  virtual void onEnter() override;
  virtual void onUpdate() override;
  virtual void onExit() override;

private:
protected:
  MqttManager *mqttManager;
  RangingSystem *ranging;
};

#endif