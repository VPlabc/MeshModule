#include "main.h"
#include "WebInterface.h"
#ifdef USE_Modbus
#include "TskMQTT.h"
WifiMqttConfig wifiMqttConfig;
#endif// USE_Modbus
#ifdef USE_Modbus
#include "Modbus_RTU.h"
Modbus_Prog mainModbusCom;
#endif//USE_Modbus   
// #include "Project/lookline.h"
struct Config {
    uint8_t BrokerAddress[6];
    int wifiChannel;
    int id;
    int netId;
    String role; // "Broker", "Node", or "Repeater"
    bool debug;  // Enable/disable debug prints
    JsonArray macSlaves;
    int dataVersion; // Data version 0: Lookline v1, 1: Lookline v2, 2: Modbus Register, 3: Serial TTL
} MeshConfig;

struct __attribute__((packed)) dataPacket {
    int ID;
    int netId;
    uint8_t data[200];
};

dataPacket packet;
struct __attribute__((packed)) bind {
    int id;
    char cmd[5]; // "BIND" (4 characters + null terminator)
};

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

struct_Parameter_message DataLooklinev1;

typedef struct struct_Parameter_messageOld {
    byte networkID;       //1
    byte nodeID;          //1
    int PLAN;             //4
    int RESULT;           //4
    byte state;           //1
    byte Mode;            //1
  } struct_Parameter_messageOld;

  struct_Parameter_messageOld DataLookline;

JSONVar mainModbusSetting;
  
#include <DNSServer.h>
DNSServer dnsServer;
const byte DNS_PORT = 53;
IPAddress apIP(192,168,4,1);
IPAddress subnet(255, 255, 255, 0);
bool configMode = false;



// void WiFiEvent(WiFiEvent_t event);
void convertDataPacketToDataLookline(const dataPacket &packet, struct_Parameter_messageOld &dataLookline);
  
//#define TCP_ETH
#define RTU_RS485


#ifdef USE_Modbus
#include "TskModbus.h"
ModbusConfig mainModbusConfig;
TaskHandle_t TaskMQTT;
#ifdef RTU_RS485
#include "Modbus_RTU.h"
Modbus_Prog iMagModbusCom;
#endif//RTU_RS485
  #ifdef TCP_ETH
  #include "Network/Modbus_TCP.h"
  Modbus_TCP_Prog iMagModbusCom;
  #endif//TCP_ETH

void ModbusInit(String ModbusParameter)
{
    #ifdef RTU_RS485
    iMagModbusCom.modbus_setup(ModbusParameter);
    #endif//RTU_RS485
    #ifdef TCP_ETH
        if((const char*)mainModbusSetting["role"] == "master"){TCP_setup(0);}
        if((const char*)mainModbusSetting["role"] == "slave"){TCP_setup(1);}
    #endif//TCP_ETH
}
void ModbusLoop(int Timeout)
{
    #ifdef RTU_RS485
    iMagModbusCom.modbus_loop(Timeout);
    #endif//RTU_RS485
    #ifdef TCP_ETH
        if((const char*)mainModbusSetting["role"] == "master"){TCP_loop(0);}
        if((const char*)mainModbusSetting["role"] == "slave"){TCP_loop(1);}
    #endif//TCP_ETH
}

#endif//USE_Modbus

void convertDataPacketToDataLookline(const dataPacket &packet, struct_Parameter_messageOld &dataLookline) {
    dataLookline.PLAN = (packet.data[0] << 8) | packet.data[1];
    dataLookline.RESULT = (packet.data[2] << 8) | packet.data[3];
    dataLookline.state = packet.data[4];
    dataLookline.Mode = packet.data[5];
}
void DataForPC(){
    if(MeshConfig.role == "Broker"){
        uint16_t Plan, Result;
        uint8_t state, RSSI, Com, WiFi, type, Cmd;
        Plan = (packet.data[0] << 8) | packet.data[1];
        Result = (packet.data[2] << 8) | packet.data[3];
        state = packet.data[4];
        RSSI = packet.data[5];
        Com = packet.data[6];
        WiFi = packet.data[7];
        type = packet.data[8];
        Cmd = packet.data[9];

        String net_id = "";
        net_id += String((packet.netId / 1000) % 10);
        net_id += String((packet.netId / 100) % 10);
        net_id += String((packet.netId / 10) % 10);
        net_id += String((packet.netId / 1) % 10);
    
        String id = "";
        id += String((packet.ID / 1000) % 10);
        id += String((packet.ID / 100) % 10);
        id += String((packet.ID / 10) % 10);
        id += String((packet.ID / 1) % 10);

        if (MeshConfig.debug) {LOGLN("Mesh receive | ID:" + id + " | network: " + net_id);}

        String StringPlan = "";
        StringPlan += (Plan / 1000) % 10;
        StringPlan += (Plan / 100) % 10;
        StringPlan += (Plan / 10) % 10;
        StringPlan += (Plan / 1) % 10;

        String StringResult = "";
        StringResult += (Result / 1000) % 10;
        StringResult += (Result / 100) % 10;
        StringResult += (Result / 10) % 10;
        StringResult += (Result / 1) % 10;

        String State = "";
        State += (state / 1000) % 10;
        State += (state / 100) % 10;    
        State += (state / 10) % 10;
        State += (state / 1) % 10;

        String sentData = id + "04" + "18" + StringPlan + StringResult + State;

        LOGLN(sentData);
    } else {
        if (MeshConfig.debug) Serial.println("Data received from Mesh");
        if(MeshConfig.debug && MeshConfig.role != "Broker") Serial.println("device not Broker!");
    }
}
void SerialInit() {
    #ifdef USE_SERIAL1
    Serial1.begin(Se_BAUD_RATE, SERIAL_8N1, Se_dRX_PIN, Se_TX_PIN);
    Serial1.println("Serial1 initialized.");
    #endif//USE_SERIAL1
    #ifdef USE_SERIAL2
    
    Serial2.begin(Se_BAUD_RATE, SERIAL_8N1, 16, 17);
    Serial2.println("Serial2 initialized.");
    // Serial2.begin(Se_BAUD_RATE, SERIAL_8N1, Se_dRX_PIN, Se_TX_PIN);
    // Serial2.println("Serial2 initialized.");
    #endif//USE_SERIAL2
}

