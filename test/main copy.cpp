#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h> // Change to LittleFS
#include <esp_now.h>
#include <DNSServer.h>
#include <Update.h>
#define DEBUG_OUTPUT_SERIAL
#include <esp_wifi.h>  // Include this library for esp_wifi_set_channel
#include <soc/soc.h>          // Include this header for RTC_CNTL_BROWN_OUT_REG
#include <soc/rtc_cntl_reg.h> // Include this header for RTC_CNTL_BROWN_OUT_REG


#ifdef DEBUG_OUTPUT_SERIAL
#define DEBUG_PIPE SERIAL_PIPE
#define LOG(string) {Serial.print(string);}
#define LOGLN(string) {Serial.println(string);}

#else
#define LOG(string) {}
#define LOGLN(string) {}
#endif 

#include "esp_wifi.h"
#include <WiFi.h>
uint8_t current_protocol;
esp_interface_t current_esp_interface;
wifi_interface_t current_wifi_interface;
int rssi_display;
StaticJsonDocument<8192> doc; // Increase the size of the JSON document

int check_protocol(bool LooklineDebug)
{
    char error_buf1[100];
  if(LooklineDebug){
    LOGLN();
    LOGLN("Lookline_________________________");
    LOGLN();
     esp_err_t error_code = esp_wifi_get_protocol(current_wifi_interface, &current_protocol);
     esp_err_to_name_r(error_code,error_buf1,100);
     LOG("error code: ");
     LOGLN(error_buf1);
    LOGLN("Code: " + String(current_protocol));
    if ((current_protocol&WIFI_PROTOCOL_11B) == WIFI_PROTOCOL_11B)
      LOGLN("Protocol is WIFI_PROTOCOL_11B");
    if ((current_protocol&WIFI_PROTOCOL_11G) == WIFI_PROTOCOL_11G)
      LOGLN("Protocol is WIFI_PROTOCOL_11G");
    if ((current_protocol&WIFI_PROTOCOL_11N) == WIFI_PROTOCOL_11N)
      LOGLN("Protocol is WIFI_PROTOCOL_11N");
    if ((current_protocol&WIFI_PROTOCOL_LR) == WIFI_PROTOCOL_LR)
      LOGLN("Protocol is WIFI_PROTOCOL_LR");
    LOGLN("___________________________________");
    LOGLN();
    LOGLN();
  }
    return current_protocol;
}

// Địa chỉ MAC của thiết bị ESP32 đích (cần thay đổi cho phù hợp)
uint8_t peerMacAddress[] = {0x24, 0x0A, 0xC4, 0x12, 0x34, 0x56};
uint8_t peerMacAddressSent[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t BroadcastMacAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
esp_now_peer_info_t peerInfo;

#define BUTTON_PIN 0 // Nút nhấn trên GPIO 0
#define SETTINGS_FILE "/settings.json"
#define DNS_PORT 53

bool configMode = true; // Chế độ cấu hình
bool LastConfigMode = true; // Chế độ cấu hình
AsyncWebServer server(80);
DNSServer dnsServer;

// Cấu trúc dữ liệu cấu hình
struct NodeConfig {
    char mac[18];
    int node_id;
    byte netId; // Network ID
    String model; // Model: V14/V15
    uint8_t rfpower; // Công suất phát RF
    uint8_t chan; // Kênh mesh
    uint8_t regBit; // Thanh ghi bit
    uint8_t reg8; // Thanh ghi 8-bit
    uint16_t reg16; // Thanh ghi 16-bit
    String role; // Role: Node/Bridge/Broker
    char ssid[32];
    char password[32];
    JsonArray macAddresses;
} config;
// NodeConfig config; // Biến lưu cấu hình

typedef struct struct_Parameter_message {
    byte networkID;       //1
    byte nodeID;          //1
    int PLAN;             //4
    int RESULT;           //4
    byte state;           //1
    byte Mode;            //1
    byte RSSI;            //1
    byte Com;            //1
    byte WiFi;            //1
    byte Cmd;            //1
    byte type;            //1
    int Nodecounter;
} struct_Parameter_message;

struct_Parameter_message DataLookline;

typedef struct __attribute__((packed)) DatasPacket {
    uint8_t id;
    uint8_t data[200];
} DatasPacket;
DatasPacket data;

typedef struct __attribute__((packed)) SettingPacket {
    uint8_t id;
    uint8_t cmd; // 0: monitor, 1: setting, 2: repeat
    uint8_t data[200];
} SettingPacket;
SettingPacket setting;

char newMacStr[18] = "";
bool newMacReceived = false;

void printConfig();
void saveConfig();
void loadConfig();
void startWebServer();
void startFirmwareUploadServer();
void processSerialInput();
void handleButtonPress();
void checkButton();
void sendDataPacket(const DatasPacket& data);
void sendSettingPacket(const SettingPacket& setting);

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    if(status == ESP_NOW_SEND_SUCCESS){
        LOGLN("Data sent successfully!");
    } else {
        LOGLN("Data sent failed!");
    }
}

