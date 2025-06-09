#ifndef TSKMQTT_H
#define TSKMQTT_H

#include <Arduino.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
// #include <MQTT.h>
#include <AsyncMqttClient.h>
#include <LittleFS.h>

#define WIFIMQTT_FILE "/wifi_mqtt.json"
//{"mqtthost":"test.mosquitto.org","mqttport":1883,"mqttuser":"usermqtt","mqttpass":"mqttpass","conId":"b8e54d33-b34a-45ab-b76f-62c8a9abc6c4"}
/* 
MQTT parameters
  mqtthost : MQTT broker address
  mqttport : MQTT broker port
  mqttuser : MQTT user name
  mqttpass : MQTT password
  conId : MQTT device ID
*/

// TaskHandle_t TaskMQTTHandle;

extern String mqttHost;
extern int mqttPort;
extern String mqttUser;
extern String mqttPass;
extern bool mqttEnable;
extern String wifiMode;
extern String ssid;
extern String pass;
extern String conId;
extern String mqttTopicStat;
extern String mqttTopicPub;
extern String mqttTopicSub;
extern int mqttKeepAlive;
extern bool mqttCleanSession;
extern int mqttQos;
extern bool mqttRetain;
extern String mqttLwtTopic;
extern String mqttLwtMessage;
extern int mqttLwtQos;
extern bool mqttLwtRetain;
extern bool mqttLwtEnabled;
extern int timezone;
extern bool mqttIsConnected;

class WifiMqttConfig {
    
    public:
        void setup();
        void loop();
        // void setupWiFi();
        void MQTTPush(String Topic,String Payload);
        void saveJsonToWifiMqttFile(char &jsonString,bool debug, fs::FS &FileSystem);
        String loadWifiMqttConfig(bool debug, fs::FS &FileSystem);
        // String mqttHost = "broker.hivemq.com";
        // int mqttPort = 1883;
        // String mqttUser = "username";
        // String mqttPass = "password";
        // bool mqttEnable = true;
        
        // String wifiMode = "STA";
        // String ssid = "yourSSID";
        // String pass = "yourPassword";
        // String conId = "b8e54d33-b34a-45ab-b76f-62c8a9abc6c4";
        // String mqttTopic = "test/topic";
        // String mqttTopicSub = "test/topic/sub";

};
// WifiMqttConfig   WifiMQTTConfig;

#endif// TSKMQTT_H