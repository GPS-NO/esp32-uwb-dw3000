#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <LinkedList.h>

#include "config_manager.h"
#include "boarddefines.h"

typedef std::function<void(const char *topic, const char *payload)> MQTTCallback;

class MqttSubscription {
public:
  char topic[128];
  MQTTCallback callback;
};

class MqttManager {
private:
  static MqttManager *instance;
  WiFiClient wifiClient;
  PubSubClient mqttClient;

  static void processMessage(const char *topic, byte *payload, unsigned int length);

  MqttManager();
  ~MqttManager();

public:
  static MqttManager *getInstance();

  void publish(const char *topic, const char *payload);
  void subscribe(const char *topic, MQTTCallback callback);
  void unsubscribe(const char *topic);
  void unsubscribeAll();
  void connect();
  void setupWifi(const char *ssid, const char *password, int maxAttempts, int attemptDelay);
  void registerDevice();
  void updateStationStatus(const char *status);
  bool messageReceived();
  bool isConnected();
  bool isWifiConnected();
  bool isConfigAvailable();
  void loop();
  static void destroy();
  void sendHeartbeat();
  String getMessageTopic();
  String getMessagePayload();
  String getBaseTopic();

  static bool compareMqttTopics(const char *subscribedTopic, const char *receivedTopic);

protected:
  ConfigManager *configManager;
};

#endif