// Hàm nhận JSON từ Serial Monitor

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
    if (mac == nullptr || incomingData == nullptr) {
        LOGLN("Received null pointer!");
        return;
    }
    // Convert MAC address to string
    snprintf(newMacStr, sizeof(newMacStr), "%02X:%02X:%02X:%02X:%02X:%02X",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    newMacReceived = true;

    StaticJsonDocument<3500> jsonDoc;
    jsonDoc["mac"] = newMacStr;

    if (len == sizeof(DatasPacket)) {
        memcpy(&data, incomingData, sizeof(DatasPacket));
        jsonDoc["ID"] = data.id;
        JsonArray dataArray = jsonDoc.createNestedArray("data");
        for (int i = 0; i < 200; i++) {
            dataArray.add(data.data[i]);
        }
        // If role is Broker, convert 4 bytes into specific parameters
        if (config.role == "Broker") {
            uint16_t Plan, Result;
            uint8_t state, RSSI, Com, WiFi, type, Cmd;
            Plan = (data.data[0] << 8) | data.data[1];
            Result = (data.data[2] << 8) | data.data[3];
            state = data.data[4];
            RSSI = data.data[5];
            Com = data.data[6];
            WiFi = data.data[7];
            type = data.data[8];
            Cmd = data.data[9];
            
            DataLookline.PLAN = Plan;
            DataLookline.RESULT = Result;
            DataLookline.state = state;
            DataLookline.RSSI = RSSI;
            DataLookline.Com = Com;
            DataLookline.WiFi = WiFi;
            DataLookline.type = type;
            DataLookline.Cmd = Cmd;
            

            jsonDoc["Plan"] = Plan;
            jsonDoc["Result"] = Result;
            jsonDoc["state"] = state;
            jsonDoc["RSSI"] = RSSI;
            jsonDoc["Com"] = Com;
            jsonDoc["WiFi"] = WiFi;
            jsonDoc["type"] = type;
            jsonDoc["Cmd"] = Cmd;

            // Print the parameters to Serial
            LOGLN("Broker Parameters:");
            LOG("Plan: "); LOGLN(Plan);
            LOG("Result: "); LOGLN(Result);
            LOG("state: "); LOGLN(state);
            LOG("RSSI: "); LOGLN(RSSI);
            LOG("Com: "); LOGLN(Com);
            LOG("WiFi: "); LOGLN(WiFi);
            LOG("type: "); LOGLN(type);
            LOG("Cmd: "); LOGLN(Cmd);
        }
        // Forward data if role is Bridge
        if (config.role == "Bridge") {
            sendDataPacket(data);
        }
        
    } else if (len == sizeof(SettingPacket)) {
        memcpy(&setting, incomingData, sizeof(SettingPacket));
        jsonDoc["ID"] = setting.id;
        jsonDoc["CMD"] = setting.cmd;
        JsonArray dataArray = jsonDoc.createNestedArray("data");
        for (int i = 0; i < 200; i++) {
            dataArray.add(setting.data[i]);
        }

        // Forward setting if role is Bridge
        if (config.role == "Bridge") {
            sendSettingPacket(setting);
        }
    }else if (len == sizeof(struct_Parameter_message)) {
        memcpy(&DataLookline, incomingData, sizeof(struct_Parameter_message));
        
        String net_id = "";
        net_id += String((DataLookline.networkID / 1000) % 10);
        net_id += String((DataLookline.networkID / 100) % 10);
        net_id += String((DataLookline.networkID / 10) % 10);
        net_id += String((DataLookline.networkID / 1) % 10);
        
        String node_id = "";
        node_id += String((DataLookline.nodeID / 1000) % 10);
        node_id += String((DataLookline.nodeID / 100) % 10);
        node_id += String((DataLookline.nodeID / 10) % 10);
        node_id += String((DataLookline.nodeID / 1) % 10);
        
        LOGLN("Mesh receive | ID:" + node_id + " | network: " + net_id);

        String id = "";
        id += String((DataLookline.nodeID / 1000) % 10);
        id += String((DataLookline.nodeID / 100) % 10);
        id += String((DataLookline.nodeID / 10) % 10);
        id += String((DataLookline.nodeID / 1) % 10);

        String StringPlan = "";
        StringPlan += (DataLookline.PLAN / 1000) % 10;
        StringPlan += (DataLookline.PLAN / 100) % 10;
        StringPlan += (DataLookline.PLAN / 10) % 10;
        StringPlan += (DataLookline.PLAN / 1) % 10;

        String StringResult = "";
        StringResult += (DataLookline.RESULT / 1000) % 10;
        StringResult += (DataLookline.RESULT / 100) % 10;
        StringResult += (DataLookline.RESULT / 10) % 10;
        StringResult += (DataLookline.RESULT / 1) % 10;

        String State = "";

        String sentData = id + "04" + "18" + StringPlan + StringResult + State;

        LOGLN(sentData);
    }  else {
        LOGLN("Received unknown data format!");
        return;
    }

    String jsonString;
    serializeJson(jsonDoc, jsonString);
    LOGLN(jsonString);
    // Serial2.println(jsonString);
}

// Helper function to convert MAC address string to byte array
void macStrToBytes(const char* macStr, uint8_t* macBytes) {
    for (int i = 0; i < 6; i++) {
        sscanf(macStr + 3 * i, "%2hhx", &macBytes[i]);
    }
}

// Helper function to convert byte array to MAC address string
void macBytesToStr(const uint8_t* macBytes, char* macStr, size_t size) {
    snprintf(macStr, size, "%02X:%02X:%02X:%02X:%02X:%02X",
             macBytes[0], macBytes[1], macBytes[2],
             macBytes[3], macBytes[4], macBytes[5]);
}

