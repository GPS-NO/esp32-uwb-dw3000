#include "mqtt_manager.h"
#include <ArduinoJson.h>

MqttManager *MqttManager::instance = nullptr;

MqttManager::MqttManager()
    : mqttClient(wifiClient)
{
    configManager = ConfigManager::getInstance();
}

MqttManager *MqttManager::getInstance()
{
    if (instance == NULL)
    {
        instance = new MqttManager();
    }

    return instance;
}

void MqttManager::setupWifi(const char *ssid, const char *password, int maxAttempts, int attemptDelay)
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    unsigned long startMillis = millis();
    int attempts = 0;

    while (WiFi.status() != WL_CONNECTED && millis() - startMillis < maxAttempts * attemptDelay)
    {
        delay(attemptDelay);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("");
        Serial.println("Successfully connected to the WiFi network with the following local IP: ");
        Serial.print(WiFi.localIP());
        Serial.println("");
    }
    else
    {
        Serial.println("Error connecting to the WiFi network!");
        Serial.println(WiFi.status());
    }
}

void MqttManager::publish(const char *topic, const char *payload)
{

    Serial.println("publishing..");
    mqttClient.publish(topic, payload);
}

void MqttManager::subscribe(const char *topic)
{
    if (!mqttClient.connected())
    {
        // connect();
    }

    mqttClient.subscribe(topic);
}

bool MqttManager::messageReceived()
{
    // mqttClient.loop();
    return true;
}

String MqttManager::getMessageTopic()
{
    // return mqttClient.topic();
    return "";
}

String MqttManager::getMessagePayload()
{
    // return mqttClient.payload();
    // return mqttClient.payloadString();
    return "";
}

void MqttManager::connect()
{
    if (WiFi.status() != WL_CONNECTED)
    {
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
    mqttClient.connect("ESP32Client", username, password);
    mqttClient.publish("test", "Hello World!");
    this->registerDevice();
}

void MqttManager::registerDevice()
{
    mqttClient.publish("devices/wifi/ssid", configManager->deviceConfig.wifi.ssid);
    char *wifiPassword = configManager->hidePartialPassword(configManager->deviceConfig.wifi.password);
    mqttClient.publish("devices/wifi/password", wifiPassword);
    mqttClient.publish("devices/wifi/maxAttempts", String(configManager->deviceConfig.wifi.maxAttempts).c_str());
    mqttClient.publish("devices/wifi/attemptDelay", String(configManager->deviceConfig.wifi.attemptDelay).c_str());

    mqttClient.publish("devices/mqtt/host", configManager->deviceConfig.mqtt.host);
    mqttClient.publish("devices/mqtt/port", String(configManager->deviceConfig.mqtt.port).c_str());
    mqttClient.publish("devices/mqtt/username", configManager->deviceConfig.mqtt.username);
    char *mqttPassword = configManager->hidePartialPassword(configManager->deviceConfig.mqtt.password);
    mqttClient.publish("devices/mqtt/password", mqttPassword);
}