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

struct DeviceConfig
{
    char ssid[32];
    char password[64];
    char deviceId[32];
};

class ConfigManager
{
public:
    static DeviceConfig deviceConfig;
    static ConfigError loadConfig();
};

#endif