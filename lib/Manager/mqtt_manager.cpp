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

MqttManager::~MqttManager() {
  destroy();
}

void MqttManager::destroy() {
  if (instance != nullptr) {
    delete instance;
    instance = nullptr;
  }
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

void MqttManager::subscribe(const char *topic, MQTTCallback callback) {
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

void MqttManager::unsubscribeAll() {
  for (auto it = subscriptions.begin(); it != subscriptions.end(); ++it) {
    mqttClient.unsubscribe(it->topic);
    subscriptions.erase(it);
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
    if (MqttManager::compareMqttTopics(sub.topic, topic) == 0) {
      sub.callback(topic, payloadStr);
      break;
    }
  }
}

bool MqttManager::isConfigAvailable() {
  return true;
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

String MqttManager::getBaseTopic() {
  char topicBuffer[64];
  sprintf(topicBuffer, "devices/%s", configManager->deviceConfig.deviceId);
  return String(topicBuffer);
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

  this->sendHeartbeat();
  this->registerDevice();
}

bool MqttManager::isConnected() {
  return this->mqttClient.connected();
}

bool MqttManager::isWifiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

void MqttManager::registerDevice() {
  char topicBuffer[64];

  sprintf(topicBuffer, "%s/wifi/ssid", this->getBaseTopic().c_str());
  mqttClient.publish(topicBuffer, configManager->deviceConfig.wifi.ssid);

  sprintf(topicBuffer, "%s/wifi/password", this->getBaseTopic().c_str());
  char *wifiPassword = configManager->hidePartialPassword(configManager->deviceConfig.wifi.password);
  mqttClient.publish(topicBuffer, wifiPassword);

  sprintf(topicBuffer, "%s/wifi/maxAttempts", this->getBaseTopic().c_str());
  mqttClient.publish(topicBuffer, String(configManager->deviceConfig.wifi.maxAttempts).c_str());

  sprintf(topicBuffer, "%s/wifi/attemptDelay", this->getBaseTopic().c_str());
  mqttClient.publish(topicBuffer, String(configManager->deviceConfig.wifi.attemptDelay).c_str());

  // ------------------------------------------------------------------------------------------

  sprintf(topicBuffer, "%s/mqtt/host", this->getBaseTopic().c_str());
  mqttClient.publish(topicBuffer, configManager->deviceConfig.mqtt.host);

  sprintf(topicBuffer, "%s/mqtt/port", this->getBaseTopic().c_str());
  mqttClient.publish(topicBuffer, String(configManager->deviceConfig.mqtt.port).c_str());

  sprintf(topicBuffer, "%s/mqtt/username", this->getBaseTopic().c_str());
  mqttClient.publish(topicBuffer, configManager->deviceConfig.mqtt.username);

  sprintf(topicBuffer, "%s/mqtt/password", this->getBaseTopic().c_str());
  char *mqttPassword = configManager->hidePartialPassword(configManager->deviceConfig.mqtt.password);
  mqttClient.publish(topicBuffer, mqttPassword);

  // ------------------------------------------------------------------------------------------

  sprintf(topicBuffer, "%s/device/id", this->getBaseTopic().c_str());
  mqttClient.publish(topicBuffer, configManager->deviceConfig.deviceId);

  sprintf(topicBuffer, "%s/device/chip", this->getBaseTopic().c_str());
  mqttClient.publish(topicBuffer, String(configManager->deviceConfig.chipId).c_str());

  sprintf(topicBuffer, "%s/device/address", this->getBaseTopic().c_str());
  mqttClient.publish(topicBuffer, String((const char *)configManager->deviceConfig.rangingId).c_str());

  sprintf(topicBuffer, "%s/device/version", this->getBaseTopic().c_str());
  mqttClient.publish(topicBuffer, VERSION_STRING);

  sprintf(topicBuffer, "%s/device/commit", this->getBaseTopic().c_str());
  mqttClient.publish(topicBuffer, GIT_COMMIT);
}

void MqttManager::sendHeartbeat() {
  char topicBuffer[64];
  sprintf(topicBuffer, "%s/heartbeat", this->getBaseTopic().c_str());
  mqttClient.publish(topicBuffer, String(millis()).c_str());
}

void MqttManager::loop() {
  mqttClient.loop();
}

bool MqttManager::compareMqttTopics(const char *topic1, const char *topic2) {
  while (*topic1 && *topic2) {
    if (*topic1 == '+' || *topic2 == '+') {
      // '+' wildcard matches any single level
      while (*topic1 && *topic1 != '/') topic1++;
      while (*topic2 && *topic2 != '/') topic2++;
    } else if (*topic1 == '#' || *topic2 == '#') {
      // '#' wildcard matches any remaining levels
      return true;
    } else if (*topic1 != *topic2) {
      return false;
    }

    // Move to the next character in both topics
    topic1++;
    topic2++;

    // Check for the end of a level
    if (*topic1 == '/') topic1++;
    if (*topic2 == '/') topic2++;
  }

  // Check if both topics reach the end at the same time
  return *topic1 == '\0' && *topic2 == '\0';
}