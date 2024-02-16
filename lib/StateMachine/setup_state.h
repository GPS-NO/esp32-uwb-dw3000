#ifndef SETUP_STATE_H
#define SETUP_STATE_H

#include <Arduino.h>
#include <WiFi.h>
#include <esp_system.h>

#include "boarddefines.h"
#include "config_manager.h"
#include "mqtt_manager.h"
#include "ranging.h"
#include "state_machine_state.h"

class SetupState : public State {
   public:
    virtual void onEnter() override;
    virtual void onUpdate() override;
    virtual void onExit() override;

   private:
    void connectToWiFi(const char *ssid, const char *password);

   protected:
    ConfigManager *configManager;
    MqttManager *mqttManager;
    RangingSystem ranging;
};

#endif