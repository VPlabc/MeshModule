#include <Arduino.h>
///////////////////////////// Print /////////////////////////////////////////////
void printConfig(struct Config &MeshConfig) {
    // if (!MeshConfig.debug) return; // Skip printing if debug is disabled
    Serial.println("===== Current Config =====");
    Serial.print("Board Model: ");
    switch (MeshConfig.boardModel) {
        case 0:
            Serial.println("Board Custom");
            break;
        case 1:
            Serial.println("Board ModRTUMesh");
            break;
        case 2:
            Serial.println("Board 410WER");
            break;
        case 3:
            Serial.println("Board LklineGw");
            break;
        case 4:
            Serial.println("Board LklineNode");
            break;
        case 5:
            Serial.println("Board 0404WER");
            break;
        case 6:
            Serial.println("Board S3SDCWER");
            break;
        default:
            Serial.println("Board Custom");
            break;
    }
    Serial.print("Mesh: ");
    Serial.println(MeshConfig.MeshEnable ? "✅ Enabled" : "❌ Disabled");
    Serial.print("Debug: ");
    Serial.println(MeshConfig.debug ? "✅ Enabled" : "❌ Disabled");
    Serial.print("LoRa: ");
    Serial.println(MeshConfig.LoRaEnable ? "✅ Enabled" : "❌ Disabled");
    Serial.print("Buzz: ");
    Serial.println(MeshConfig.BuzzEnable ? "✅ Enabled" : "❌ Disabled");
    Serial.print("Broker Address: ");
    Serial.println(convertBrokerAddressToString(MeshConfig.BrokerAddress));
    Serial.print("WiFi Channel: ");
    Serial.println(MeshConfig.wifiChannel);
    Serial.print("ID: ");
    Serial.println(MeshConfig.id);
    Serial.print("Net ID: ");
    Serial.println(MeshConfig.netId);
    Serial.print("Role: ");
    Serial.println(MeshConfig.role); // Print role
    Serial.print("Data Version: ");
    Serial.println(MeshConfig.dataVersion); // Print data version
    Serial.println("=========================");
    Serial.println("MAC Slaves:");
    for (JsonVariant value : MeshConfig.macSlaves) {
        Serial.println(value.as<const char*>());
    }
    Serial.println("=========================");
}