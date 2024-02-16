#ifndef DEVICE_CONFIG_H
#define DEVICE_CONFIG_H

#include <Arduino.h>
#include <ArduinoJson.h>

enum ConfigError
{
    CONFIG_OK,
    FILE_SYSTEM_ERROR,
    FILE_OPEN_ERROR,
    FILE_EMPTY_ERROR,
    DESERIALIZATION_ERROR,
    FORMATTING_ERROR,
    INITIALIZATION_AFTER_FORMAT_ERROR,
    UNKNOWN_ERROR
};

struct WifiConfig
{
    char ssid[64];
    char password[64];
    int attemptDelay;
    int maxAttempts;
};

struct MqttConfig
{
    char host[64];
    int port;
    char username[64];
    char password[64];
};

struct DeviceConfig
{
    WifiConfig wifi;
    MqttConfig mqtt;
    char deviceId[64];
    uint32_t chipId;
    char macAddress[64];
    uint8_t rangingId[4];
};

class ConfigManager
{
private:
    static ConfigManager *instance;
    void copyString(const char *source, char *dest, size_t size);
    void getMacAddress(char *macAddress);
    void getChipId(uint32_t &chipId);
    ConfigManager();

public:
    static void generateId(char *buffer, int length);
    static void chipIDToAddress(uint8_t *buffer, uint32_t id);
    static ConfigManager *getInstance();

    static DeviceConfig deviceConfig;
    static ConfigError loadConfig();
    char *hidePartialPassword(const char *password);
};

#endif