// Function to send DataPacket
void sendDataPacket(const DatasPacket& data) {
    esp_err_t result = esp_now_send(peerMacAddress, (uint8_t*)&data, sizeof(DatasPacket));
    if (result == ESP_OK) {
        LOGLN("DataPacket sent successfully!");
    }   
    else if (result == ESP_ERR_ESPNOW_NOT_INIT)
    {
      LOGLN("ESPNOW not Init.");
    }
    else if (result == ESP_ERR_ESPNOW_ARG)
    {
      LOGLN("Invalid Argument");
    }
    else if (result == ESP_ERR_ESPNOW_INTERNAL)
    {
      LOGLN("Internal Error");
    }
    else if (result == ESP_ERR_ESPNOW_NO_MEM)
    {
      LOGLN("ESP_ERR_ESPNOW_NO_MEM");
    }
    else if (result == ESP_ERR_ESPNOW_NOT_FOUND)
    {
      LOGLN("Peer not found.");
    }
    else
    {
      LOGLN("Unknown error");
    }
}

// Function to send SettingPacket
void sendSettingPacket(const SettingPacket& setting) {
    esp_err_t result = esp_now_send(peerMacAddressSent, (uint8_t*)&setting, sizeof(SettingPacket));
    if (result == ESP_OK) {
        LOGLN("SettingPacket sent successfully!");
    } else {
        LOG("Failed to send SettingPacket! Error code: ");
        LOGLN(result);
    }
}

// Function to send Ping
void sendPing() {
    const char* pingMessage = "Ping from Broker";
    esp_err_t result = esp_now_send(BroadcastMacAddress, (uint8_t*)pingMessage, strlen(pingMessage));
    if (result == ESP_OK) {
        LOGLN("Ping sent successfully!");
    } else {
        LOG("Failed to send Ping! Error code: ");
        LOGLN(result);
    }
}
void sendataLooklineV1(){
    
    esp_now_send(peerMacAddress, (uint8_t *) &DataLookline, sizeof(DataLookline));
}
// Function to process serial input and send appropriate packet
void processSerialInput() {
    static String DataInput = ""; // Buffer to store incoming data
    while (Serial2.available() > 0 && config.role != "Broker" && configMode == false) {
        char incomingChar = Serial2.read(); // Read one character at a time
        if (incomingChar == '\n') { // End of JSON line
            DeserializationError error = deserializeJson(doc, DataInput);
            if (error) {
                LOGLN("Failed to parse JSON!");
                LOGLN(DataInput);
                LOGLN(error.c_str());
                LOGLN("Input length: " + String(DataInput.length()));
                DataInput = ""; // Clear the buffer
                return;
            }else{
                LOGLN("Parse JSON success!");
            }

            if (doc.containsKey("type") && doc["type"] == "DataPacket") {
                data.id = doc["id"];
                JsonArray dataArray = doc["data"].as<JsonArray>();
                for (int i = 0; i < 200; i++) {
                    data.data[i] = dataArray[i];
                }
                if(config.model == "V14" && config.role != "Broker" && !configMode){
                    sendataLooklineV1();
                }
                if(config.model == "V15" && config.role != "Broker" && !configMode){
                    sendDataPacket(data);
                }
            } else if (doc.containsKey("type") && doc["type"] == "SettingPacket") {
                setting.id = doc["id"];
                setting.cmd = doc["cmd"];
                JsonArray dataArray = doc["data"].as<JsonArray>();
                for (int i = 0; i < 200; i++) {
                    setting.data[i] = dataArray[i];
                }

                const char* macStr = "14:2B:2F:DA:0F:FC";//doc["mac"];
                macStrToBytes(macStr, peerMacAddressSent);
                if(config.model == "V14" && config.role != "Broker" && !configMode){
                    sendataLooklineV1();
                    
                }
                if(config.model == "V15" && config.role != "Broker" && !configMode){
                    sendSettingPacket(setting);
                }
            } else if (doc.containsKey("type") && doc["type"] == "GetMacList") {
                LOGLN("MAC Addresses:");
                for (JsonVariant value : config.macAddresses) {
                    LOGLN(value.as<const char*>());
                }
            } else if (doc.containsKey("type") && doc["type"] == "Configuration") {
                configMode = true;
                saveConfig();
                ESP.restart();
            } else if (doc.containsKey("type") && doc["type"] == "Run") {
                configMode = false;
                saveConfig();
                ESP.restart();
            } else if (doc.containsKey("type") && doc["type"] == "GetStatus") {
                LOGLN("{\"Status\":\"" + String(configMode) + "\"}");
            } else {
                LOGLN("Unknown packet type!");
            }

            DataInput = ""; // Clear the buffer after processing
            doc.clear(); // Clear the JSON document
        } else {
            DataInput += incomingChar; // Append character to buffer
        }
    }
    while (Serial.available() > 0) {
        char incomingChar = Serial.read(); // Read one character at a time
        if (incomingChar == '\n') { // End of JSON line
            
            DeserializationError error = deserializeJson(doc, DataInput);
            if (error) {
                LOGLN("> Failed to parse JSON!");
                LOGLN(DataInput);
                LOGLN(error.c_str());
                LOGLN("> Input length: " + String(DataInput.length()));
                DataInput = ""; // Clear the buffer
                return;
            }else{
                LOGLN("> Parse JSON success!");
            }
            if (doc.containsKey("type") && doc["type"] == "ConfigPacket") {
                if (doc.containsKey("mac") && doc.containsKey("node_id") && doc.containsKey("chan") &&
                    doc.containsKey("regBit") && doc.containsKey("reg8") && doc.containsKey("reg16") &&
                    doc.containsKey("role") && doc.containsKey("ssid") && doc.containsKey("password")) {
                    
                    strlcpy(config.mac, doc["mac"], sizeof(config.mac));
                    config.node_id = doc["node_id"];
                    config.chan = doc["chan"];
                    config.regBit = doc["regBit"];
                    config.reg8 = doc["reg8"];
                    config.reg16 = doc["reg16"];
                    config.role = doc["role"].as<String>();
                    strlcpy(config.ssid, doc["ssid"], sizeof(config.ssid));
                    strlcpy(config.password, doc["password"], sizeof(config.password));

                    if (doc.containsKey("macAddresses")) {
                        JsonArray macArray = doc["macAddresses"].as<JsonArray>();
                        config.macAddresses = macArray; // Assign the array
                    }

                    saveConfig();
                    printConfig();
                } else {
                    LOGLN("> Missing required keys in ConfigPacket!");
                }
            }

            DataInput = ""; // Clear the buffer after processing
            doc.clear(); // Clear the JSON document
        } else {
            DataInput += incomingChar; // Append character to buffer
        }
    }
}

