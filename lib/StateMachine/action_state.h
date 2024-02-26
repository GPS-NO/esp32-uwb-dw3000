#ifndef ACTION_STATE_H
#define ACTION_STATE_H

#include <Arduino.h>

#include "ranging.h"
#include "mqtt_manager.h"
#include "state_machine_state.h"

class ActionState : public State {
public:
  virtual void onEnter() override;
  virtual void onUpdate() override;
  virtual void onExit() override;

private:
  unsigned long lastHeartbeat;

protected:
  MqttManager *mqttManager;
  RangingSystem *ranging;
};

#endif