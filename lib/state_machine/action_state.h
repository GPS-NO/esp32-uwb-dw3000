#ifndef ACTION_STATE_H
#define ACTION_STATE_H

#include <Arduino.h>

#include "ranging.h"
#include "mqtt_manager.h"
#include "state_machine.h"

const StateMachineSubState RANING_INIT = 10;
const StateMachineSubState RANING_RESPONDER = 11;

class ActionState : public State {
public:
  virtual void onEnter() override;
  virtual void onUpdate() override;
  virtual void onExit() override;

private:
  StateMachine *stateMachinePtr;

  void onAction(const char *payload);

  unsigned long lastHeartbeat;
  StateMachineSubState subState;
  uint8_t initiatorId[4];
  uint8_t responderId[4];
  uint32_t timeout;
  bool isRunning;

protected:
  MqttManager *mqttManager;
  ConfigManager *configManager;
  RangingSystem *ranging;
};

#endif