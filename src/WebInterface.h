#ifndef WEBINTERFACE_H
#define WEBINTERFACE_H

#include <Arduino.h>
// #include <ArduinoJson.h>
#include "main.h"
// #include "TskMQTT.h"

#define WIFIMQTT_FILE "/wifi_mqtt.json"
#define CONFIG_FILE "/config.json"
#define MODBUS_FILE "/modbus.json"
#define MACLIST_FILE "/maclist.json"
#define DATA_MAPPING_FILE "/DataMapping.json"

extern bool mqttEnable;
extern String wifiMode;

class WebinterFace{
    public:
        void setupWebConfig();
};



#endif//WEBINTERFACE_H