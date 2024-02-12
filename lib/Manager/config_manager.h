#ifndef DEVICE_CONFIG_H
#define DEVICE_CONFIG_H

#include <Arduino.h>

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

struct WifiConfig {
    char ssid[32];
    char password[64];
    int attemptDelay;
    int maxAttempts;
};

struct MqttConfig {
    char host[32];
    int port;
    char username[32];
    char password[64];
};

struct DeviceConfig {
    WifiConfig wifi;
    MqttConfig mqtt;
    char deviceId[32];
};
class ConfigManager
{
private:
    static ConfigManager *instance;
    void copyString(const char* source, char* dest, size_t size);

    ConfigManager();

public:
    static ConfigManager *getInstance();

    static DeviceConfig deviceConfig;
    static ConfigError loadConfig();
};

#endif