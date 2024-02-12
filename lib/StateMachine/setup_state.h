#ifndef SETUP_STATE_H
#define SETUP_STATE_H

#include "state_machine_state.h"
#include "config_manager.h"
#include <esp_system.h>
#include <WiFi.h>
#include <Arduino.h>
#include "config_manager.h"
#include "mqtt_manager.h"

class SetupState : public State
{
public:
    virtual void onEnter() override;
    virtual void onUpdate() override;
    virtual void onExit() override;

private:
    void generateId(char *buffer, int length);
    void connectToWiFi(const char *ssid, const char *password);

protected:
    ConfigManager *configManager;
    MqttManager *mqttManager;
};

#endif