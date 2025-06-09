#ifndef Main_H
#define Main_H

#define USE_VEHICLE
#define TCP_ETH
#define USE_SD
#define USE_LITTLEFS // Ensure LittleFS is used
// #define ESP32_RISCV

#define USE_SERIAL2 // Ensure Serial2 is used
// #define USE_SERIAL1 // Ensure Serial1 is used
#ifdef USE_SERIAL1
#define USE_Modbus
#define USE_MQTT
#endif// USE_SERIAL1

#ifdef TCP_ETH
// #define USE_LAN8720
#define USE_W5500
#endif// TCP_ETH

#ifdef USE_SERIAL2
// #define USE_Modbus
#define USE_MQTT
#endif// USE_SERIAL2

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

#include "esp_wifi.h"


#define Module_RS485
// #define Module_10O4I

#define CONFIG_FILE "/config.json"
#define MACLIST_FILE "/maclist.json"

#include <Wire.h>
#include <SPI.h>
extern int8_t BUZZ;
extern int8_t SETUP_BUTTON;
extern int8_t LED_STT;
extern int8_t I2C_SDA;
extern int8_t I2C_SCL;
extern int8_t Y8;
extern int8_t Y9;
extern int8_t DA0;
extern int8_t DA1;
extern int8_t InPut0;
extern int8_t InPut1;
extern int8_t InPut2;
extern int8_t InPut3;
extern int8_t InPut4;
extern int8_t Ser_1RX;
extern int8_t Ser_1TX;
extern int8_t M0_PIN;
extern int8_t M1_PIN;
extern int8_t CS_PIN;
extern int8_t RST_PIN;
extern int8_t INIT_PIN;
extern int8_t Ser_2RX;
extern int8_t Ser_2TX;


// #ifdef Module_10O4I
// #include "Adafruit_MCP23008.h"
#ifdef USE_LAN8720
#include <ETH.h>
#define ETH_ADDR 1
#define ETH_POWER_PIN -1 // Do not use it, it can cause conflict during the software reset.
#define ETH_TYPE ETH_PHY_LAN8720
#define ETH_CLK_MODE ETH_CLOCK_GPIO0_IN
#endif//USE_LAN8720
#ifdef USE_W5500
#include <Ethernet.h>
#endif//USE_W5500
#ifdef USE_SD
#include <SD.h>
#endif//USE_SD

// #endif// Module 10O4I
#define Se_BAUD_RATE 115200
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
void SupendTask();
void ResumeTask();
#endif//Gateway

#endif//Main_h