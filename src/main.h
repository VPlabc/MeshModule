#define USE_LITTLEFS // Ensure LittleFS is used
// #define ESP32_RISCV

// #define USE_SERIAL2 // Ensure Serial2 is used
#define USE_SERIAL1 // Ensure Serial1 is used
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

#include "esp_wifi.h"


#define Module_RS485
// #define Module_10O4I

#define CONFIG_FILE "/config.json"
#define MACLIST_FILE "/maclist.json"
#ifdef Module_RS485
  #define BUZZ        -1
  #define SETUP_BUTTON  26 // GPIO 0
  #define LED_STT  25 // GPIO 2
#endif// Module RS485
#ifdef Module_10O4I
// #include "Adafruit_MCP23008.h"

// #define I2C_SDA1 32   // khai bao chan I2C
// #define I2C_SCL1 33


#define Y8  4
#define Y9  5

#define ledPin1    Y9
// #define ledPinStt  -1
// #define ledPinOut  -1

#define BUZZ        12
#define SETUP_BUTTON   13
#define BTN_IO0     0
#define LED_STT  Y8 

// #define DA0         39
// #define DA1         36
///////// ADC //////////////////
#define ADCPin1 36
#define ADCPin2 39

#define InPut0  BTN_IO0
#define InPut1  34
#define InPut2  35
#define InPut3  15
#define InPut4  2



#define ETH_ADDR 1
#define ETH_POWER_PIN -1 // Do not use it, it can cause conflict during the software reset.
#define ETH_POWER_PIN_ALTERNATIVE 14
#define ETH_MDC_PIN 23
#define ETH_MDIO_PIN 18
#define ETH_TYPE ETH_PHY_LAN8720
#define ETH_CLK_MODE ETH_CLOCK_GPIO0_IN



#endif// Module 10O4I
// #define M0_PIN  5
// #define M1_PIN  27 

// #define M0_PIN_1  2 
// #define M1_PIN_1  15


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
#define Se_dRX_PIN 2//4
#define Se_TX_PIN 15//5
#define Se_BAUD_RATE 115200
