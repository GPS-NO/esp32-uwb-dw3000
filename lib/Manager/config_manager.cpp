#include "config_manager.h"
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <esp_system.h>

ConfigManager *ConfigManager::instance = nullptr;
DeviceConfig ConfigManager::deviceConfig;

ConfigManager::ConfigManager() {}

ConfigManager *ConfigManager::getInstance()
{
    if (instance == NULL)
    {
        instance = new ConfigManager();
        instance->loadConfig();
    }

    return instance;
}

ConfigError ConfigManager::loadConfig()
{
    String configFile = "/device.json";

    if (!SPIFFS.begin(true))
    {
        Serial.println("Error initializing the file system!");
        return FORMATTING_ERROR;
    }

    File file = SPIFFS.open(configFile, "r");
    if (!file)
    {
        Serial.println("Error opening the configuration file!");
        SPIFFS.end();
        return FILE_OPEN_ERROR;
    }

    size_t fileSize = file.size();
    if (fileSize == 0)
    {
        Serial.println("The configuration file is empty!");
        file.close();
        SPIFFS.end();
        return FILE_EMPTY_ERROR;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);

    file.close();

    if (error)
    {
        Serial.print("Error deserializing the configuration file: ");
        Serial.println(error.c_str());
        return DESERIALIZATION_ERROR;
    }

    instance->copyString(doc["wifi"]["ssid"].as<const char *>(), deviceConfig.wifi.ssid, sizeof(deviceConfig.wifi.ssid));
    instance->copyString(doc["wifi"]["password"].as<const char *>(), deviceConfig.wifi.password, sizeof(deviceConfig.wifi.password));
    instance->copyString(doc["device_id"].as<const char *>(), deviceConfig.deviceId, sizeof(deviceConfig.deviceId));

    deviceConfig.wifi.attemptDelay = doc["wifi"]["attemptDelay"].as<int>();
    deviceConfig.wifi.maxAttempts = doc["wifi"]["maxAttempts"].as<int>();

    instance->copyString(doc["mqtt"]["host"].as<const char *>(), deviceConfig.mqtt.host, sizeof(deviceConfig.mqtt.host));

    deviceConfig.mqtt.port = doc["mqtt"]["port"].as<int>();

    instance->copyString(doc["mqtt"]["username"].as<const char *>(), deviceConfig.mqtt.username, sizeof(deviceConfig.mqtt.username));
    instance->copyString(doc["mqtt"]["password"].as<const char *>(), deviceConfig.mqtt.password, sizeof(deviceConfig.mqtt.password));

    return CONFIG_OK;
}

void ConfigManager::copyString(const char* source, char* dest, size_t size) {
    strncpy(dest, source, size);
    dest[size - 1] = '\0';
}