void formatMacAddress(const uint8_t *macAddr, char *buffer, int maxLength)
{
  snprintf(buffer, maxLength, "%02x:%02x:%02x:%02x:%02x:%02x", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
}

String convertBrokerAddressToString(const uint8_t *address) {
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X", 
             address[0], address[1], address[2], address[3], address[4], address[5]);
    return String(macStr);
}

#ifndef Gateway
void loadConfig() {
    if (!FileSystem.exists(CONFIG_FILE)) {
        Serial.println("Config file not found, creating default MeshConfig.");
        // Set default values
        static const uint8_t defaultBrokerAddress[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        memcpy(MeshConfig.BrokerAddress, defaultBrokerAddress, 6);
        MeshConfig.wifiChannel = 1;
        MeshConfig.id = 1;
        MeshConfig.netId = 1;
        MeshConfig.role = "Node"; // Default role
        MeshConfig.debug = true;  // Default debug enabled
        DynamicJsonDocument doc(512);
        MeshConfig.macSlaves = doc.createNestedArray("macSlaves");
        MeshConfig.dataVersion = 0; // Default dataVersion
        saveConfig();
        return;
    }

    File file = FileSystem.open(CONFIG_FILE, "r");
    if (!file) {
        Serial.println("Failed to open config file for reading.");
        return;
    }

    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, file);
    if (error) {
        Serial.println("Failed to parse config file.");
        Serial.println(error.c_str());
        return;
    }

    const char* BrokerStr = doc["BrokerAddress"];
    if (BrokerStr) {
        int values[6];
        if (sscanf(BrokerStr, "%02x:%02x:%02x:%02x:%02x:%02x", 
                   &values[0], &values[1], &values[2], 
                   &values[3], &values[4], &values[5]) == 6) {
            for (int i = 0; i < 6; i++) {
                MeshConfig.BrokerAddress[i] = static_cast<uint8_t>(values[i]);
            }
        } else {
            if (MeshConfig.debug) Serial.println("Invalid BrokerAddress format in MeshConfig.");
        }
    } else {
        if (MeshConfig.debug) Serial.println("BrokerAddress not found in MeshConfig.");
    }

    MeshConfig.wifiChannel = doc["wifiChannel"];
    MeshConfig.id = doc["id"];
    MeshConfig.netId = doc["netId"];
    MeshConfig.dataVersion = doc["dataVersion"] | 0; // Default to 0 if not specified
    MeshConfig.role = doc["role"] | "Node"; // Default to "Node" if not specified
    MeshConfig.debug = doc["debug"] | true; // Default debug enabled
    MeshConfig.macSlaves = doc["macSlaves"].as<JsonArray>();
    file.close();
    if (MeshConfig.debug) Serial.println("Config loaded.");
}

void saveConfig() {
    File file = FileSystem.open(CONFIG_FILE, "w");
    if (!file) {
        if (MeshConfig.debug) Serial.println("Failed to open config file for writing.");
        return;
    }

    DynamicJsonDocument doc(512);
    doc["BrokerAddress"] = convertBrokerAddressToString(MeshConfig.BrokerAddress);
    doc["wifiChannel"] = MeshConfig.wifiChannel;
    doc["id"] = MeshConfig.id;
    doc["netId"] = MeshConfig.netId;
    doc["dataVersion"] = MeshConfig.dataVersion; // Save data version
    doc["role"] = MeshConfig.role; // Save role
    doc["debug"] = MeshConfig.debug; // Save debug flag
    JsonArray macArray = doc.createNestedArray("macSlaves");
    for (JsonVariant value : MeshConfig.macSlaves) {
        macArray.add(value.as<const char*>());
    }

    if (serializeJson(doc, file) == 0) {
        if (MeshConfig.debug) Serial.println("Failed to write to config file.");
    } else {
        if (MeshConfig.debug) Serial.println("Config saved.");
    }
    file.close();
}

