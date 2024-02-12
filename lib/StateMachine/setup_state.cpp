#include "setup_state.h"

void SetupState::onEnter()
{
    Serial.println("[*] Enter State: Setup");
    configManager = ConfigManager::getInstance();
    mqttManager = MqttManager::getInstance("", 1883);
}

void SetupState::onUpdate()
{
    if (strlen(configManager->deviceConfig.ssid) == 0 || strlen(configManager->deviceConfig.password) == 0)
    {
        Serial.println("No WiFi credentials found.");
        return;
    }

    mqttManager->connect();

    // Existiert eine Verbindung?
    // + Verbinde mit MQTT

    Serial.println("================================");
    Serial.println("1: Wechsel in State: Error");
    Serial.println("================================");

    while (1)
    {
        if (Serial.available() > 0)
        {
            int serialInput = Serial.parseInt();

            switch (serialInput)
            {
            case 1:
                Serial.println("Dieser State existiert noch nicht!");
                break;
            default:
                Serial.println("Ungültige Kombination.");
                break;
            }
        }

        delay(100);
    }
}

void SetupState::onExit()
{
    //
}

void SetupState::generateId(char *buffer, int length)
{
    const char characters[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    for (size_t i = 0; i < length - 1; ++i)
    {
        buffer[i] = characters[random(sizeof(characters) - 1)];
    }

    buffer[length - 1] = '\0';
}
