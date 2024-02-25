#include "mqtt_manager.h"

#include <ArduinoJson.h>

MqttManager *MqttManager::instance = nullptr;

MqttManager::MqttManager()
  : mqttClient(wifiClient) {
  configManager = ConfigManager::getInstance();
}

MqttManager *MqttManager::getInstance() {
  if (instance == NULL) {
    instance = new MqttManager();
  }

  return instance;
}

void MqttManager::setupWifi(const char *ssid, const char *password, int maxAttempts, int attemptDelay) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  unsigned long startMillis = millis();
  int attempts = 0;

  while (WiFi.status() != WL_CONNECTED && millis() - startMillis < maxAttempts * attemptDelay) {
    delay(attemptDelay);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("Successfully connected to the WiFi network with the following local IP: ");
    Serial.print(WiFi.localIP());
    Serial.println("");
  } else {
    Serial.println("Error connecting to the WiFi network!");
    Serial.println(WiFi.status());
  }
}

void MqttManager::publish(const char *topic, const char *payload) {
  Serial.println("publishing..");
  mqttClient.publish(topic, payload);
}

void MqttManager::subscribe(const char *topic, std::function<void(const char *payload)> callback) {
  if (!mqttClient.connected()) {
    // connect();
  }

  mqttClient.subscribe(topic);
  subscriptions.push_back({ topic, callback });
}

void MqttManager::unsubscribe(const char *topic) {
  mqttClient.unsubscribe(topic);

  for (auto it = subscriptions.begin(); it != subscriptions.end(); ++it) {
    if (strcmp(it->topic, topic) == 0) {
      subscriptions.erase(it);
      break;
    }
  }
}

void MqttManager::processMessage(const char *topic, byte *payload, unsigned int length) {
  payload[length] = '\0';

  const char *payloadStr = reinterpret_cast<const char *>(payload);
  Serial.print("Received new message in topic: ");
  Serial.print(topic);
  Serial.print(" with payload: ");
  Serial.println(payloadStr);

  for (const MqttSubscription &sub : MqttManager::getInstance()->subscriptions) {
    if (strcmp(sub.topic, topic) == 0) {
      sub.callback(payloadStr);
      break;
    }
  }
}

bool MqttManager::messageReceived() {
  // mqttClient.loop();
  return true;
}

String MqttManager::getMessageTopic() {
  // return mqttClient.topic();
  return "";
}

String MqttManager::getMessagePayload() {
  // return mqttClient.payload();
  // return mqttClient.payloadString();
  return "";
}

void MqttManager::connect() {
  if (WiFi.status() != WL_CONNECTED) {
    char *ssid = configManager->deviceConfig.wifi.ssid;
    char *password = configManager->deviceConfig.wifi.password;
    int maxAttempts = configManager->deviceConfig.wifi.maxAttempts;
    int attemptDelay = configManager->deviceConfig.wifi.attemptDelay;

    this->setupWifi(ssid, password, maxAttempts, attemptDelay);
  }

  char *host = configManager->deviceConfig.mqtt.host;
  int port = configManager->deviceConfig.mqtt.port;
  char *username = configManager->deviceConfig.mqtt.username;
  char *password = configManager->deviceConfig.mqtt.password;

  mqttClient.setServer(host, port);
  mqttClient.connect(configManager->deviceConfig.deviceId, username, password);
  mqttClient.setCallback(MqttManager::processMessage);

  char topicBuffer[64];
  sprintf(topicBuffer, "devices/%s/heartbeat", configManager->deviceConfig.deviceId);
  mqttClient.publish(topicBuffer, String(millis()).c_str());

  this->registerDevice();
}

void MqttManager::registerDevice() {
  char topicBuffer[64];

  sprintf(topicBuffer, "devices/%s/wifi/ssid", configManager->deviceConfig.deviceId);
  mqttClient.publish(topicBuffer, configManager->deviceConfig.wifi.ssid);

  sprintf(topicBuffer, "devices/%s/wifi/password", configManager->deviceConfig.deviceId);
  char *wifiPassword = configManager->hidePartialPassword(configManager->deviceConfig.wifi.password);
  mqttClient.publish(topicBuffer, wifiPassword);

  sprintf(topicBuffer, "devices/%s/wifi/maxAttempts", configManager->deviceConfig.deviceId);
  mqttClient.publish(topicBuffer, String(configManager->deviceConfig.wifi.maxAttempts).c_str());

  sprintf(topicBuffer, "devices/%s/wifi/attemptDelay", configManager->deviceConfig.deviceId);
  mqttClient.publish(topicBuffer, String(configManager->deviceConfig.wifi.attemptDelay).c_str());

  // ------------------------------------------------------------------------------------------

  sprintf(topicBuffer, "devices/%s/mqtt/host", configManager->deviceConfig.deviceId);
  mqttClient.publish(topicBuffer, configManager->deviceConfig.mqtt.host);

  sprintf(topicBuffer, "devices/%s/mqtt/port", configManager->deviceConfig.deviceId);
  mqttClient.publish(topicBuffer, String(configManager->deviceConfig.mqtt.port).c_str());

  sprintf(topicBuffer, "devices/%s/mqtt/username", configManager->deviceConfig.deviceId);
  mqttClient.publish(topicBuffer, configManager->deviceConfig.mqtt.username);

  sprintf(topicBuffer, "devices/%s/mqtt/password", configManager->deviceConfig.deviceId);
  char *mqttPassword = configManager->hidePartialPassword(configManager->deviceConfig.mqtt.password);
  mqttClient.publish(topicBuffer, mqttPassword);

  // ------------------------------------------------------------------------------------------

  sprintf(topicBuffer, "devices/%s/device/id", configManager->deviceConfig.deviceId);
  mqttClient.publish(topicBuffer, configManager->deviceConfig.deviceId);

  sprintf(topicBuffer, "devices/%s/device/chip", configManager->deviceConfig.deviceId);
  mqttClient.publish(topicBuffer, String(configManager->deviceConfig.chipId).c_str());

  sprintf(topicBuffer, "devices/%s/device/address", configManager->deviceConfig.deviceId);
  mqttClient.publish(topicBuffer, String((const char *)configManager->deviceConfig.rangingId).c_str());

  sprintf(topicBuffer, "devices/%s/device/version", configManager->deviceConfig.deviceId);
  mqttClient.publish(topicBuffer, VERSION_STRING);

  sprintf(topicBuffer, "devices/%s/device/commit", configManager->deviceConfig.deviceId);
  mqttClient.publish(topicBuffer, GIT_COMMIT);
}

void MqttManager::loop() {
  mqttClient.loop();
}