// Hàm in cấu hình ra Serial để kiểm tra
void printConfig() {
    LOGLN("===== Cấu hình hiện tại =====");
    LOG("MAC: "); LOGLN(config.mac);
    LOG("Chanel: "); LOGLN(config.chan);
    LOG("Node ID: "); LOGLN(config.node_id);
    LOG("regBit: "); Serial.println(config.regBit, BIN);
    LOG("reg8: "); LOGLN(config.reg8);
    LOG("reg16: "); LOGLN(config.reg16);
    LOG("Run mode: "); LOGLN(configMode ? "Config" : "Run");
    LOG("Role: "); LOGLN(config.role);
    LOG("SSID: "); LOGLN(config.ssid);
    LOG("Password: "); LOGLN(config.password);
    LOG("WiFi Protocol: "); LOGLN(config.rfpower);
    LOG("Network ID: "); LOGLN(config.netId);
    LOG("Model: "); LOGLN(config.model); 
    LOG("ESP32 MAC: "); LOGLN(WiFi.macAddress());    // Init ESP-NOW
    LOGLN("=============================");
}

// Chuyển đổi chế độ khi nhấn nút
void handleButtonPress() {
    configMode = !configMode;
    saveConfig();
    delay(2000);
    ESP.restart();
}

// Lưu cấu hình vào SPIFFS
void saveConfig() {
    StaticJsonDocument<1024> doc;
    doc["mac"] = config.mac;
    doc["node_id"] = config.node_id;
    doc["regBit"] = config.regBit;
    doc["reg8"] = config.reg8;
    doc["reg16"] = config.reg16;
    doc["configMode"] = configMode;
    doc["role"] = config.role;
    doc["chan"] = config.chan;
    doc["ssid"] = config.ssid;
    doc["password"] = config.password;
    doc["rfpower"] = config.rfpower;
    doc["netId"] = config.netId;
    doc["model"] = config.model;

    if (!config.macAddresses.isNull()) {
        JsonArray macArray = doc.createNestedArray("macAddresses");
        for (JsonVariant value : config.macAddresses) {
            macArray.add(value.as<const char*>());
            yield(); // Prevent watchdog timeout during array population
        }
    }

    File file = LittleFS.open(SETTINGS_FILE, FILE_WRITE);
    if (!file) {
        LOGLN("Failed to open config file for writing");
        return;
    }

    if (serializeJson(doc, file) == 0) {
        LOGLN("Failed to write to config file");
    } else {
        LOGLN("Config saved successfully");
    }

    file.close();
    yield(); // Prevent watchdog timeout after file close
    LOGLN("Save config done!");
}

void loadConfig() {
    if (!LittleFS.exists(SETTINGS_FILE)) { // Change to LittleFS
        LOGLN("Config file not found, creating new config file.");

        // Set default configuration
        strlcpy(config.mac, "FF:FF:FF:FF:FF:FF", sizeof(config.mac));
        config.chan = 1;
        config.node_id = 1;
        config.regBit = 0b10101010; // Example default value
        config.reg8 = 255; // Example default value
        config.reg16 = 65535; // Example default value
        configMode = true; // Default config mode
        config.role = "Node"; // Default role
        config.chan = 1; // Default channel
        strlcpy(config.ssid, "ISoft", sizeof(config.ssid));
        strlcpy(config.password, "i-soft@2023", sizeof(config.password));
        config.rfpower = WIFI_PROTOCOL_11B; // Default WiFi protocol
        config.netId = 1; // Default network ID
        config.model = "V15"; // Default model
        config.macAddresses = JsonArray(); // Initialize empty JSON array
        esp_wifi_set_protocol(current_wifi_interface, config.rfpower);
        // Save default configuration to file
        saveConfig();
        return;
    }
  
    File file = LittleFS.open(SETTINGS_FILE, FILE_READ); // Change to LittleFS
    if (!file) {
        LOGLN("Failed to open config file for reading");
        return;
    }

    StaticJsonDocument<1024> doc; // Increase the size of the JSON document
    DeserializationError error = deserializeJson(doc, file);
    if (error) {
        LOGLN("Failed to read config file");
        LOGLN(error.c_str());
        return;
    }
  
    strlcpy(config.mac, doc["mac"], sizeof(config.mac));
    config.chan = doc["chan"];
    config.node_id = doc["node_id"];
    config.regBit = doc["regBit"];
    config.reg8 = doc["reg8"];
    config.reg16 = doc["reg16"];
    configMode = doc["configMode"];
    config.role = doc["role"].as<String>();
    config.chan = doc["chan"];
    strlcpy(config.ssid, doc["ssid"], sizeof(config.ssid));
    strlcpy(config.password, doc["password"], sizeof(config.password));
    config.rfpower = doc["rfpower"];
    config.netId = doc["netId"];
    config.model = doc["model"].as<String>();
    config.macAddresses = doc["macAddresses"].as<JsonArray>();
    LastConfigMode = configMode;
    file.close();
    LOGLN("Load config done!");

    // Set WiFi protocol
    esp_wifi_set_protocol(current_wifi_interface, config.rfpower);
    check_protocol(true);
// Clear the JSON document to free memory
doc.clear();
}

