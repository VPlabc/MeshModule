#include "config.h"
#include "utils.h"
#include <WiFi.h>

#define SETTINGS_FILE "/settings.json"
NodeConfig config;

void printConfig() {
    Serial.println("===== Cấu hình hiện tại =====");
    Serial.print("MAC: "); Serial.println(config.mac);
    Serial.print("Node ID: "); Serial.println(config.node_id);
    Serial.print("regBit: "); Serial.println(config.regBit, BIN);
    Serial.print("reg8: "); Serial.println(config.reg8);
    Serial.print("reg16: "); Serial.println(config.reg16);
    Serial.print("Chế độ: "); Serial.println(configMode ? "CẤU HÌNH" : "CHẠY");
    Serial.print("Role: "); Serial.println(config.role);
    Serial.print("SSID: "); Serial.println(config.ssid);
    Serial.print("Password: "); Serial.println(config.password);
    Serial.print("ESP32 MAC: "); Serial.println(WiFi.macAddress());
    Serial.println("=============================");
}

void saveConfig() {
    StaticJsonDocument<256> doc;
    doc["mac"] = config.mac;
    doc["node_id"] = config.node_id;
    doc["regBit"] = config.regBit;
    doc["reg8"] = config.reg8;
    doc["reg16"] = config.reg16;
    doc["configMode"] = configMode;
    doc["role"] = config.role;
    doc["ssid"] = config.ssid;
    doc["password"] = config.password;
  
    File file = SPIFFS.open(SETTINGS_FILE, FILE_WRITE);
    if (!file) {
        Serial.println("Lỗi ghi file cấu hình!");
        return;
    }
    serializeJson(doc, file);
    file.close();
    Serial.println("Lưu cấu hình thành công!");
  
    // Update peerMacAddress
    macStrToBytes(config.mac, peerMacAddress);
}

void loadConfig() {
    if (!SPIFFS.exists(SETTINGS_FILE)) {
        Serial.println("Không tìm thấy file cấu hình! Tạo file cấu hình mặc định.");
        
        // Set default configuration
        strlcpy(config.mac, "24:0A:C4:12:34:56", sizeof(config.mac));
        config.node_id = 1;
        config.regBit = 0b10101010; // Example default value
        config.reg8 = 255; // Example default value
        config.reg16 = 65535; // Example default value
        configMode = false; // Default config mode
        config.role = "Node"; // Default role
        strlcpy(config.ssid, "your-ssid", sizeof(config.ssid));
        strlcpy(config.password, "your-password", sizeof(config.password));
  
        // Save default configuration to file
        saveConfig();
        return;
    }
  
    File file = SPIFFS.open(SETTINGS_FILE, FILE_READ);
    if (!file) {
        Serial.println("Lỗi mở file cấu hình!");
        return;
    }
  
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, file);
    if (error) {
        Serial.print("Lỗi đọc JSON: ");
        Serial.println(error.c_str());
        file.close();
        return;
    }
  
    strlcpy(config.mac, doc["mac"], sizeof(config.mac));
    config.node_id = doc["node_id"];
    config.regBit = doc["regBit"];
    config.reg8 = doc["reg8"];
    config.reg16 = doc["reg16"];
    configMode = doc["configMode"];
    config.role = doc["role"].as<String>();
    strlcpy(config.ssid, doc["ssid"], sizeof(config.ssid));
    strlcpy(config.password, doc["password"], sizeof(config.password));
  
    file.close();
    Serial.println("Đọc cấu hình thành công!");
  
    // Update peerMacAddress
    macStrToBytes(config.mac, peerMacAddress);
}
