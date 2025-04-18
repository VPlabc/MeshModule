#define USE_LITTLEFS // Ensure LittleFS is used
// #define ESP32_RISCV

#define USE_SERIAL2 // Ensure Serial2 is used
// #define USE_SERIAL1 // Ensure Serial1 is used
#ifdef USE_SERIAL1
#define USE_Modbus
#define USE_MQTT
#endif// USE_SERIAL1

#define DEBUG_OUTPUT_SERIAL

#ifdef DEBUG_OUTPUT_SERIAL
#define DEBUG_PIPE SERIAL_PIPE
#define LOG(string) {Serial.print(string);}
#define LOGLN(string) {Serial.println(string);}

#else
#define LOG(string) {}
#define LOGLN(string) {}
#endif 

#include <Arduino.h>
#include <ArduinoJson.h>
#include "Arduino_JSON.h"
#include <FS.h>
#ifdef ESP32
  #include <WiFi.h>
  #include <esp_now.h>
  #include <esp_wifi.h>  // Include this library for esp_wifi_set_channel
#else//ESP8266
  #include <ESP8266WiFi.h>
  #include <espnow.h>
#endif//ESP32

#ifdef USE_LITTLEFS
  #include <LittleFS.h>
  #define FileSystem LittleFS
#else
  #error "SPIFFS is not supported on ESP32-S3. Please use LittleFS."
#endif
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#define CONFIG_FILE "/config.json"
#define MACLIST_FILE "/maclist.json"
#define SETUP_BUTTON  26 // GPIO 0
#define LED_STT  25 // GPIO 2
#define M0_PIN  5
#define M1_PIN  27 

#define M0_PIN_1  2 
#define M1_PIN_1  15


// #define MY_ROLE         ESP_NOW_ROLE_COMBO              // set the role of this device: CONTROLLER, SLAVE, COMBO
// #define RECEIVER_ROLE   ESP_NOW_ROLE_COMBO              // set the role of the receiver
// #define WIFI_CHANNEL    1
#ifndef Gateway
void saveConfig();
void loadConfig();
void processSerialInput();
void printConfig();
void Broker(const String &message);
void printMacList();

void DataForPC();
void SerialInit();
void formatMacAddress(const uint8_t *macAddr, char *buffer, int maxLength);
void sendBindRequest();
void saveMacToMacList(const uint8_t *macAddr, int id);
void receiveDataPacketFromSerial2();
void receiveCallback(const uint8_t *senderMac, const uint8_t *data, int dataLen);
void sentCallback(const uint8_t *macAddr, esp_now_send_status_t status);
#endif//Gateway
#define Se_dRX_PIN 2//4
#define Se_TX_PIN 15//5
#define Se_BAUD_RATE 115200
