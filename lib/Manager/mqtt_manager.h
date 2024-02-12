#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "config_manager.h"

class MqttManager
{
private:
    static MqttManager *instance;
    const char *mqttServer;
    const int mqttPort;
    WiFiClient wifiClient;
    PubSubClient mqttClient;

    MqttManager(const char *server, int port);

public:
    static MqttManager *getInstance(const char *server, int port);

    void publish(const char *topic, const char *payload);
    void subscribe(const char *topic);
    void connect();
    void setupWifi(const char *ssid, const char *password, int maxAttempts, int attemptDelay);
    bool messageReceived();
    String getMessageTopic();
    String getMessagePayload();

protected:
    ConfigManager *configManager;
};

#endif
