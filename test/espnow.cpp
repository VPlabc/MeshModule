#include "espnow.h"
#include "config.h"

DatasPacket data;
SettingPacket setting;

void sendDataPacket(const DatasPacket& data) {
    esp_err_t result = esp_now_send(peerMacAddress, (uint8_t*)&data, sizeof(DatasPacket));
    if (result == ESP_OK) {
        Serial.println("DataPacket sent successfully!");
    } else {
        Serial.println("Failed to send DataPacket!");
    }
}

void sendSettingPacket(const SettingPacket& setting) {
    esp_err_t result = esp_now_send(peerMacAddress, (uint8_t*)&setting, sizeof(SettingPacket));
    if (result == ESP_OK) {
        Serial.println("SettingPacket sent successfully!");
    } else {
        Serial.println("Failed to send SettingPacket!");
    }
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("Last Packet Send Status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
    if (len == sizeof(DatasPacket)) {
        DatasPacket data;
        memcpy(&data, incomingData, sizeof(DatasPacket));
        Serial.print("Received DataPacket from MAC: ");
        for (int i = 0; i < 6; i++) {
            Serial.print(mac[i], HEX);
            if (i < 5) Serial.print(":");
        }
        Serial.println();
        Serial.print("ID: ");
        Serial.print(data.id);
        Serial.print(", RunStop: ");
        Serial.print(data.RunStop);
        Serial.print(", Plan: ");
        Serial.print(data.Plan);
        Serial.print(", Result: ");
        Serial.print(data.Result);
        Serial.print(", Time: ");
        Serial.print(data.Time);
        Serial.print(", RunMode: ");
        Serial.println(data.RunMode);

        // Forward data if role is Bridge
        if (config.role == "Bridge") {
            sendDataPacket(data);
        }
        
    } else if (len == sizeof(SettingPacket)) {
        memcpy(&setting, incomingData, sizeof(SettingPacket));
        Serial.print("Received SettingPacket from MAC: ");
        for (int i = 0; i < 6; i++) {
            Serial.print(mac[i], HEX);
            if (i < 5) Serial.print(":");
        }
        Serial.println();
        Serial.print("RunStop: ");
        Serial.print(setting.RunStop);
        Serial.print(", Plan: ");
        Serial.print(setting.Plan);
        Serial.print(", PlanSet: ");
        Serial.print(setting.PlanSet);
        Serial.print(", Result: ");
        Serial.print(setting.Result);
        Serial.print(", ResultSet: ");
        Serial.print(setting.ResultSet);
        Serial.print(", Time: ");
        Serial.print(setting.Time);
        Serial.print(", PCShour: ");
        Serial.print(setting.PCShour);
        Serial.print(", RunMode: ");
        Serial.println(setting.RunMode);

        // Forward setting if role is Bridge
        if (config.role == "Bridge") {
            sendSettingPacket(setting);
        }
    } else {
        Serial.println("Received unknown data format!");
    }
}