// Web server để cấu hình từ smartphone
void startWebServer() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        String html = "<html lang='vi'><head><link rel='stylesheet' href='/bulma.min.css'><title>Mesh Config</title></head><body><section class='section'><div class='container'><h1 class='title'>Mesh Config</h1><form method='post' action='/save'>"
                      "<div class='field'><label class='label'>Gateway MAC</label><div class='control'><input class='input' name='mac' value='" + String(config.mac) + "'></div></div>"
                      "<div class='field'><label class='label'>Node ID</label><div class='control'><input class='input' name='node_id' value='" + String(config.node_id) + "'></div></div>"
                      "<div class='field'><label class='label'>regBit</label><div class='control'><input class='input' name='regBit' value='" + String(config.regBit) + "'></div></div>"
                      "<div class='field'><label class='label'>reg8</label><div class='control'><input class='input' name='reg8' value='" + String(config.reg8) + "'></div></div>"
                      "<div class='field'><label class='label'>reg16</label><div class='control'><input class='input' name='reg16' value='" + String(config.reg16) + "'></div></div>"
                      "<div class='field'><label class='label'>Role</label><div class='control'><div class='select'><select name='role'>"
                      "<option value='Node'" + String(config.role == "Node" ? " selected" : "") + ">Node</option>"
                      "<option value='Bridge'" + String(config.role == "Bridge" ? " selected" : "") + ">Bridge</option>"
                      "<option value='Broker'" + String(config.role == "Broker" ? " selected" : "") + ">Broker</option>"
                      "</select></div></div></div>"
                      "<div class='field'><label class='label'>Chanel</label><div class='control'><input class='input' name='chan' value='" + String(config.chan) + "'></div></div>"
                      "<div class='field'><label class='label'>SSID</label><div class='control'><input class='input' name='ssid' value='" + String(config.ssid) + "'></div></div>"
                      "<div class='field'><label class='label'>Password</label><div class='control'><input class='input' type='password' name='password' value='" + String(config.password) + "'></div></div>"
                      "<div class='field'><label class='label'>WiFi Protocol</label><div class='control'><div class='select'><select name='rfpower'>"
                      "<option value='1'" + String(config.rfpower == 1 ? " selected" : "") + ">WIFI_PROTOCOL_11B</option>"
                      "<option value='2'" + String(config.rfpower == 2 ? " selected" : "") + ">WIFI_PROTOCOL_11G</option>"
                      "<option value='4'" + String(config.rfpower == 4 ? " selected" : "") + ">WIFI_PROTOCOL_11N</option>"
                      "<option value='3'" + String(config.rfpower == 3 ? " selected" : "") + ">WIFI_PROTOCOL_11B/11G</option>"
                      "<option value='7'" + String(config.rfpower == 7 ? " selected" : "") + ">WIFI_PROTOCOL_11B/11G/11N</option>"
                      "<option value='8'" + String(config.rfpower == 8 ? " selected" : "") + ">WIFI_PROTOCOL_LR</option>"
                      "</select></div></div></div>"
                      "<div class='field'><label class='label'>Network ID</label><div class='control'><input class='input' name='netId' value='" + String(config.netId) + "'></div></div>"
                      "<div class='field'><label class='label'>Model</label><div class='control'><div class='select'><select name='model'>"
                      "<option value='V14'" + String(config.model == "V14" ? " selected" : "") + ">V14</option>"
                      "<option value='V15'" + String(config.model == "V15" ? " selected" : "") + ">V15</option>"
                      "</select></div></div></div>"
                      "<div class='field'><div class='control'><button class='button is-primary' type='submit'>Save</button></div></div></form>"
                      "<form method='post' action='/run'><div class='field'><div class='control'><button class='button is-link' type='submit'>Switch to Run Mode</button></div></div></form></div></section></body></html>";
        request->send(200, "text/html", html);
    });

    server.on("/bulma.min.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/bulma.min.css", "text/css"); // Change to LittleFS
    });

    server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (request->hasParam("mac", true) &&
            request->hasParam("node_id", true) &&
            request->hasParam("regBit", true) &&
            request->hasParam("reg8", true) &&
            request->hasParam("reg16", true) &&
            request->hasParam("role", true) &&
            request->hasParam("chan", true) &&
            request->hasParam("ssid", true) &&
            request->hasParam("password", true) &&
            request->hasParam("rfpower", true) &&
            request->hasParam("netId", true) &&
            request->hasParam("model", true)) {

            strlcpy(config.mac, request->getParam("mac", true)->value().c_str(), sizeof(config.mac));
            config.node_id = request->getParam("node_id", true)->value().toInt();
            config.regBit = request->getParam("regBit", true)->value().toInt();
            config.reg8 = request->getParam("reg8", true)->value().toInt();
            config.reg16 = request->getParam("reg16", true)->value().toInt();
            config.role = request->getParam("role", true)->value();
            config.chan = request->getParam("chan", true)->value().toInt();
            strlcpy(config.ssid, request->getParam("ssid", true)->value().c_str(), sizeof(config.ssid));
            strlcpy(config.password, request->getParam("password", true)->value().c_str(), sizeof(config.password));
            config.rfpower = request->getParam("rfpower", true)->value().toInt();
            config.netId = request->getParam("netId", true)->value().toInt();
            config.model = request->getParam("model", true)->value();

            // Set WiFi protocol
            esp_wifi_set_protocol(current_wifi_interface, config.rfpower);
            check_protocol(true);

            saveConfig();
            request->send(200, "text/html", "<html lang='vi'>Save config success!</html>");
        } else {
            request->send(400, "text/html", "<html lang='vi'>Save Failed!</html>");
        }
    });

    server.on("/run", HTTP_POST, [](AsyncWebServerRequest *request) {
        configMode = false;
        saveConfig();
        request->send(200, "text/html", "<html lang='vi'>Switched to Run Mode. Rebooting...</html>");
        delay(2000);
        ESP.restart();
    });


    // API to get the list of MAC addresses
    server.on("/getMacList", HTTP_GET, [](AsyncWebServerRequest *request) {
        StaticJsonDocument<512> doc;
        JsonArray macArray = doc.createNestedArray("macAddresses");
        for (JsonVariant value : config.macAddresses) {
            macArray.add(value.as<const char*>());
        }
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
        // Clear the JSON document to free memory
        doc.clear();
    });

    // API to add a new MAC address to the list
    server.on("/addMac", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (request->hasParam("mac", true)) {
            String newMac = request->getParam("mac", true)->value();
            bool macExists = false;
            for (JsonVariant value : config.macAddresses) {
                if (strcmp(value.as<const char*>(), newMac.c_str()) == 0) {
                    macExists = true;
                    break;
                }
            }
            if (!macExists) {
                config.macAddresses.add(newMac);
                saveConfig(); // Save the updated configuration
                request->send(200, "text/plain", "MAC address added successfully!");
            } else {
                request->send(400, "text/plain", "MAC address already exists!");
            }
        } else {
            request->send(400, "text/plain", "MAC address parameter missing!");
        }
    });

    // API to delete a MAC address from the list
    server.on("/deleteMac", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (request->hasParam("mac", true)) {
            String macToDelete = request->getParam("mac", true)->value();
            for (size_t i = 0; i < config.macAddresses.size(); i++) {
                if (strcmp(config.macAddresses[i], macToDelete.c_str()) == 0) {
                    config.macAddresses.remove(i);
                    saveConfig(); // Save the updated configuration
                    request->send(200, "text/plain", "MAC address deleted successfully!");
                    return;
                }
            }
            request->send(400, "text/plain", "MAC address not found!");
        } else {
            request->send(400, "text/plain", "MAC address parameter missing!");
        }
    });

    // Web page to manage MAC address list
    server.on("/manageMac", HTTP_GET, [](AsyncWebServerRequest *request) {
        String html = "<html lang='vi'><h1>Manage MAC Addresses</h1><form method='post' action='/addMac'>"
                      "Add MAC: <input name='mac'><br>"
                      "<input type='submit' value='Add'></form><br><br>";
        if (config.macAddresses.size() > 0) {
            html += "<form method='post' action='/deleteMac'>"
                    "Delete MAC: <input name='mac'><br>"
                    "<input type='submit' value='Delete'></form><br><br>"
                    "<h2>Current MAC Addresses:</h2><ul>";
            for (JsonVariant value : config.macAddresses) {
                html += "<li>" + String(value.as<const char*>()) + "</li>";
            }
            html += "</ul>";
        } else {
            html += "<p>No MAC addresses found.</p>";
        }
        html += "</html>";
        request->send(200, "text/html", html);
    });

    // Route for updating firmware
    server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
        String html = "<html lang='vi'><h1>Update Firmware</h1><form method='POST' action='/update' enctype='multipart/form-data'>"
                      "<input type='file' name='firmware'><br>"
                      "<input type='submit' value='Update'></form></html>";
        request->send(200, "text/html", html);
    });

    server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (Update.hasError()) {
            request->send(200, "text/plain", "Update Failed!");
            LOGLN("Update Failed!");
        } else {
            request->send(200, "text/plain", "Update Successful. Rebooting...");
            LOGLN("Update Successful. Rebooting...");
            delay(2000);
            ESP.restart();
        }
    }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
        if (!index) {
            Serial.printf("Update Start: %s\n", filename.c_str());
            if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
                Update.printError(Serial);
                request->send(500, "text/plain", "Update Begin Failed!");
                return;
            }
        }
        if (Update.write(data, len) != len) {
            Update.printError(Serial);
            request->send(500, "text/plain", "Update Write Failed!");
            return;
        }
        if (final) {
            if (Update.end(true)) {
                Serial.printf("Update Success: %uB\n", index + len);
            } else {
                Update.printError(Serial);
                request->send(500, "text/plain", "Update End Failed!");
                return;
            }
        }
    });
    // Route for updating file system
    server.on("/updateFS", HTTP_GET, [](AsyncWebServerRequest *request) {
        String html = "<html lang='vi'><h1>Update File System</h1><form method='POST' action='/updateFS' enctype='multipart/form-data'>"
                      "<input type='file' name='filesystem'><br>"
                      "<input type='submit' value='Update'></form></html>";
        request->send(200, "text/html", html);
    });

    server.on("/updateFS", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (Update.hasError()) {
            request->send(200, "text/plain", "Update Failed!");
            LOGLN("Update Failed!");
        } else {
            request->send(200, "text/plain", "Update Successful. Rebooting...");
            LOGLN("Update Successful. Rebooting...");
            delay(2000);
            ESP.restart();
        }
    }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
        if (!index) {
            Serial.printf("Update Start: %s\n", filename.c_str());
            if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_SPIFFS)) { // Start with SPIFFS partition
                Update.printError(Serial);
                request->send(500, "text/plain", "Update Begin Failed!");
                return;
            }
        }
        if (Update.write(data, len) != len) {
            Update.printError(Serial);
            request->send(500, "text/plain", "Update Write Failed!");
            return;
        }
        if (final) {
            if (Update.end(true)) {
                Serial.printf("Update Success: %uB\n", index + len);
            } else {
                Update.printError(Serial);
                request->send(500, "text/plain", "Update End Failed!");
                return;
            }
        }
    });

    server.begin();
}

