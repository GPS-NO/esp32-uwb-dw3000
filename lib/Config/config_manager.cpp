#include "config_manager.h"
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

DeviceConfig ConfigManager::deviceConfig;

void ConfigManager::loadConfig() {
    String configFile = "/device.json";

    if (!SPIFFS.begin()) {
        Serial.println("Error initializing the file system!");

        Serial.println("Formatting SPIFFS...");
        if (SPIFFS.format()) {
            Serial.println("SPIFFS successfully formatted!");
        } else {
            Serial.println("Error formatting SPIFFS!");
            return;
        }

        if (!SPIFFS.begin()) {
            Serial.println("Error re-initializing SPIFFS after formatting!");
            return;
        }

        Serial.println("SPIFFS successfully initialized after formatting.");
    }

    File file = SPIFFS.open(configFile, "r");
    if (!file) {
        Serial.println("Error opening the configuration file!");
        SPIFFS.end();
        return;
    }

    size_t fileSize = file.size();
    if (fileSize == 0) {
        Serial.println("The configuration file is empty!");
        file.close();
        SPIFFS.end();
        return;
    }
    
    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, file);

    file.close();
    SPIFFS.end();

    if (error) {
        Serial.print("Error deserializing the configuration file: ");
        Serial.println(error.c_str());
        return;
    }

    deviceConfig.ssid = doc["ssid"];
    deviceConfig.password = doc["password"];
}
