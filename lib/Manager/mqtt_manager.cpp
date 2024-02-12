#include "mqtt_manager.h"

MQTTClient* MqttManager::instance = nullptr;

MqttManager::MQTTClient(const char* server, int port) {
    mqttServer = server;
    mqttPort = port;
    pubSubClient = PubSubClient(wifiClient);
}

void MqttManager::getInstance(const char* server, int port) {
    if (instance == NULL) {
        instance = new MqttManager(server, port);
    }

    return instance;
}

void MqttManager::publish(const char* topic, const char* payload) {
    if(!pubSubClient.connected()) {
        //reconnect();
    }

    pubSubClient.publish(topic, payload);
}

void MqttManager::subscribe(const char* topic) {
    if(!pubSubClient.connected()) {
        //reconnect();
    }

    pubSubClient.subscribe(topic);
}

String MqttManager::messageReceived() {
    return pubSubClient.available();
}

String MqttManager::getMessageTopic() {
    return pubSubClient.topic();
}

String MqttManager::getMessagePayload() {
    return pubSubClient.payload();
    //return pubSubClient.payloadString();
}

void MqttManager::reconnect() {
    //
}