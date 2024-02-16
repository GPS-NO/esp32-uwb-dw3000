#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include "config_manager.h"
#include "boarddefines.h"

class MqttManager
{
private:
    static MqttManager *instance;
    WiFiClient wifiClient;
    PubSubClient mqttClient;

    MqttManager();

public:
    static MqttManager *getInstance();

    void publish(const char *topic, const char *payload);
    void subscribe(const char *topic);
    void connect();
    void setupWifi(const char *ssid, const char *password, int maxAttempts, int attemptDelay);
    void registerDevice();
    bool messageReceived();
    String getMessageTopic();
    String getMessagePayload();

protected:
    ConfigManager *configManager;
};

#endif
