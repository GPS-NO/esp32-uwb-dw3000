#include "config_manager.h"
#include <FS.h>
#include <SPIFFS.h>
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

    // instance->getMacAddress(deviceConfig.macAddress);
    // instance->copyString(doc["macAddress"].as<const char *>(), deviceConfig.macAddress, sizeof(deviceConfig.macAddress));

    // instance->getChipId(deviceConfig.chipId);
    // instance->copyString(doc["chipId"].as<const char *>(), deviceConfig.chipId, sizeof(deviceConfig.chipId));

    return CONFIG_OK;
}

void ConfigManager::copyString(const char *source, char *dest, size_t size)
{
    strncpy(dest, source, size);
    dest[size - 1] = '\0';
}

void ConfigManager::getMacAddress(char *macAddress)
{
    uint8_t mac[6];
    esp_err_t macReadStatus = esp_read_mac(mac, ESP_MAC_WIFI_STA);

    if (macReadStatus == ESP_OK)
    {
        snprintf(macAddress, 18, "%02X:%02X:%02X:%02X:%02X:%02X",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }
    else
    {
        const char *defaultMac = "00:00:00:00:00:00";
        strncpy(macAddress, defaultMac, 18);
    }
}

void ConfigManager::getChipId(char *chipId)
{
    uint32_t chipIdValue = 0;
    for (int i = 0; i < 17; i = i + 8)
    {
        chipIdValue |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }

    if (chipIdValue == 0)
    {
        snprintf(chipId, 9, "%08X", 0x00000000);
    }
    else
    {
        snprintf(chipId, 9, "%08X", chipIdValue);
    }
}

char *ConfigManager::hidePartialPassword(const char *password)
{
    const size_t visibleChars = 3;
    size_t len = strlen(password);
    char *hiddenPassword = new char[len + 1];

    if (len > visibleChars)
    {
        for (size_t i = 0; i < len; ++i)
        {
            if (i < visibleChars)
            {
                hiddenPassword[i] = password[i];
            }
            else
            {
                hiddenPassword[i] = '*';
            }
        }
    }
    else
    {
        strncpy(hiddenPassword, password, len);
    }

    hiddenPassword[len] = '\0';

    return hiddenPassword;
}