void startFirmwareUploadServer() {
    // Serve the firmware upload form
    server.on("/upload", HTTP_GET, [](AsyncWebServerRequest *request) {
        String html = "<html lang='en'><head><title>Firmware Upload</title></head><body>"
                      "<h1>Upload Firmware</h1>"
                      "<form method='POST' action='/upload' enctype='multipart/form-data'>"
                      "<input type='file' name='firmware'><br><br>"
                      "<input type='submit' value='Upload'>"
                      "</form></body></html>";
        request->send(200, "text/html", html);
    });

    // Handle the firmware upload
    server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (Update.hasError()) {
            request->send(500, "text/plain", "Firmware update failed!");
            LOGLN("Firmware update failed!");
        } else {
            request->send(200, "text/plain", "Firmware update successful. Rebooting...");
            LOGLN("Firmware update successful. Rebooting...");
            delay(2000);
            ESP.restart();
        }
    }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
        if (!index) {
            Serial.printf("Firmware upload started: %s\n", filename.c_str());
            if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { // Start firmware update
                Update.printError(Serial);
            }
        }
        if (Update.write(data, len) != len) { // Write firmware data
            Update.printError(Serial);
        }
        if (final) {
            if (Update.end(true)) { // End firmware update
                Serial.printf("Firmware upload completed: %u bytes\n", index + len);
            } else {
                Update.printError(Serial);
            }
        }
    });
}

