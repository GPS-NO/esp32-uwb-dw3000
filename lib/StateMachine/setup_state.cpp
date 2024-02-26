#include "setup_state.h"

void SetupState::onEnter() {
  Serial.println("[*] Enter State: Setup");
  configManager = ConfigManager::getInstance();
  mqttManager = MqttManager::getInstance();
  ranging = RangingSystem::getInstance();

  if (strlen(configManager->deviceConfig.wifi.ssid) == 0 || strlen(configManager->deviceConfig.wifi.password) == 0) {
    Serial.println("No WiFi credentials found.");
    return;
  }

  mqttManager->connect();

  char topicBuffer[128];
  sprintf(topicBuffer, "%s/ranging/+/+", mqttManager->getBaseTopic().c_str());
  mqttManager->subscribe(topicBuffer, [](const char *topic, const char *payload) {
    Serial.println("(SETUP_STATE): " + String(payload));
    String receivedTopic = String(topic);
    size_t baseTopicLength = (MqttManager::getInstance()->getBaseTopic() + "/ranging").length();
    receivedTopic.remove(0, baseTopicLength + 1);

    String otherID = receivedTopic.substring(0, 4);
    String rangingType = receivedTopic.substring(5);

    Serial.println("(SETUP_STATE): " + receivedTopic + " " + rangingType + " " + otherID);
  });

  int8_t rangingInitResult = ranging->init(configManager->deviceConfig.rangingId, PIN_IRQ, PIN_RST, PIN_SS);
  if (rangingInitResult == 0)
    Serial.println("Ranging system initialized successfully.");
  else
    Serial.println("Failed to initialize ranging system.");
}

void SetupState::onUpdate() {
  Serial.println("================================");
  Serial.println("1: Wechsel in State: Error");
  Serial.println("2: Wechsel in State: Register Device");
  Serial.println("2: Wechsel in State: Ranging State");
  Serial.println("================================");

  while (1) {
    if (Serial.available() > 0) {
      int serialInput = Serial.parseInt();

      switch (serialInput) {
        case 1:
          {
            Serial.println("Dieser State existiert noch nicht!");
            break;
          }
        case 2:
          {
            mqttManager->publish("Test", "Hello World!");
            mqttManager->registerDevice();
            break;
          }
        case 3:
          {
            StateMachineState::currentState = StateMachineState::rangingState;
            SetupState::onExit();
            return;
          }
        default:
          Serial.println("Ungültige Kombination.");
          break;
      }
    }

    mqttManager->loop();
    delay(100);
  }
}

void SetupState::onExit() {
  mqttManager->unsubscribeAll();
}