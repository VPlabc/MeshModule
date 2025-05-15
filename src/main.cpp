#include "main.h"
#include <ArduinoJson.h>

#include <Arduino.h>
int8_t BUZZ = -1;
int8_t SETUP_BUTTON = -1;
int8_t LED_STT = -1;
int8_t I2C_SDA = -1;
int8_t I2C_SCL = -1;
int8_t Y8 = -1;
int8_t Y9 = -1;
int8_t DA0 = -1;
int8_t DA1 = -1;
int8_t InPut0 = -1;
int8_t InPut1 = -1;
int8_t InPut2 = -1;
int8_t InPut3 = -1;
int8_t InPut4 = -1;
int8_t Ser_1RX = -1;
int8_t Ser_1TX = -1;
int8_t M0_PIN = -1;
int8_t M1_PIN = -1;
int8_t ETH_POWER_PIN_ALTERNATIVE = -1;
int8_t ETH_MDC_PIN = -1;
int8_t ETH_MDIO_PIN = -1;
int8_t Ser_2RX = -1;
int8_t Ser_2TX = -1;
// int timezone = 7;

//#define TCP_ETH
#define RTU_RS485


#ifdef USE_Modbus
#include "TskModbus.h"
ModbusConfig mainModbusConfig;
TaskHandle_t TaskMQTT;
TaskHandle_t TaskModbus;

#ifdef RTU_RS485
#include "Modbus_RTU.h"
Modbus_Prog iMagModbusCom;
#endif//RTU_RS485
  #ifdef TCP_ETH
  #include "Network/Modbus_TCP.h"
  Modbus_TCP_Prog iMagModbusCom;
  #endif//TCP_ETH

#include "WebInterface.h"
WebinterFace mainwebInterface;
#ifdef USE_Modbus
#include "TskMQTT.h"
WifiMqttConfig MQTTwifiConfig; // Ensure this is declared only if USE_Modbus is defined
#define RTC_Onl
#include "RTC_Online.h"
RTCTimeOnline rtcTimeOnl; // Ensure this is declared only if USE_Modbus is defined
#include "LoRa.h"
LoRaFunction mainLoRa; // Ensure this is declared only if USE_Modbus is defined
// bool LoRanIit = false; // Added missing variable
#endif// USE_Modbus
#ifdef USE_Modbus
Modbus_Prog mainModbusCom;
#endif//USE_Modbus   
#include "Wifi_conf.h"
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
    byte boardModel;
    bool LoRaEnable;
} MeshConfig;


