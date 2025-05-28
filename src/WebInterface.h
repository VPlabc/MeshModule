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
#define DATA_VIEWER_FILE "/PinMap.json"
#define ETHERNET_FILE "/Ethernet.json"

extern bool mqttEnable;
extern String wifiMode;
extern bool socketConnected;
extern bool WebConnected;


class WebinterFace{
    public:
        void setupWebConfig();
        void SocketLoop();
        void SendMessageToClient(const String& message);
};



#endif//WEBINTERFACE_H