// Kiểm tra nút nhấn
void checkButton() {
    static unsigned long lastPress = 0;
    if (digitalRead(BUTTON_PIN) == LOW) {
        if (millis() - lastPress > 500) {
            handleButtonPress();
            lastPress = millis();
        }
    }
}

// Function to send DatasPacket via Serial
void sendDatasPacketSerial(const DatasPacket& packet) {
    Serial.write((uint8_t*)&packet, sizeof(DatasPacket)); // Send the raw bytes of the struct
    Serial.flush(); // Ensure all data is sent
}

// Function to receive DatasPacket via Serial
bool receiveDatasPacketSerial(DatasPacket& packet) {
    static uint8_t buffer[sizeof(DatasPacket)];
    static size_t bytesRead = 0;

    while (Serial.available() > 0) {
        buffer[bytesRead++] = Serial.read(); // Read one byte at a time
        if (bytesRead == sizeof(DatasPacket)) { // Check if the full packet has been received
            memcpy(&packet, buffer, sizeof(DatasPacket)); // Copy the data into the struct
            bytesRead = 0; // Reset the buffer for the next packet
            return true; // Packet successfully received
        }
    }
    return false; // No complete packet received yet
}

void setup() {
    Serial.begin(115200);
    Serial2.begin(115200);
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    // Disable brownout detector
    // WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    // delay(100); // Add a delay to ensure stability

    if (!LittleFS.begin(true)) {
        LOGLN("Init LittleFS failed!");
        return;
    }

    loadConfig();
    printConfig();
    configMode = false;
    if (configMode) {
        LOGLN("AP mode init");
        // Start AP mode
        WiFi.disconnect(true);
        WiFi.mode(WIFI_AP);
        esp_wifi_set_protocol(current_wifi_interface, WIFI_PROTOCOL_11N);check_protocol(1);
        LOGLN("AP debug");

        String macAddress = WiFi.macAddress();
        macAddress.replace(":", "");
        String WifiName = "iSoftMeshConfig_" + macAddress.substring(0, 6);
        WiFi.softAP(WifiName.c_str(), "12345678");
        LOGLN("Wi-Fi AP started! SSID: " + WifiName + ", Password: 12345678");
        dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
        startWebServer();
        startFirmwareUploadServer();
        LOGLN("AP mode started!");
        LOGLN("AP IP Address: " + WiFi.softAPIP().toString());
    } else {
        WiFi.mode(WIFI_STA);
        // if (config.ssid[0] != '\0') {
        //     WiFi.begin(config.ssid, config.password);
        //     LOG("Connecting to WiFi...");
        //     unsigned long startAttemptTime = millis();
        //     while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
        //         delay(500);
        //         LOG(".");
        //     }
        //     if (WiFi.status() == WL_CONNECTED) {
        //         LOGLN("\nConnected to WiFi!");
        //         LOGLN("IP Address: " + WiFi.localIP().toString());
        //     } else {
        //         LOGLN("\nFailed to connect to WiFi!");
        //     }
        // } else {
        //     LOGLN("WiFi credentials not configured!");
        // }

        esp_wifi_set_channel(config.chan, WIFI_SECOND_CHAN_NONE);
        LOG("Wi-Fi Channel set to: ");
        LOGLN(config.chan);

        if (esp_now_init() != ESP_OK) {
            LOGLN("ESP-NOW Init Failed");
            return;
        }

        esp_now_register_send_cb(OnDataSent);
        esp_now_register_recv_cb(OnDataRecv);

        if (!esp_now_is_peer_exist(peerMacAddress))
        {
            macStrToBytes(config.mac, peerMacAddress);
            memcpy(peerInfo.peer_addr, peerMacAddress, 6);
            peerInfo.channel = config.chan;
            peerInfo.encrypt = false;
            if (esp_now_add_peer(&peerInfo) != ESP_OK) {
                LOGLN("Failed to add peer");
            } else {
                LOGLN("Peer added successfully");
            }
        }
    }
}