void sendBindRequest() {
    bind bindRequest;
    bindRequest.id = MeshConfig.id;
    strncpy(bindRequest.cmd, "BIND", sizeof(bindRequest.cmd));

    esp_now_peer_info_t peerInfo = {};
    memcpy(&peerInfo.peer_addr, MeshConfig.BrokerAddress, 6);
    if (!esp_now_is_peer_exist(MeshConfig.BrokerAddress)) {
        peerInfo.channel = MeshConfig.wifiChannel;
        esp_now_add_peer(&peerInfo);
    }

    esp_err_t result = esp_now_send(MeshConfig.BrokerAddress, (uint8_t*)&bindRequest, sizeof(bindRequest));
    if (result == ESP_OK) {
        if (MeshConfig.debug) Serial.println("Bind request sent successfully.");
    } else {
        if (MeshConfig.debug) Serial.println("Failed to send bind request.");
    }
}
/// @brief ///////////
/// @param ////////////
/// @return //////////
void processSerialInput() {
    static String inputBuffer = "";
    while (Serial.available() > 0) {
        char incomingChar = Serial.read();
        if (incomingChar == '\n') {
            DynamicJsonDocument doc(512);
            DeserializationError error = deserializeJson(doc, inputBuffer);
            if (error) {
                if (MeshConfig.debug) Serial.println("Failed to parse JSON input.");
                inputBuffer = "";
                return;
            }

            // Handle binding mode activation
            if (doc.containsKey("cmd") && doc["cmd"] == "Binding") {
                if (MeshConfig.debug) Serial.println("Binding mode activated.");
                sendBindRequest();
            }
            else if (doc.containsKey("cmd") && doc["cmd"] == "PrintMac") {
                if (MeshConfig.debug) Serial.println("MAC list");
                printMacList();
            }
            #ifdef USE_Modbus
            else if (doc.containsKey("Modbus") && doc["Modbus"] == "Config") {
                mainModbusConfig.saveJsonToModbusFile(incomingChar, MeshConfig.debug, FileSystem);
                if (MeshConfig.debug) Serial.println("Modbus config saved.");
            }
            #endif//USE_Modbus
            else
            {
                if (doc.containsKey("BrokerAddress")) {
                    const char* BrokerStr = doc["BrokerAddress"];
                    if (BrokerStr) {
                        int values[6];
                        if (sscanf(BrokerStr, "%02x:%02x:%02x:%02x:%02x:%02x", 
                                &values[0], &values[1], &values[2], 
                                &values[3], &values[4], &values[5]) == 6) {
                            for (int i = 0; i < 6; i++) {
                                MeshConfig.BrokerAddress[i] = static_cast<uint8_t>(values[i]);
                            }
                        } else {
                            if (MeshConfig.debug) Serial.println("Invalid BrokerAddress format in JSON input.");
                        }
                    }
                }
                if (doc.containsKey("wifiChannel")) {
                    MeshConfig.wifiChannel = doc["wifiChannel"];
                    esp_now_peer_info_t peerInfo;
                    if (esp_now_get_peer(MeshConfig.BrokerAddress, &peerInfo) == ESP_OK) {
                        peerInfo.channel = MeshConfig.wifiChannel; // Cập nhật kênh
                        esp_now_mod_peer(&peerInfo);  // Áp dụng thay đổi
                        Serial.println("Peer channel updated.");
                    } else {
                        Serial.println("Failed to get peer info.");
                    }
                }
                if (doc.containsKey("id")) {
                    MeshConfig.id = doc["id"];
                }
                if (doc.containsKey("netId")) {
                    MeshConfig.netId = doc["netId"];
                }
                if( doc.containsKey("dataVersion")) {
                    MeshConfig.dataVersion = doc["dataVersion"];
                }
                if (doc.containsKey("role")) {
                    MeshConfig.role = doc["role"].as<String>();
                }
                if (doc.containsKey("debug")) {
                    MeshConfig.debug = doc["debug"];
                }
                if (doc.containsKey("macSlaves")) {
                    JsonArray macArray = doc["macSlaves"].as<JsonArray>();
                    MeshConfig.macSlaves = macArray;
                }

                saveConfig();
                if (MeshConfig.debug) Serial.println("Config updated.");
            }
                
            inputBuffer = "";
        } else {
            inputBuffer += incomingChar;
        }
    }
}
///////////////////////////// Print /////////////////////////////////////////////
void printConfig() {
    if (!MeshConfig.debug) return; // Skip printing if debug is disabled
    Serial.println("===== Current Config =====");
    Serial.print("Broker Address: ");
    Serial.println(convertBrokerAddressToString(MeshConfig.BrokerAddress));
    Serial.print("WiFi Channel: ");
    Serial.println(MeshConfig.wifiChannel);
    Serial.print("ID: ");
    Serial.println(MeshConfig.id);
    Serial.print("Net ID: ");
    Serial.println(MeshConfig.netId);
    Serial.print("Role: ");
    Serial.println(MeshConfig.role); // Print role
    Serial.print("Data Version: ");
    Serial.println(MeshConfig.dataVersion); // Print data version
    Serial.print("Debug: ");
    Serial.println(MeshConfig.debug ? "Enabled" : "Disabled");
    Serial.println("MAC Slaves:");
    for (JsonVariant value : MeshConfig.macSlaves) {
        Serial.println(value.as<const char*>());
    }
    Serial.println("=========================");
}

