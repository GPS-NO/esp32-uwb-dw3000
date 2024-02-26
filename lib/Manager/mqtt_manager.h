#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <vector>

#include "config_manager.h"
#include "boarddefines.h"

struct MqttSubscription {
  const char *topic;
  std::function<void(const char *payload)> callback;
};

class MqttManager {
private:
  static MqttManager *instance;
  WiFiClient wifiClient;
  PubSubClient mqttClient;
  std::vector<MqttSubscription>
    subscriptions;

  static void processMessage(const char *topic, byte *payload, unsigned int length);

  MqttManager();

public:
  static MqttManager *getInstance();

  void publish(const char *topic, const char *payload);
  void subscribe(const char *topic, std::function<void(const char *payload)> callback);
  void unsubscribe(const char *topic);
  void connect();
  void setupWifi(const char *ssid, const char *password, int maxAttempts, int attemptDelay);
  void registerDevice();
  bool messageReceived();
  void loop();
  String getMessageTopic();
  String getMessagePayload();
  char *getBaseTopic();

protected:
  ConfigManager *configManager;
};

#endif
