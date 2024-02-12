#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>

class MqttManager {
    private:
        static MQTTClient* instance;
        const char* mqttServer;
        const int mqttPort;
        WiFiClient wifiClient;
        PubSubClient pubSubClient;

        MQTTClient(const char* server, int port);

    public:
        static MQTTClient* getInstance(const char* server, int port);

        void publish(const char* topic, const char* payload);
        void subscribe(const char* topic);
        void reconnect();
        bool messageReceived();
        String getMessageTopic();
        String getMessagePayload();
};

#endif