void printMacList() {
    if (!FileSystem.exists(MACLIST_FILE)) {
        if (MeshConfig.debug) Serial.println("maclist.json not found.");
        return;
    }

    File file = FileSystem.open(MACLIST_FILE, "r");
    if (!file) {
        if (MeshConfig.debug) Serial.println("Failed to open maclist.json for reading.");
        return;
    }

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        if (MeshConfig.debug) Serial.println("Failed to parse maclist.json.");
        if (MeshConfig.debug) Serial.println(error.c_str());
        return;
    }

    if (MeshConfig.debug) Serial.println("===== MAC List =====");
    for (JsonObject obj : doc.as<JsonArray>()) {
        if (MeshConfig.debug) Serial.print("ID: ");
        if (MeshConfig.debug) Serial.print(obj["id"].as<int>());
        if (MeshConfig.debug) Serial.print(" | MAC: ");
        if (MeshConfig.debug) Serial.println(obj["mac"].as<const char*>());
    }
    if (MeshConfig.debug) Serial.println("====================");
}
#endif//Gateway
///////////////////////////// Print /////////////////////////////////////////////
// Function to save MAC address and ID to maclist.json
void saveMacToMacList(const uint8_t *macAddr, int id) {
    File file = FileSystem.open(MACLIST_FILE, "r");
    DynamicJsonDocument doc(1024);

    if (file) {
        DeserializationError error = deserializeJson(doc, file);
        if (error) {
            if (MeshConfig.debug) Serial.println("Failed to parse maclist.json, creating a new one.");
            doc.clear();
        }
        file.close();
    }

    // Check if the MAC address already exists
    String macStr = convertBrokerAddressToString(macAddr);
    bool exists = false;
    for (JsonObject obj : doc.as<JsonArray>()) {
        if (obj["mac"] == macStr) {
            exists = true;
            break;
        }
    }

    if (!exists) {
        JsonObject newEntry = doc.createNestedObject();
        newEntry["mac"] = macStr;
        newEntry["id"] = id;

        file = FileSystem.open(MACLIST_FILE, "w");
        if (!file) {
            if (MeshConfig.debug) Serial.println("Failed to open maclist.json for writing.");
            return;
        }

        if (serializeJson(doc, file) == 0) {
            if (MeshConfig.debug) Serial.println("Failed to write to maclist.json.");
        } else {
            if (MeshConfig.debug) Serial.println("MAC address and ID saved to maclist.json.");
        }
        file.close();
    } else {
        if (MeshConfig.debug) Serial.println("MAC address already exists in maclist.json.");
    }
}





#ifdef ESP32
// callback when data is sent
void sentCallback(const uint8_t *macAddr, esp_now_send_status_t status)
{
    // char macStr[18];
    // formatMacAddress(macAddr, macStr, 18);
    // Serial.print("Last Packet Sent to: ");
    // Serial.println(macStr);
    if (MeshConfig.debug) Serial.print("Last Packet Send Status: ");
    if (MeshConfig.debug) Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
#ifdef ESP32_RISCV
void receiveCallback(const esp_now_recv_info *recvInfo, const uint8_t *data, int dataLen) {
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
             recvInfo->src_addr[0], recvInfo->src_addr[1], recvInfo->src_addr[2],
             recvInfo->src_addr[3], recvInfo->src_addr[4], recvInfo->src_addr[5]);

    if (dataLen == sizeof(dataPacket)) {
        if (MeshConfig.debug) {
            Serial.println("DataPacket received from Mesh (dataVersion = 1):");
            Serial.print("ID: " + String(MeshConfig.id) + " | ");
            Serial.print("Received data from: ");
            Serial.println(macStr);
        }
        memcpy(&packet, data, sizeof(packet));

        #ifdef USE_SERIAL1
        Serial1.write((uint8_t*)&packet, sizeof(packet)); // Send struct over Serial1
        Serial1.flush();
        #endif
        #ifdef USE_SERIAL2
        Serial2.write((uint8_t*)&packet, sizeof(packet)); // Send struct over Serial2
        Serial2.flush();
        #endif

        if (MeshConfig.debug) {
            Serial.print("ID: " + String(packet.ID) + " | netId: " + String(packet.netId));
            Serial.println();
        }

    } else if (dataLen == sizeof(bind)) {
        bind receivedBind;
        memcpy(&receivedBind, data, sizeof(bind));

        if (strncmp(receivedBind.cmd, "BIND", sizeof(receivedBind.cmd)) == 0) {
            if (MeshConfig.debug) Serial.println("Bind request received.");
            saveMacToMacList(recvInfo->src_addr, receivedBind.id);

            // Send acknowledgment back to the sender
            String ackMessage = "{\"id\":" + String(MeshConfig.id) + ",\"cmd\":\"BindAck\"}";
            esp_now_send(recvInfo->src_addr, (const uint8_t *)ackMessage.c_str(), ackMessage.length());
        }
    }
}
#else//ESP32_RISCV