struct __attribute__((packed)) dataPacket {
    int ID;
    int netId;
    uint8_t data[200];
    uint8_t dataSize;
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

  
// #include "DataMapping.h"
#include "StoreData.h"
// Định nghĩa thực tế của nodeDataMaps
std::map<int, NodeDatas> nodeDataMaps;

JSONVar mainModbusSetting;
  
#include <DNSServer.h>
DNSServer dnsServer;
const byte DNS_PORT = 53;
IPAddress apIP(192,168,4,1);
IPAddress subnet(255, 255, 255, 0);
bool configMode = false;

//BUZZ | SETUP_BUTTON | LED_STT | I2C_SDA | I2C_SCL | Y8 | Y9 | DA0 | DA1 | InPut0 | InPut1 | InPut2 | InPut3 | InPut4 | Ser_1RX | Ser_1TX | M0_PIN | M1_PIN | ETH_POWER_PIN_ALTERNATIVE | ETH_MDC_PIN | ETH_MDIO_PIN | Ser_2RX | Ser_2TX
/*board RS485-Wifi*/
int8_t Board0[] = {/*BUZZ*/ -1, /*SETUP_BUTTON*/ 26, /*LED_STT*/ 25, /*I2C_SDA*/ 21, /*I2C_SCL*/ 22, /*Y8*/ -1, /*Y9*/ -1, /*DA0*/ -1, /*DA1*/ -1, /*InPut0*/ -1, /*InPut1*/ -1, /*InPut2*/ -1, /*InPut3*/ -1, /*InPut4*/ -1, /*Ser_1RX*/ 15, /*Ser_1TX*/  2, /*M0_PIN*/ -1, /*M1_PIN*/ -1, /*ETH_POWER_PIN_ALTERNATIVE*/ 14, /*ETH_MDC_PIN*/ 23, /*ETH_MDIO_PIN*/ 18, /*Ser_2RX*/ 16, /*Ser_2TX*/ 17};
/*board 410WER-Wifi*/
int8_t Board1[] = {/*BUZZ*/ 12, /*SETUP_BUTTON*/ 13, /*LED_STT*/  4, /*I2C_SDA*/ 32, /*I2C_SCL*/ 33, /*Y8*/  4, /*Y9*/  5, /*DA0*/ 39, /*DA1*/ 36, /*InPut0*/  0, /*InPut1*/ 34, /*InPut2*/ 35, /*InPut3*/ 15, /*InPut4*/ -1, /*Ser_1RX*/ 33, /*Ser_1TX*/ 32, /*M0_PIN*/ -1, /*M1_PIN*/ -1, /*ETH_POWER_PIN_ALTERNATIVE*/ 14, /*ETH_MDC_PIN*/ 23, /*ETH_MDIO_PIN*/ 18, /*Ser_2RX*/ 16, /*Ser_2TX*/ 17};
/*board LkLineGW*/
int8_t Board2[] = {/*BUZZ*/ -1, /*SETUP_BUTTON*/  0, /*LED_STT*/ 27, /*I2C_SDA*/ 21, /*I2C_SCL*/ 22, /*Y8*/ -1, /*Y9*/ -1, /*DA0*/ -1, /*DA1*/ -1, /*InPut0*/ -1, /*InPut1*/ -1, /*InPut2*/ -1, /*InPut3*/ -1, /*InPut4*/ -1, /*Ser_1RX*/ -1, /*Ser_1TX*/ -1, /*M0_PIN*/ -1, /*M1_PIN*/ -1, /*ETH_POWER_PIN_ALTERNATIVE*/ 14, /*ETH_MDC_PIN*/ 23, /*ETH_MDIO_PIN*/ 18, /*Ser_2RX*/ 16, /*Ser_2TX*/ 17};
/*board LkLineNode*/
int8_t Board3[] = {/*BUZZ*/ -1, /*SETUP_BUTTON*/ 26, /*LED_STT*/ 25, /*I2C_SDA*/ 21, /*I2C_SCL*/ 22, /*Y8*/ -1, /*Y9*/ -1, /*DA0*/ -1, /*DA1*/ -1, /*InPut0*/ -1, /*InPut1*/ -1, /*InPut2*/ -1, /*InPut3*/ -1, /*InPut4*/ -1, /*Ser_1RX*/ 15, /*Ser_1TX*/  2, /*M0_PIN*/ -1, /*M1_PIN*/ -1, /*ETH_POWER_PIN_ALTERNATIVE*/ -1, /*ETH_MDC_PIN*/ 16, /*ETH_MDIO_PIN*/ 17, /*Ser_2RX*/ 16, /*Ser_2TX*/ 17};
/*board 0404WER*/
int8_t Board4[] = {/*BUZZ*/ -1, /*SETUP_BUTTON*/  0, /*LED_STT*/ 13, /*I2C_SDA*/ 21, /*I2C_SCL*/ 22, /*Y8*/ -1, /*Y9*/ -1, /*DA0*/ 36, /*DA1*/ 39, /*InPut0*/  0, /*InPut1*/ 33, /*InPut2*/ 32, /*InPut3*/ 34, /*InPut4*/ 35, /*Ser_1RX*/ 35, /*Ser_1TX*/  2, /*M0_PIN*/ 16, /*M1_PIN*/ 17, /*ETH_POWER_PIN_ALTERNATIVE*/ 16, /*ETH_MDC_PIN*/ 23, /*ETH_MDIO_PIN*/ 18, /*Ser_2RX*/ 16, /*Ser_2TX*/ 17};

void set_Pinout(uint8_t BoardModel){
    int8_t* selectedBoard;
    switch (BoardModel) {
        case 0:
        selectedBoard = Board0;
        break;
        case 1:
        selectedBoard = Board1;
        break;
        case 2:
        selectedBoard = Board2;
        break;
        case 3:
        selectedBoard = Board3;
        break;
        default:
        selectedBoard = nullptr; // Handle invalid BoardModel
        break;
    }
    BUZZ = selectedBoard[0];
    SETUP_BUTTON = selectedBoard[1];
    LED_STT = selectedBoard[2];
    I2C_SDA = selectedBoard[3];
    I2C_SCL = selectedBoard[4];
    Y8 = selectedBoard[5];
    Y9 = selectedBoard[6];
    DA0 = selectedBoard[7];
    DA1 = selectedBoard[8];
    InPut0 = selectedBoard[9];
    InPut1 = selectedBoard[10];
    InPut2 = selectedBoard[11];
    InPut3 = selectedBoard[12];
    InPut4 = selectedBoard[13];
    Ser_1RX = selectedBoard[14];
    Ser_1TX = selectedBoard[15];
    M0_PIN = selectedBoard[16];
    M1_PIN = selectedBoard[17];
    ETH_POWER_PIN_ALTERNATIVE = selectedBoard[18];
    ETH_MDC_PIN = selectedBoard[19];
    ETH_MDIO_PIN = selectedBoard[20];
    Ser_2RX = selectedBoard[21];
    Ser_2TX = selectedBoard[22];
}

void convertDataPacketToDataLookline(const dataPacket &packet, struct_Parameter_messageOld &dataLookline);
  

