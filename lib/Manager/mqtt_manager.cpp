#include "mqtt_manager.h"

MqttManager *MqttManager::instance = nullptr;

MqttManager::MqttManager(const char *server, int port)
    : mqttServer(server), mqttPort(port), mqttClient(wifiClient)
{
    configManager = ConfigManager::getInstance();
}

MqttManager *MqttManager::getInstance(const char *server, int port)
{
    if (instance == NULL)
    {
        instance = new MqttManager(server, port);
    }

    return instance;
}

void MqttManager::setupWifi(const char *ssid, const char *password, int maxAttempts, int attemptDelay)
{
    Serial.println(ssid);
    Serial.println(password);
    Serial.println(maxAttempts);
    Serial.println(attemptDelay);
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
    if (!mqttClient.connected())
    {
        // connect();
    }

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
        char *ssid = configManager->deviceConfig.ssid;
        char *password = configManager->deviceConfig.password;
        int maxAttempts = configManager->deviceConfig.maxAttempts;
        int attemptDelay = configManager->deviceConfig.attemptDelay;

        this->setupWifi(ssid, password, maxAttempts, attemptDelay);
    }
}