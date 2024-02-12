#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

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