  void MainLoop();

void SupendTask(){
    vTaskSuspend(TaskModbus);
}
void ResumeTask(){
    vTaskResume(TaskModbus);
}
void ModbusInit(String ModbusParameter)
{
    #ifdef RTU_RS485
    iMagModbusCom.modbus_setup(ModbusParameter, Ser_2RX, Ser_2TX);
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

void convertDataPacketToDataLookline(const dataPacket &packet, struct_Parameter_messageOld &dataLookline) {
    dataLookline.PLAN = (packet.data[0] << 8) | packet.data[1];
    dataLookline.RESULT = (packet.data[2] << 8) | packet.data[3];
    dataLookline.state = packet.data[4];
    dataLookline.Mode = packet.data[5];
}


void SerialInit() {
    #ifdef USE_SERIAL1
    Serial1.begin(Se_BAUD_RATE, SERIAL_8N1, Ser_1RX, Ser_1TX);
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
        MeshConfig.LoRaEnable = false; //Default LoRa disabled
        DynamicJsonDocument doc(512);
        MeshConfig.macSlaves = doc.createNestedArray("macSlaves");
        MeshConfig.dataVersion = 0; // Default dataVersion
        MeshConfig.boardModel = 1;
        MeshConfig.LoRaEnable = 0;
        saveConfig(); // Save the configuration
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
    MeshConfig.boardModel = doc["boardModel"];
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
    MeshConfig.LoRaEnable = doc["loraEnb"] | false;
    file.close();
    set_Pinout(MeshConfig.boardModel);
    if (MeshConfig.debug) Serial.println("Config loaded.");
    if (MeshConfig.LoRaEnable) {
        mainLoRa.LoRaSetup(MeshConfig.id);
    } else {
        Serial.println("LoRa is disabled.");
    }
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
    doc["boardModel"] = MeshConfig.boardModel; //Save board model
    doc["LoRaEnable"] = MeshConfig.LoRaEnable; // Save LoRaEnable
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
    static const uint8_t defaultBrokerAddress[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    esp_now_peer_info_t peerInfo = {};
    memcpy(&peerInfo.peer_addr, defaultBrokerAddress, 6);
    if (!esp_now_is_peer_exist(defaultBrokerAddress)) {
        peerInfo.channel = MeshConfig.wifiChannel;
        esp_now_add_peer(&peerInfo);
    }

    esp_err_t result = esp_now_send(defaultBrokerAddress, (uint8_t*)&bindRequest, sizeof(bindRequest));
    if (result == ESP_OK) {
        if (MeshConfig.debug) Serial.println("Bind request sent successfully.");
    } else {
        if (MeshConfig.debug) Serial.println("Failed to send bind request.");
    }
}
/// Serial Input ///////////
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

                if (doc.containsKey("boardModel")) {
                    MeshConfig.boardModel = doc["boardModel"];
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
    Serial.print("Board Model: ");
    switch (MeshConfig.boardModel) {
        case 0:
            Serial.println("Board ModRTUMesh");
            break;
        case 1:
            Serial.println("Board 410WER");
            break;
        case 2:
            Serial.println("Board LklineGw");
            break;
        case 3:
            Serial.println("Board LklineNode");
            break;
        default:
            Serial.println("Unknown Board Model");
            break;
    }
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
    static int totalPacketsSent = 0;
    static int successfulPackets = 0;
// callback when data is sent
void sentCallback(const uint8_t *macAddr, esp_now_send_status_t status) 
{
    if (MeshConfig.debug) Serial.print("Last Packet Send Status: ");
    if (MeshConfig.debug) Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
    // status == ESP_NOW_SEND_SUCCESS ? digitalWrite(BUZZ, LOW) :  digitalWrite(BUZZ, HIGH);delay(10);digitalWrite(BUZZ, LOW);


    totalPacketsSent++;
    if (status == ESP_NOW_SEND_SUCCESS) {
        successfulPackets++;
    }

    float successRate = (totalPacketsSent > 0) ? (successfulPackets * 100.0 / totalPacketsSent) : 0.0;

    if (MeshConfig.debug) {
        Serial.print("Total Packets Sent: ");
        Serial.print(totalPacketsSent);
        Serial.print(" | Successful Packets: ");
        Serial.print(successfulPackets);
        Serial.print(" | Success Rate: ");
        Serial.print(successRate);
        Serial.println("%");
    }
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
#include <queue>




std::queue<dataPacket> dataQueue;

void receiveCallback(const uint8_t *senderMac, const uint8_t *data, int dataLen)
{
    StaticJsonDocument<3500> jsonDoc;
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", senderMac[0], senderMac[1], senderMac[2], senderMac[3], senderMac[4], senderMac[5]);
    if(dataLen == sizeof(dataPacket)) {
        digitalWrite(LED_STT, !digitalRead(LED_STT)); // Toggle LED state
        delay(100);
        digitalWrite(LED_STT, !digitalRead(LED_STT)); // Toggle LED state
        if (MeshConfig.debug) Serial.println("DataPacket received from Mesh (dataVersion = 1):");
        if (MeshConfig.debug) Serial.print("ID: " + String(MeshConfig.id) + " | ");
        if (MeshConfig.debug) Serial.print("Received data from: ");
        if (MeshConfig.debug) Serial.println(macStr);
        if (MeshConfig.debug) Serial.println("RSSI: " + String(rssi_display));
        

        memcpy(&packet, data, sizeof(packet));
        dataQueue.push(packet); // Lưu vào hàng đợi
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
            if (MeshConfig.role == "Repeater" || MeshConfig.role == "Node") {
                int result = esp_now_send(MeshConfig.BrokerAddress, (uint8_t *)&packet, sizeof(packet));
                if (result == ESP_OK) {
                    digitalWrite(BUZZ, HIGH);delay(2);digitalWrite(BUZZ, LOW);
                    if (MeshConfig.debug) Serial.println("Repeater forwarded data to Broker successfully.");
                } else {
                    if (MeshConfig.debug) Serial.println("Repeater failed to forward data to Broker.");
                }
            }
            updateNodeData(packet, macStr); // Save the received data along with the sender's MAC address
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
void processQueue() {
    while (!dataQueue.empty()) {
        dataPacket packet = dataQueue.front(); // Lấy phần tử đầu tiên
        dataQueue.pop(); // Xóa phần tử đầu tiên
        // Xử lý dữ liệu trong packet
        
        // updateNodeDataWithMapping(packet); // Cập nhật dữ liệu node
        calculateSuccessRates(); // Tính toán tỷ lệ thành công
        if (MeshConfig.debug) {
            Serial.print("Processing packet ID: ");
            Serial.println(packet.ID);
        }
        #ifdef USE_Modbus
        // mainModbusCom.ModbusGetData(packet.data, sizeof(packet.data));
        // if (MeshConfig.debug) Serial.println("DataPacket received dataVersion = 2):");   
        // if (MeshConfig.debug) Serial.print("ID: " + String(packet.ID) + " | ");
        // if (MeshConfig.debug) Serial.print("netId: " + String(packet.netId));
        // if (MeshConfig.debug) Serial.println();
        // if (MeshConfig.debug) Serial.print("Data: ");
        uint8_t dataIndex = 0; // Index to track position in packet.data
        for (int i = 0; i < (int)mainModbusSetting["Value"].length(); i++) {
            if ((int)mainModbusSetting["Type"][i] == 0) {
                // Type 0: Coil Register
                if (dataIndex < sizeof(packet.data)) {
                    // Serial.println("coils["+ String((int)mainModbusSetting["Value"][i]) + "]:" + String(packet.data[dataIndex++]));
                    if(packet.data[dataIndex++] > 1){mainModbusCom.SetCoilReg((int)mainModbusSetting["Value"][i],1);}else{mainModbusCom.SetCoilReg((int)mainModbusSetting["Value"][i],0);}
                }
            } else if ((int)mainModbusSetting["Type"][i] == 1) {
                // Type 1: Holding Register
                if (dataIndex + 1 < sizeof(packet.data)) {
                    uint16_t value = packet.data[dataIndex] << 8 | packet.data[dataIndex + 1];dataIndex = dataIndex + 2;
                    // Serial.println("holdingReg[" + String((int)mainModbusSetting["Value"][i]) + "]:" + String(value));
                    mainModbusCom.SetHoldingReg((int)mainModbusSetting["Value"][i], value);
                }
            } else if ((int)mainModbusSetting["Type"][i] == 2) {
                // Type 2: DWORD
                if (dataIndex + 3 < sizeof(packet.data)) {
                    uint32_t dwordValue = mainModbusCom.DWORD(
                        packet.data[dataIndex] << 8 | packet.data[dataIndex + 1],
                        packet.data[dataIndex + 2] << 8 | packet.data[dataIndex + 3]); dataIndex = dataIndex + 4;
                    // Serial.println("DWORD[" + String((int)mainModbusSetting["Value"][i]) + "]:" + String(dwordValue));
                    mainModbusCom.SetHoldingReg((int)mainModbusSetting["Value"][i], packet.data[dataIndex] << 8 | packet.data[dataIndex + 1]);
                    mainModbusCom.SetHoldingReg((int)mainModbusSetting["Value"][i+1], packet.data[dataIndex + 2] << 8 | packet.data[dataIndex + 3]);
                }
            } else if ((int)mainModbusSetting["Type"][i] == 3) {
                // Type 3: Float
                if (dataIndex + 3 < sizeof(packet.data)) {
                    float floatValue = (float)(mainModbusCom.DWORD(
                        packet.data[dataIndex] << 8 | packet.data[dataIndex + 1],
                        packet.data[dataIndex + 2] << 8 | packet.data[dataIndex + 3])) / 1000.0;dataIndex = dataIndex + 4;
                    // Serial.println("Float[" + String((int)mainModbusSetting["Value"][i]) + "]:" + String(floatValue));
                    mainModbusCom.SetHoldingReg((int)mainModbusSetting["Value"][i], packet.data[dataIndex] << 8 | packet.data[dataIndex + 1]);
                    mainModbusCom.SetHoldingReg((int)mainModbusSetting["Value"][i+1], packet.data[dataIndex + 2] << 8 | packet.data[dataIndex + 3]);
                }
            }
        }
    #endif//USE_Modbus
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

    if (MeshConfig.debug) Serial.println("Starting configuration portal...");
    digitalWrite(BUZZ, HIGH);delay(5);digitalWrite(BUZZ, LOW);
    digitalWrite(BUZZ, HIGH);delay(5);digitalWrite(BUZZ, LOW);
 
    // Dừng ESP-NOW nếu đang chạy
    if (esp_now_deinit() == ESP_OK) {
        if (MeshConfig.debug) Serial.println("Mesh deInit Success");
    } else {
        if (MeshConfig.debug) Serial.println("Mesh deInit Failed");
    }

    // Ngắt kết nối WiFi và đặt chế độ WiFi về OFF
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    dnsServer.stop();
    esp_wifi_set_promiscuous(false);// Tắt chế độ Promiscuous
    esp_wifi_set_max_tx_power(78); // 78 tương ứng với 20 dBm
    esp_wifi_set_ps(WIFI_PS_NONE); // Disable power save mode
    WiFi.mode(WIFI_AP);
    // Đặt lại giao thức WiFi về mặc định
    esp_err_t result = esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G);
    if (result == ESP_OK) {
        if (MeshConfig.debug) Serial.println("WiFi protocol set successfully.");
    } else {
        if (MeshConfig.debug) Serial.print("Failed to set WiFi protocol. Error: ");
        if (MeshConfig.debug) Serial.println(esp_err_to_name(result));
    }
    current_wifi_interface = WIFI_IF_AP; // Sử dụng giao diện Wi-Fi Station
        WiFi.mode(WIFI_AP);
        WiFi.softAPConfig(apIP, apIP, subnet);
        String APname = "MeshModule[" + String(MeshConfig.id) + ']';
        WiFi.softAP(APname.c_str(), "12345678"); // Replace with your desired SSID and password
        dnsServer.start(DNS_PORT, "*", apIP);
        check_protocol();
        configMode = true;
        if (MeshConfig.debug) Serial.println("Configuration portal started.");
        mainwebInterface.setupWebConfig();
        #ifdef USE_Modbus
    // }
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

void TskModbus(void *pvParameter)
{
    LOG("TaskModbus Run in core ");
    LOGLN(xPortGetCoreID());
    
    for (;;)
    {
        if(!configMode){
        #ifdef USE_Modbus
            ModbusLoop(3000);  
        #endif//USE_Modbus
            MainLoop();
        } 
        if(configMode){
            dnsServer.processNextRequest();
            static long timeCount = 0;
            if (millis() - timeCount >= 100) {
                timeCount = millis();
                digitalWrite(LED_STT, !digitalRead(LED_STT)); // Toggle LED state
            }
        } 
        checkConfigButton();// Button Setup
        processSerialInput();// Serial input cmnd processing
    }
}
void TaskWifiMQTT(void *pvParameter)
{
    LOG("TaskMQTT Run in core ");
    LOGLN(xPortGetCoreID());
    #ifdef USE_Modbus
    #endif//USE_Modbus
    for (;;)
    {
        #ifdef USE_Modbus
        MQTTwifiConfig.loop();
        #endif//USE_Modbus
    }
}
bool EthernetAvilable = false;
int getIpBlock(int index, String str)
{
    char separator = '.';
    int found = 0;
    int strIndex[] = {0, -1};
    int maxIndex = str.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++)
    {
        if (str.charAt(i) == separator || i == maxIndex)
        {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i + 1 : i;
        }
    }
    return found > index ? str.substring(strIndex[0], strIndex[1]).toInt() : 0;
}
IPAddress str2IP(String str)
{
    IPAddress ret(getIpBlock(0, str), getIpBlock(1, str), getIpBlock(2, str), getIpBlock(3, str));
    return ret;
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
    if(mqttEnable){
        MQTTwifiConfig.setup();
    }
    if(MeshConfig.role == "Broker"){
        // loadDataMapping();
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////// Ethernet /////////////////////////////////////////////////////
    if(ETH_MDC_PIN == -1 && ETH_MDIO_PIN == -1 && ETH_POWER_PIN_ALTERNATIVE == -1){
        Serial.println(" Pin ETH_MDC_PIN " + String(ETH_MDC_PIN) + " \n Pin ETH_MDIO_PIN " + String(ETH_MDIO_PIN) + " \n Pin ETH_POWER_PIN_ALTERNATIVE " + String(ETH_POWER_PIN_ALTERNATIVE));
        pinMode(ETH_POWER_PIN_ALTERNATIVE, OUTPUT);
        digitalWrite(ETH_POWER_PIN_ALTERNATIVE, HIGH);
        if(ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE)){
        //   ETH.config(str2IP(SettingData[6]), str2IP(SettingData[7]), str2IP(SettingData[8]), str2IP(SettingData[9]), str2IP(SettingData[10]));
          EthernetAvilable = true;
        }else{
          LOGLN("Failed to init Ethernet");
          EthernetAvilable = false;
        }
    }
    else
    {
        LOGLN("Ethernet not available");
        EthernetAvilable = false;
    }

    
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    #ifdef USE_Modbus
        String  StrConfig = mainModbusConfig.loadModbusConfig(MeshConfig.debug, FileSystem);
        mainModbusSetting = JSON.parse(StrConfig);
        ModbusInit(StrConfig);
        StrConfig.clear();
    #endif//USE_Modbus
    delay(1000);
    #endif//Gateway
    if(mqttEnable && wifiMode == "STA"){
        esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N );
        while (WiFi.status() != WL_CONNECTED){
            static long timeCount = 0;
            static long timeCountConnect = 0;
            if (millis() - timeCount >= 1000) {
                timeCount = millis();
                digitalWrite(LED_STT, !digitalRead(LED_STT)); // Toggle LED state
                if (MeshConfig.debug) Serial.println("Waitt connecting to WiFi...");
                timeCountConnect++;
                if(timeCountConnect >= 10){
                    if (MeshConfig.debug) Serial.println("Connecting to WiFi failed.");
                    mqttEnable = false;
                    break;
                }
            }
        }
        if(WiFi.status() == WL_CONNECTED){
            // MeshConfig.wifiChannel = WiFi.channel();
            // if (MeshConfig.debug) Serial.println("WiFi channel set to: " + String(MeshConfig.wifiChannel));
            esp_wifi_set_promiscuous_rx_cb(&promiscuous_rx_cb);
        } else {
            if (MeshConfig.debug) Serial.println("WiFi not connected. Retrying...");
            WiFi.begin(); // Retry WiFi connection
            delay(5000);  // Wait for connection
            if (WiFi.status() != WL_CONNECTED) {
                if (MeshConfig.debug) Serial.println("Failed to connect to WiFi. Check your configuration.");
            }
        }
    }
    else{
        WiFi.disconnect();
        WiFi.mode(WIFI_STA);
        WiFi.disconnect(); // Ngắt kết nối WiFi để đặt lại kênh
        esp_wifi_set_channel(MeshConfig.wifiChannel, WIFI_SECOND_CHAN_NONE); // Đặt kênh WiFi
        WiFi.mode(WIFI_STA);
        esp_wifi_set_promiscuous_rx_cb(&promiscuous_rx_cb);
        // Kích hoạt chế độ Long Range
        
        esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N );
        // esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR);
    }
        esp_wifi_set_promiscuous(false);
        current_wifi_interface = WIFI_IF_STA; // Sử dụng giao diện Wi-Fi Station
        //Set device in STA mode to begin with
        // 
        if (MeshConfig.debug) Serial.println("Start Mesh");
        // Output my MAC address - useful for later
        if (MeshConfig.debug) Serial.print("Device Address: ");
        if (MeshConfig.debug) Serial.println(WiFi.macAddress());
        // shut down wifi
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
        peerInfo.ifidx = WIFI_IF_STA;
        peerInfo.lmk[0] = 0;
        esp_wifi_config_espnow_rate(WIFI_IF_STA, WIFI_PHY_RATE_LORA_250K);
        if (!esp_now_is_peer_exist(MeshConfig.BrokerAddress))
        {
            esp_now_add_peer(&peerInfo);
        }
        // init_wifi_promiscuous_mode();
        // esp_wifi_set_max_tx_power(84); // 84 * 0.25 = 21dBm
        check_protocol();
    }
    else
    {
        if (MeshConfig.debug) Serial.println("Mesh Init Failed");
        delay(3000);
        ESP.restart();
    }
    #ifdef USE_Modbus
    xTaskCreatePinnedToCore(TaskWifiMQTT, "TaskMain", 10000, NULL, 1, &TaskMQTT, 1);
    #endif//USE_Modbus
#else//ESP8266
    WiFi.disconnect();        // we do not want to connect to a WiFi network

    if(esp_now_init() != 0) {
        if (MeshConfig.debug) Serial.println("ESP-NOW initialization failed");
        return;
    }

    esp_now_set_self_role(MY_ROLE);   
    esp_now_register_send_cb(transmissionComplete);         
    esp_now_register_recv_cb(dataReceived);               
    esp_now_add_peer(MeshConfig.BrokerAddress, RECEIVER_ROLE, MeshConfig.wifiChannel, NULL, 0);

#endif//ESP32
    // use the built in button
    pinMode(SETUP_BUTTON, INPUT_PULLUP);
    pinMode(LED_STT, OUTPUT);
    digitalWrite(LED_STT, HIGH); // turn off the LED
    xTaskCreatePinnedToCore(TskModbus, "TaskModbus", 10000, NULL, 2, &TaskModbus, 1);
    Serial.println("Timezone: " + String(timezone));
    rtcTimeOnl.Time_setup(timezone);
    // #ifdef Module_10O4I
    pinMode(BUZZ, OUTPUT);
    pinMode(Y8, OUTPUT);
    pinMode(Y9, OUTPUT);
    digitalWrite(BUZZ, HIGH);delay(50);digitalWrite(BUZZ, LOW);
    // #endif
    // LittleFS.remove("/index.html");
    // setupWebSocket();
}
long timeCount = 0;
bool once1 = false;
void loop()
{
    static long ModbusCurrentMillis = millis();
    if(millis() -  ModbusCurrentMillis >= 3000) {
        ModbusCurrentMillis = millis();
        if (MeshConfig.debug) Serial.println("========================================================================");
        if (MeshConfig.debug) Serial.println("                          Loop Function");
        if (MeshConfig.debug) Serial.println("========================================================================");
        // printNodeDataWithMapping(); // In dữ liệu node
        printNodeData() ;
            String jsonString = createJsonForWebSocket();
            // Serial.println("Data: " + jsonString);
        if(socketConnected){
            mainwebInterface.SendMessageToClient(jsonString);
        }
    }
    if(!configMode && mqttEnable){
        MQTTwifiConfig.loop();
    }
    processQueue();
    if(configMode){
        mainwebInterface.SocketLoop();
    }
}

void MainLoop()
{
    if(once1 == false){
        if (MeshConfig.debug) Serial.println("========================================================================");
        if (MeshConfig.debug) Serial.println("                          Start Main Loop");
        if (MeshConfig.debug) Serial.println("========================================================================");
        once1 = true;
    }
    if(!configMode){
        MQTTwifiConfig.loop();
        // mainLoRa.receiveData();
    if(MeshConfig.dataVersion == 3 || MeshConfig.dataVersion == 0){receiveDataPacketFromSerial2();}//Data version3: Send recive data from Serial2 to Mesh
    if(MeshConfig.role == "Broker"){receiveDataPacketFromSerial2();}//Data version3: Send recive data from Serial2 to Mesh
    static int randomDelay = 3000;
    static long ModbusCurrentMillis = millis();
    if(millis() -  ModbusCurrentMillis >= randomDelay) {randomDelay = random(5000, 7000);
        ModbusCurrentMillis = millis();  
    if(MeshConfig.role == "Node" || MeshConfig.role == "Repeater" ){if(LoRaInit)mainLoRa.LoRaLoop(MeshConfig.netId);}//Data version3: Send recive data from Serial2 to Mesh
        // Serial.println("Loop with dataVersion = " + String(MeshConfig.dataVersion) +" with Mesh role " + MeshConfig.role + " | LoRa Init: " + String(LoRaInit ? "connected":"disconnected"));
        #ifdef USE_MQTT
        if(WiFi.status() == WL_CONNECTED && mqttIsConnected == true) {
            String payloadSent = "";
            String DataAt = "";
            rtcTimeOnl.Time_loop();rtcTimeOnl.GetTime();
            String TimeAt = String(Getyear) + "-" + String(Getmonth) + "-" + String(Getday) + " " + String(Gethour) + ":" + String(Getmin) + ":" + String(Getsec);
            DataAt = createJsonForMqttt();
            payloadSent = "{\"connId\":\"" + String(conId) + "\",\"data\":[" + DataAt + "],\"exeAt\":\"" + String(TimeAt)+"\"}";
            MQTTwifiConfig.MQTTPush(mqttTopic, payloadSent);payloadSent = "";
        }
        #endif//USE_MQTT
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
            if(MeshConfig.dataVersion == 2 && !configMode){
                uint8_t dataIndex = 0; // Index to track position in dataPacket.data
                for (int i = 0; i < (int)mainModbusSetting["Value"].length(); i++) {
                    if ((int)mainModbusSetting["Type"][i] == 0) {
                        // Type 0: Coil Register
                        if (dataIndex < sizeof(packet.data)) {
                            packet.data[dataIndex++] = mainModbusCom.GetCoilReg((int)mainModbusSetting["Value"][i]);
                        }
                    } 
                    
                    else if ((int)mainModbusSetting["Type"][i] == 1) {
                        // Type 1: Holding Register
                        if (dataIndex + 1 < sizeof(packet.data)) {
                            uint16_t value = mainModbusCom.GetHoldingReg((int)mainModbusSetting["Value"][i]);
                            packet.data[dataIndex++] = (value >> 8) & 0xFF; // High byte
                            packet.data[dataIndex++] = value & 0xFF;        // Low byte
                        }
                    } 
                    
                    else if ((int)mainModbusSetting["Type"][i] == 2) {
                        // Type 2: DWORD
                        if (dataIndex + 3 < sizeof(packet.data)) {
                            uint32_t dwordValue = mainModbusCom.DWORD(
                                mainModbusCom.GetHoldingReg((int)mainModbusSetting["Value"][i]),
                                mainModbusCom.GetHoldingReg(((int)mainModbusSetting["Value"][i]) + 1));
                            packet.data[dataIndex++] = (dwordValue >> 24) & 0xFF; // High byte
                            packet.data[dataIndex++] = (dwordValue >> 16) & 0xFF;
                            packet.data[dataIndex++] = (dwordValue >> 8) & 0xFF;
                            packet.data[dataIndex++] = dwordValue & 0xFF;        // Low byte
                        }
                    } 
                    
                    else if ((int)mainModbusSetting["Type"][i] == 3) {
                        // Type 3: Float
                        if (dataIndex + 3 < sizeof(packet.data)) {
                            float floatValue = (float)(mainModbusCom.DWORD(
                                mainModbusCom.GetHoldingReg((int)mainModbusSetting["Value"][i]),
                                mainModbusCom.GetHoldingReg(((int)mainModbusSetting["Value"][i]) + 1)));
                            uint8_t *floatBytes = (uint8_t *)&floatValue;
                            packet.data[dataIndex++] = floatBytes[0];
                            packet.data[dataIndex++] = floatBytes[1];
                            packet.data[dataIndex++] = floatBytes[2];
                            packet.data[dataIndex++] = floatBytes[3];
                        }
                    }
                }
                packet.dataSize = dataIndex;
                packet.ID = MeshConfig.id;
                packet.netId = MeshConfig.netId;
                // Serial.println("DataPacket sent dataVersion = 2) with Mesh role " + MeshConfig.role);
                if(MeshConfig.role == "Node" || MeshConfig.role == "Repeater"){
                    int result = esp_now_send(MeshConfig.BrokerAddress, (uint8_t *)&packet, sizeof(packet));
                    if (result == 0)
                    {
                        if (MeshConfig.debug) Serial.println("Node message success");
                    }
                    else
                    {
                        if (MeshConfig.debug) Serial.println("Unknown error");
                    }
                }
                if(MeshConfig.role == "Broker"){
                    // static const uint8_t defaultBrokerAddress[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
                    // int result = esp_now_send(defaultBrokerAddress, (uint8_t *)&packet, sizeof(packet));
                    // if (result == 0)
                    // {
                    //     if (MeshConfig.debug) Serial.println("Broker message success");
                    // }
                    // else
                    // {
                    //     if (MeshConfig.debug) Serial.println("Unknown error");
                    // }
                }
            }//Data version2: Modbus Reg
        #endif//USE_Modbus
    }
#ifdef ESP32
   
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
    }
// delay(3000);
}

//580 - 6-15 chỉ
