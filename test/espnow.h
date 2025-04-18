#ifndef ESPNOW_H
#define ESPNOW_H

#include <esp_now.h>

typedef struct __attribute__((packed)) DatasPacket {
    uint8_t id;
    uint8_t RunStop;
    uint16_t Plan;
    uint16_t Result;
    uint16_t Time;
    uint8_t RunMode;
} DatasPacket;

typedef struct __attribute__((packed)) SettingPacket {
    uint8_t RunStop;
    uint16_t Plan;
    uint16_t PlanSet;
    uint16_t Result;
    uint16_t ResultSet;
    uint16_t Time;
    uint16_t PCShour;
    uint8_t RunMode;
} SettingPacket;

// Địa chỉ MAC của thiết bị ESP32 đích (cần thay đổi cho phù hợp)
uint8_t peerMacAddress[] = {0x24, 0x0A, 0xC4, 0x12, 0x34, 0x56};
uint8_t peerMacAddressSent[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

extern uint8_t peerMacAddress;
extern uint8_t peerMacAddressSent;
extern esp_now_peer_info_t peerInfo;
extern esp_now_peer_info_t peerInfoSent;
extern DatasPacket data;
extern SettingPacket setting;

void sendDataPacket(const DatasPacket& data);
void sendSettingPacket(const SettingPacket& setting);
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);

#endif // ESPNOW_H
