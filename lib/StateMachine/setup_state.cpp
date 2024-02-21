#include "setup_state.h"

void SetupState::onEnter()
{
    Serial.println("[*] Enter State: Setup");
    configManager = ConfigManager::getInstance();
    mqttManager = MqttManager::getInstance();
    ranging = RangingSystem::getInstance();

    if (strlen(configManager->deviceConfig.wifi.ssid) == 0 || strlen(configManager->deviceConfig.wifi.password) == 0)
    {
        Serial.println("No WiFi credentials found.");
        return;
    }

    mqttManager->connect();

    int8_t rangingInitResult = ranging->init(configManager->deviceConfig.rangingId, PIN_IRQ, PIN_RST, PIN_SS);
    if (rangingInitResult == 0)
        Serial.println("Ranging system initialized successfully.");
    else
        Serial.println("Failed to initialize ranging system.");

}

void SetupState::onUpdate()
{
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
            case 2:
                mqttManager->publish("Test", "Hello World!");
                mqttManager->registerDevice();
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