#ifndef CONFIG_H
#define CONFIG_H

#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>


bool configMode = false; // Chế độ cấu hình
struct NodeConfig {
    char mac[18];
    int node_id;
    uint8_t regBit;
    uint8_t reg8; // Thanh ghi 8-bit
    uint16_t reg16;
    String role; // Role: Node/Bridge/Broker
    char ssid[32];
    char password[32];
};

extern NodeConfig config;

void printConfig();
void saveConfig();
void loadConfig();

#endif // CONFIG_H