void loop() {
    checkButton();
    // processSerialInput(); // Xử lý dữ liệu từ Serial

    if (configMode && LastConfigMode == configMode) {
        dnsServer.processNextRequest();
    }
    else{
        static unsigned long lastSendTime = 0;
        static unsigned long lastPrintTime = 0;
        if ((config.role == "Node" || config.role == "Bridge"  )&& millis() - lastSendTime > 3000 && LastConfigMode == configMode) {
            // DatasPacket data = {1, 1, 100, 90, 120, 2}; // Example data
            lastSendTime = millis();
        }

        
        // Process new MAC address if received
        // if (newMacReceived) {
        //     newMacReceived = false;
        //     bool macExists = false;
        //     for (JsonVariant value : config.macAddresses) {
        //         if (strcmp(value.as<const char*>(), newMacStr) == 0) {
        //             macExists = true;
        //             break;
        //         }
        //     }
        //     if (!macExists) {
        //         config.macAddresses.add(newMacStr);
        //         saveConfig(); // Save the updated configuration
        //     }
        // }

        // if (config.role == "Broker" && millis() - lastSendTime > 5000) {
        //     sendPing();
        //     lastSendTime = millis();
        // }
        // Print free RAM and chip temperature every 5 seconds
        if (millis() - lastPrintTime > 5000) {
            LOG("Free RAM: ");
            LOG(ESP.getFreeHeap()/1000);
            LOGLN(" bytes");

            LOG("Chip Temperature: ");
            LOG(temperatureRead());
            LOGLN(" °C");

            lastPrintTime = millis();
        }
    }
}

String serializeDataPacketToJson(const DatasPacket& data) {
    StaticJsonDocument<1024> jsonDoc;
    jsonDoc["type"] = "DataPacket";
    jsonDoc["id"] = data.id;
    JsonArray dataArray = jsonDoc.createNestedArray("data");
    for (int i = 0; i < 200; i++) {
        dataArray.add(data.data[i]);
    }
    String jsonString;
    serializeJson(jsonDoc, jsonString);
    return jsonString;
}

String serializeSettingPacketToJson(const SettingPacket& setting) {
    StaticJsonDocument<1024> jsonDoc;
    jsonDoc["type"] = "SettingPacket";
    jsonDoc["id"] = setting.id;
    jsonDoc["cmd"] = setting.cmd;
    JsonArray dataArray = jsonDoc.createNestedArray("data");
    for (int i = 0; i < 200; i++) {
        dataArray.add(setting.data[i]);
    }
    String jsonString;
    serializeJson(jsonDoc, jsonString);
    return jsonString;
}

String serializeConfigPacketToJson(const NodeConfig& config) {
    StaticJsonDocument<1024> jsonDoc;
    jsonDoc["type"] = "ConfigPacket";
    jsonDoc["mac"] = config.mac;
    jsonDoc["node_id"] = config.node_id;
    jsonDoc["chan"] = config.chan;
    jsonDoc["regBit"] = config.regBit;
    jsonDoc["reg8"] = config.reg8;
    jsonDoc["reg16"] = config.reg16;
    jsonDoc["role"] = config.role;
    jsonDoc["ssid"] = config.ssid;
    jsonDoc["password"] = config.password;
    JsonArray macArray = jsonDoc.createNestedArray("macAddresses");
    for (JsonVariant value : config.macAddresses) {
        macArray.add(value.as<const char*>());
    }
    String jsonString;
    serializeJson(jsonDoc, jsonString);
    return jsonString;
}

void sendUdpPacket(WiFiUDP& udp, const char* targetIp, uint16_t port, const char* message) {
    if (WiFi.status() != WL_CONNECTED) {
        LOGLN("Wi-Fi not connected. Cannot send UDP packet.");
        return;
    }

    udp.beginPacket(targetIp, port);
    udp.write(reinterpret_cast<const uint8_t*>(message), strlen(message)); // Cast message to const uint8_t*
    if (udp.endPacket() == 0) {
        LOGLN("Failed to send UDP packet!");
    } else {
        LOGLN("UDP packet sent successfully!");
    }
}