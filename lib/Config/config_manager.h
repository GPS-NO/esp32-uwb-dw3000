#ifndef DEVICE_CONFIG_H
#define DEVICE_CONFIG_H

#include <Arduino.h>

struct DeviceConfig {
    const char* ssid;
    const char* password;
};

class ConfigManager {
    public:
        static DeviceConfig deviceConfig;
        static void loadConfig();
};


#endif