void receiveCallback(const uint8_t *senderMac, const uint8_t *data, int dataLen)
{
    StaticJsonDocument<3500> jsonDoc;
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", senderMac[0], senderMac[1], senderMac[2], senderMac[3], senderMac[4], senderMac[5]);
    if(dataLen == sizeof(dataPacket)) {
            if (MeshConfig.debug) Serial.println("DataPacket received from Mesh (dataVersion = 1):");
            if (MeshConfig.debug) Serial.print("ID: " + String(MeshConfig.id) + " | ");
            if (MeshConfig.debug) Serial.print("Received data from: ");
            if (MeshConfig.debug) Serial.println(macStr);
            memcpy(&packet, data, sizeof(packet));
            #ifdef USE_SERIAL1
                #define SerialTTL Serial1
            #else//USE_SERIAL1
                #define SerialTTL Serial2
            #endif//USE_SERIAL1
            if(MeshConfig.dataVersion == 3){
                SerialTTL.write((uint8_t*)&packet, sizeof(packet)); //Data version3: Send struct over Serial TTL
                SerialTTL.flush();
            }
            else if(MeshConfig.dataVersion == 2){//Modbus Reg
                #ifdef USE_Modbus
                    // mainModbusCom.ModbusGetData(packet.data, sizeof(packet.data));
                #endif//USE_Modbus
            }
            else if(MeshConfig.dataVersion == 1){
                if(MeshConfig.role == "Broker"){
                    DataForPC();
                }
            }
            else if(MeshConfig.dataVersion == 0){
                
            }

    }
    if(dataLen == sizeof(DataLookline)) {
        if (MeshConfig.debug) Serial.println("DataLookline received from Mesh (dataVersion = 0):");//Data version0: Lookline v1 
        memcpy(&DataLookline, data, sizeof(DataLookline));
        if(MeshConfig.debug) {
            // In ra dữ liệu đã chuyển đổi
            Serial.println("DataLookline:");
            Serial.print("Network ID: ");
            Serial.println(DataLookline.networkID);
            Serial.print("Node ID: ");
            Serial.println(DataLookline.nodeID);
            Serial.print("PLAN: ");
            Serial.println(DataLookline.PLAN);
            Serial.print("RESULT: ");
            Serial.println(DataLookline.RESULT);
            Serial.print("State: ");
            Serial.println(DataLookline.state);
            Serial.print("Mode: ");
            Serial.println(DataLookline.Mode);
        }
        // Send DataLookline to Serial2
        #ifdef USE_SERIAL1
            #define SerialTTL Serial1
        #else//USE_SERIAL1
            #define SerialTTL Serial2
        #endif//USE_SERIAL1
            SerialTTL.write((uint8_t*)&DataLookline, sizeof(DataLookline));
            SerialTTL.flush();
        #endif//USE_SERIAL2
        if(MeshConfig.role == "Broker"){
            DataForPC();
        }
    }
    if (dataLen == sizeof(bind)) {
        bind receivedBind;
        memcpy(&receivedBind, data, sizeof(bind));

        if (strncmp(receivedBind.cmd, "BIND", sizeof(receivedBind.cmd)) == 0) {
            if (MeshConfig.debug) Serial.println("Bind request received.");
            saveMacToMacList(senderMac, receivedBind.id);

            // Send acknowledgment back to the sender
            String ackMessage = "{\"id\":" + String(MeshConfig.id) + ",\"cmd\":\"BindAck\"}";
            esp_now_send(senderMac, (const uint8_t *)ackMessage.c_str(), ackMessage.length());
        }
    }
}

#endif//ESP32_RISCV
#ifdef ESP32
#else//ESP8266
void transmissionComplete(uint8_t *receiver_mac, uint8_t transmissionStatus) {
    if(transmissionStatus == 0) {
        if (MeshConfig.debug) Serial.println("Data sent successfully");
    } else {
        if (MeshConfig.debug) Serial.print("Error code: ");
        if (MeshConfig.debug) Serial.println(transmissionStatus);
    }
}
void dataReceived(uint8_t *senderMac, uint8_t *data, uint8_t dataLength) {
    char macStr[18];
    dataPacket packet;  

    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", senderMac[0], senderMac[1], senderMac[2], senderMac[3], senderMac[4], senderMac[5]);

    if (MeshConfig.debug) Serial.println();
    if (MeshConfig.debug) Serial.print("ID: 3 | ");
    if (MeshConfig.debug) Serial.print("Received data from: ");
    if (MeshConfig.debug) Serial.println(macStr);
    
    memcpy(&packet, data, sizeof(packet));
    
    if (MeshConfig.debug) Serial.print("ID: ");
    if (MeshConfig.debug) Serial.println(packet.ID);
    if (MeshConfig.debug) Serial.print("netId: ");
    if (MeshConfig.debug) Serial.println(packet.netId);
    if (MeshConfig.debug) Serial.print("data: ");
    for (int i = 0; i < sizeof(packet.data); i++) {
        if (MeshConfig.debug) Serial.print(packet.data[i], HEX);
        if (MeshConfig.debug) Serial.print(" ");
    }
    if (MeshConfig.debug) Serial.println();
}

