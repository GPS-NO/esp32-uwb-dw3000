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
    SPIFFS.end();

    if (error)
    {
        Serial.print("Error deserializing the configuration file: ");
        Serial.println(error.c_str());
        return DESERIALIZATION_ERROR;
    }

    strncpy(deviceConfig.ssid, doc["ssid"].as<const char *>(), sizeof(deviceConfig.ssid));
    deviceConfig.ssid[sizeof(deviceConfig.ssid) - 1] = '\0';

    strncpy(deviceConfig.password, doc["password"].as<const char *>(), sizeof(deviceConfig.password));
    deviceConfig.password[sizeof(deviceConfig.password) - 1] = '\0';

    strncpy(deviceConfig.deviceId, doc["device_id"].as<const char *>(), sizeof(deviceConfig.deviceId));
    deviceConfig.deviceId[sizeof(deviceConfig.deviceId) - 1] = '\0';

    deviceConfig.attemptDelay = doc["attemptDelay"].as<int>();
    deviceConfig.maxAttempts = doc["maxAttempts"].as<int>();

    return CONFIG_OK;
}