#endif//ESP32
#ifdef ESP32
void Broker(const String &message)
{
    esp_now_peer_info_t peerInfo = {};
    static const uint8_t defaultBrokerAddress[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    memcpy(&peerInfo.peer_addr, defaultBrokerAddress, 6);
    if (!esp_now_is_peer_exist(defaultBrokerAddress))
    {
        peerInfo.channel = MeshConfig.wifiChannel;
        esp_now_add_peer(&peerInfo);
    }
    esp_err_t result = esp_now_send(defaultBrokerAddress, (const uint8_t *)message.c_str(), message.length());
    if (result == ESP_OK)
    {
        if (MeshConfig.debug) Serial.println("Broker message success");
    }
    else if (result == ESP_ERR_ESPNOW_NOT_INIT)
    {
        if (MeshConfig.debug) Serial.println("Mesh not Init.");
    }
    else if (result == ESP_ERR_ESPNOW_ARG)
    {
        if (MeshConfig.debug) Serial.println("Invalid Argument");
    }
    else if (result == ESP_ERR_ESPNOW_INTERNAL)
    {
        if (MeshConfig.debug) Serial.println("Internal Error");
    }
    else if (result == ESP_ERR_ESPNOW_NO_MEM)
    {
        if (MeshConfig.debug) Serial.println("ESP_ERR_ESPNOW_NO_MEM");
    }
    else if (result == ESP_ERR_ESPNOW_NOT_FOUND)
    {
        if (MeshConfig.debug) Serial.println("Peer not found.");
    }
    else
    {
        if (MeshConfig.debug) Serial.println("Unknown error");
    }
}

#endif//ESP32
unsigned long buttonPressTime = 0;

void startConfigPortal() {
    // WiFi.onEvent(WiFiEvent);
    // Set up the access point for configuration mode
    #ifdef USE_Modbus
    if(wifiMode == "AP"){ 
    #endif//USE_Modbus
        if (MeshConfig.debug) Serial.println("Starting configuration portal...");
        WiFi.disconnect();
        WiFi.mode(WIFI_OFF);
        dnsServer.stop();
        if (MeshConfig.debug) Serial.println("Starting configuration portal...");
        WiFi.mode(WIFI_AP);
        WiFi.softAPConfig(apIP, apIP, subnet);
        String APname = "MeshModule_" + String(MeshConfig.id);
        WiFi.softAP(APname.c_str(), "12345678"); // Replace with your desired SSID and password
        dnsServer.start(DNS_PORT, "*", apIP);
        configMode = true;
        if (MeshConfig.debug) Serial.println("Configuration portal started.");
        setupWebConfig();
        #ifdef USE_Modbus
    }
    #endif//USE_Modbus
}
void checkConfigButton() {
    if(digitalRead(SETUP_BUTTON) == LOW) {
        if(buttonPressTime == 0) {
            buttonPressTime = millis();
        }
        else if(millis() - buttonPressTime > 3000 && millis() - buttonPressTime < 5000) { // Nhấn giữ 3s
            if(!configMode) {
                startConfigPortal();
            }
        }
        else if(millis() - buttonPressTime > 5000) { // Nhấn giữ 5s
            ESP.restart();
        }
    }
    else {
        buttonPressTime = 0;
    }
}

// void WiFiEvent(WiFiEvent_t event)
// {
//     switch (event)
//     {
//     case SYSTEM_EVENT_ETH_START:
//         LOGLN("ETH Started");
//         // set eth hostname here
//         //ETH.setHostname("iotdevice");
//         break;
//     case SYSTEM_EVENT_ETH_CONNECTED:
//         LOGLN("ETH Connected");
//         break;
//     case SYSTEM_EVENT_ETH_GOT_IP:
//         Serial.print("ETH MAC: ");
//         //Serial.print(ETH.macAddress());
//         Serial.print(", IPv4: ");
//         //Serial.print(ETH.localIP());
//         // if (ETH.fullDuplex())
//         // {
//         //     Serial.print(", FULL_DUPLEX");
//         // }
//         Serial.print(", ");
//         // Serial.print(ETH.linkSpeed());
//         LOGLN("Mbps");
//         //eth_connected = true;

//         break;
//     case SYSTEM_EVENT_ETH_DISCONNECTED:
//         LOGLN("ETH Disconnected");
//         //eth_connected = false;
//         break;
//     case SYSTEM_EVENT_ETH_STOP:
//         LOGLN("ETH Stopped");
//         // eth_connected = false;
//         break;

//     case SYSTEM_EVENT_STA_CONNECTED:
//         LOGLN("STA Connected");
//         //SocketConnect = true;
//         //xTimerStart(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi

//         // ap_connected = false;SocketConnect = false;
//         break;

//     case SYSTEM_EVENT_STA_DISCONNECTED:
//         LOGLN("STA Disconnected");
//         // WiFi.disconnect();
//         // WiFi.mode(WIFI_OFF);
//         // WiFi.mode(WIFI_STA);
//         //vTaskResume(TskWIFI);
//         //xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
//         //ap_connected = false;
//         //SocketConnect = false;//ReconnectWifi = true;
//         break;
//     case SYSTEM_EVENT_AP_STADISCONNECTED:
//         LOGLN("AP Disconnected");
//         //ap_connected = false;
//         //SocketConnect = false;
//         break;
//     case SYSTEM_EVENT_AP_STACONNECTED:
//         LOGLN("AP Connected");
//         //ap_connected = true;
//         //SocketConnect = true;
//         break;
//     case SYSTEM_EVENT_AP_START:
//         LOGLN("AP Start");
//         break;
//     case SYSTEM_EVENT_AP_STOP:
//         LOGLN("AP Stop");
//         break;
//     case SYSTEM_EVENT_STA_GOT_IP:
//         Serial.println("WiFi connected");
//         Serial.println("IP address: ");
//         Serial.println(WiFi.localIP());
//         // connectToMqtt();
//     break;
//     default:
//         break;
//     }
// }
///////////////////////////////////////////////////////// Serial TTL -> Mesh /////////////////////////////////////////////////////
void convertPacketToDataLookline(const uint8_t *data, struct_Parameter_messageOld &DataLookline) {
    // Chuyển đổi dữ liệu từ packet.data sang DataLookline
    DataLookline.networkID = packet.netId; // Byte 0-1: networkID (2 bytes)
    DataLookline.nodeID = packet.ID; // Byte 0-1: nodeID (2 bytes)
    DataLookline.PLAN = (data[0] << 8) | data[1]; // Byte 2-3: PLAN (2 bytes)
    DataLookline.RESULT = (data[2] << 8) | data[3]; // Byte 4-5: RESULT (2 bytes)
    DataLookline.state = data[4];   // Byte 6: state
    DataLookline.Mode = data[5];    // Byte 7: Mode
}

uint8_t  incomingData[sizeof(struct dataPacket)];
size_t   received_msg_length;
void receiveDataPacketFromSerial2() {
#ifdef USE_SERIAL1
#define SerialTTL Serial1
#else//USE_SERIAL1
#define SerialTTL Serial2
#endif//USE_SERIAL1
    if (SerialTTL.available()) {//Send data to Serial TTL to Mesh
        received_msg_length = SerialTTL.readBytesUntil('\n', incomingData, sizeof(incomingData));
        if (received_msg_length == sizeof(incomingData)) {  // got a msg from a sensor
            memcpy(&packet, incomingData, sizeof(packet));
            if (MeshConfig.debug) Serial.println("ID: " + String(packet.ID) + " | netId: " + String(packet.netId));
            if (MeshConfig.debug) Serial.println("Data: ");
            // for (int i = 0; i < sizeof(packet.data); i++){
            //     if (MeshConfig.debug) Serial.print(packet.data[i], HEX);
            //     if (MeshConfig.debug) Serial.print(" ");
            // }
            // if (MeshConfig.debug) Serial.println();
            esp_now_peer_info_t peerInfo = {};
            memcpy(&peerInfo.peer_addr, MeshConfig.BrokerAddress, 6);
            if (!esp_now_is_peer_exist(MeshConfig.BrokerAddress)) {
                peerInfo.channel = MeshConfig.wifiChannel;
                esp_now_add_peer(&peerInfo);
            }
                // Giả sử packet chứa dữ liệu
                convertPacketToDataLookline(packet.data, DataLookline);
            if(MeshConfig.debug) {
                // In ra dữ liệu đã chuyển đổi
                Serial.println("DataLookline:");
                Serial.print("Network ID: ");
                Serial.println(DataLookline.networkID);
                Serial.print("Node ID: ");
                Serial.println(DataLookline.nodeID);
                Serial.print("PLAN: ");
                Serial.println(DataLookline.PLAN);
                Serial.print("RESULT: ");
                Serial.println(DataLookline.RESULT);
                Serial.print("State: ");
                Serial.println(DataLookline.state);
                Serial.print("Mode: ");
                Serial.println(DataLookline.Mode);
            }
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
          if(DataLookline.state){State ="1" + String(DataLookline.Mode);}else{State = "0" + String(DataLookline.Mode);}
          String sentData = id + "04" + "18" + StringPlan + StringResult + State;
          esp_err_t result;
            if(MeshConfig.dataVersion == 3){  result = esp_now_send(MeshConfig.BrokerAddress, (uint8_t *) &DataLookline, sizeof(DataLookline));}
            if(MeshConfig.dataVersion == 0){  result = esp_now_send(MeshConfig.BrokerAddress, (const uint8_t *)sentData.c_str(), sentData.length());}
            if (result == ESP_OK)
            {
                if (MeshConfig.debug) Serial.println("DataPacket sent success");
            }
            else if (result == ESP_ERR_ESPNOW_NOT_INIT)
            {
                if (MeshConfig.debug) Serial.println("Mesh not Init.");
            }
            else if (result == ESP_ERR_ESPNOW_ARG)
            {
                if (MeshConfig.debug) Serial.println("Invalid Argument");
            }
            else if (result == ESP_ERR_ESPNOW_INTERNAL)
            {
                if (MeshConfig.debug) Serial.println("Internal Error");
            }
            else if (result == ESP_ERR_ESPNOW_NO_MEM)
            {
                if (MeshConfig.debug) Serial.println("ESP_ERR_ESPNOW_NO_MEM");
            }
            else if (result == ESP_ERR_ESPNOW_NOT_FOUND)
            {
                if (MeshConfig.debug) Serial.println("Peer not found.");
            } else {
                if (MeshConfig.debug) Serial.println("Unknown error.");
            }
        
        }
    }     
}

void TaskModbus(void *pvParameter)
{
    LOG("TaskModbus Run in core ");
    LOGLN(xPortGetCoreID());
    for (;;)
    {

    }
}
void TaskWifiMQTT(void *pvParameter)
{
    LOG("TaskMQTT Run in core ");
    LOGLN(xPortGetCoreID());
    #ifdef USE_Modbus
    wifiMqttConfig.setup();
    #endif//USE_Modbus
    for (;;)
    {
        #ifdef USE_Modbus
        wifiMqttConfig.loop();
        #endif//USE_Modbus
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void setup()
{
    Serial.begin(115200);
    SerialInit();
    #ifndef Gateway
    if (!FileSystem.begin()) {
        if (MeshConfig.debug) Serial.println("Failed to initialize LittleFS. Attempting to format...");
        if (FileSystem.format()) {
            if (MeshConfig.debug) Serial.println("File system formatted successfully. Retrying initialization...");
            if (!FileSystem.begin()) {
                if (MeshConfig.debug) Serial.println("Failed to initialize LittleFS after formatting.");
                return;
            }
        } else {
            if (MeshConfig.debug) Serial.println("Failed to format LittleFS.");
            return;
        }
    }
    loadConfig();
    printConfig();
    pinMode(M0_PIN, OUTPUT);
    pinMode(M1_PIN, OUTPUT);
    pinMode(M0_PIN_1, OUTPUT);
    pinMode(M1_PIN_1, OUTPUT);
    digitalWrite(M0_PIN, LOW);
    digitalWrite(M1_PIN, LOW);
    digitalWrite(M0_PIN_1, LOW);
    digitalWrite(M1_PIN_1, LOW);
    #ifdef USE_Modbus
        // Truyền chuỗi JSON vào hàm ModbusInit
        String  StrConfig = mainModbusConfig.loadModbusConfig(MeshConfig.debug, FileSystem);
        // LOGLN(StrConfig);
        ModbusInit(StrConfig);
    #endif//USE_Modbus
    delay(1000);
    #endif//Gateway


    //Set device in STA mode to begin with
    WiFi.mode(WIFI_STA);
    if (MeshConfig.debug) Serial.println("Start Mesh");
    // Output my MAC address - useful for later
    if (MeshConfig.debug) Serial.print("Device Address: ");
    if (MeshConfig.debug) Serial.println(WiFi.macAddress());
    // shut down wifi
    WiFi.disconnect();
    // startup ESP Now
    
#ifdef ESP32
    if (esp_now_init() == ESP_OK)
    {
        if (MeshConfig.debug) Serial.println("Mesh Init Success");
        esp_now_register_recv_cb(receiveCallback);
        esp_now_register_send_cb(sentCallback);
        esp_now_peer_info_t peerInfo = {};
        memcpy(&peerInfo.peer_addr, MeshConfig.BrokerAddress, 6);
        peerInfo.channel = MeshConfig.wifiChannel;
        peerInfo.encrypt = false;
        if (!esp_now_is_peer_exist(MeshConfig.BrokerAddress))
        {
            esp_now_add_peer(&peerInfo);
        }


    }
    else
    {
        if (MeshConfig.debug) Serial.println("Mesh Init Failed");
        delay(3000);
        ESP.restart();
    }

#else//ESP8266
    WiFi.disconnect();        // we do not want to connect to a WiFi network

    if(esp_now_init() != 0) {
        if (MeshConfig.debug) Serial.println("ESP-NOW initialization failed");
        return;
    }

    esp_now_set_self_role(MY_ROLE);   
    esp_now_register_send_cb(transmissionComplete);         // this function will get called once all data is sent
    esp_now_register_recv_cb(dataReceived);               // this function will get called whenever we receive data
    esp_now_add_peer(MeshConfig.BrokerAddress, RECEIVER_ROLE, MeshConfig.wifiChannel, NULL, 0);

#endif//ESP32
    // use the built in button
    pinMode(SETUP_BUTTON, INPUT_PULLUP);
    pinMode(LED_STT, OUTPUT);
    digitalWrite(LED_STT, LOW); // turn off the LED
    #ifdef USE_Modbus
    mainModbusSetting = JSON.parse(mainModbusConfig.loadModbusConfig(MeshConfig.debug, FileSystem));
    #endif//USE_Modbus
    // ModbusLoop();TaskMain
    // xTaskCreatePinnedToCore(TaskModbus, "TaskModbusRTU", 5000, NULL, 2, NULL, 1);
    #ifdef USE_Modbus
    xTaskCreatePinnedToCore(TaskWifiMQTT, "TaskMain", 5000, NULL, 1, &TaskMQTT, 1);
    #endif//USE_Modbus
}
long timeCount = 0;
void loop()
{
    #ifdef USE_Modbus
        ModbusLoop(3000);   
    #endif//USE_Modbus
    processSerialInput();// Serial input cmnd processing
    checkConfigButton();// Button Setup
    if(MeshConfig.dataVersion == 3 || MeshConfig.dataVersion == 0){receiveDataPacketFromSerial2();}//Data version3: Send recive data from Serial2 to Mesh
    if(MeshConfig.role == "Broker"){receiveDataPacketFromSerial2();}//Data version3: Send recive data from Serial2 to Mesh

    static long ModbusCurrentMillis = millis();
    if(millis() -  ModbusCurrentMillis >= 3000) {
        ModbusCurrentMillis = millis();
        #ifdef USE_Modbus
            if (MeshConfig.debug){
                LOGLN("Registers Value:");
                for (int i = 0; i < (int)mainModbusSetting["Value"].length(); i++) {
                if((int)mainModbusSetting["Type"][i] == 0){
                    LOG(String((int)mainModbusSetting["Value"][i]) + " [" + String(mainModbusCom.GetCoilReg((int)mainModbusSetting["Value"][i])) + "] | ");
                }
                if((int)mainModbusSetting["Type"][i] == 1){
                    LOG(String((int)mainModbusSetting["Value"][i]) + " [" + String(mainModbusCom.GetHoldingReg((int)mainModbusSetting["Value"][i])) + "] | ");
                }
                if((int)mainModbusSetting["Type"][i] == 2){
                    uint32_t dwordValue = mainModbusCom.DWORD(mainModbusCom.GetHoldingReg((int)mainModbusSetting["Value"][i]), mainModbusCom.GetHoldingReg(((int)mainModbusSetting["Value"][i]) + 1));
                    LOG(String((int)mainModbusSetting["Value"][i]) + " [" + String(dwordValue) + "] | ");
                }
                if((int)mainModbusSetting["Type"][i] == 3){
                    LOG(String((int)mainModbusSetting["Value"][i]) + " [" + String((float)(mainModbusCom.DWORD(mainModbusCom.GetHoldingReg((int)mainModbusSetting["Value"][i]), mainModbusCom.GetHoldingReg(((int)mainModbusSetting["Value"][i])+ 1)))) + "] | ");
                }
                }
                LOGLN();
            }
        #endif//USE_Modbus
    }
#ifdef ESP32
    if(configMode){
        dnsServer.processNextRequest();
        static long timeCount = 0;
        if (millis() - timeCount >= 200) {
            timeCount = millis();
            digitalWrite(LED_STT, !digitalRead(LED_STT)); // Toggle LED state
        }
    } else {
        digitalWrite(LED_STT, LOW); // Turn off the LED when not in config mode
    }
#else 
    int result = esp_now_send(MeshConfig.BrokerAddress, (uint8_t *) &packet, sizeof(packet));
    if (result == 0)
    {
        if (MeshConfig.debug) Serial.println("Broker message success");
    }
    else
    {
        if (MeshConfig.debug) Serial.println("Unknown error");
    }
    
#endif//ESP32
    // delay(3000);
}