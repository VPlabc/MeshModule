#include "main.h"
#include <ArduinoJson.h>
#include <esp_spiram.h>
#include <Arduino.h>
#ifndef USE_DoorLocker
// int timezone = 7;

//#define TCP_ETH
#define RTU_RS485
#define USE_OTA

struct Config {
    uint8_t BrokerAddress[6];
    int wifiChannel;
    int id;
    int netId;
    char role[16]; // "Broker", "Node", or "Repeater"
    bool debug;  // Enable/disable debug prints
    JsonArray macSlaves;
    int dataVersion; // Data version 0: Lookline v1, 1: Lookline v2, 2: Modbus Register, 3: Serial TTL
    byte boardModel;
    bool LoRaEnable;
    bool BuzzEnable;
    bool MeshEnable; 
}MeshConfig;

struct __attribute__((packed)) dataPacket {
    int ID;
    int netId;
    uint8_t data[200];
    uint8_t dataSize;
};
dataPacket packet;

struct __attribute__((packed)) meshBind {
    int id;
    char cmd[5]; // "BIND" (4 characters + null terminator)
};

#ifdef USE_VEHICLE
#include "./Project/LEDvehicle.h"
#endif // USE_VEHICLE
//////////////////////// Modbus ////////////////////////
#ifdef USE_Modbus
#include "TskModbus.h"
ModbusConfig mainModbusConfig;

TaskHandle_t TaskModbus;
#endif//USE_Modbus

TaskHandle_t TaskEthernet;

TaskHandle_t TaskApp;


#include "WebInterface.h"
WebinterFace mainwebInterface;

#ifdef USE_W5500
#include "W5500Ethernet.h"
#endif//USE_W5500

#ifdef USE_Modbus
#include "Modbus_RTU.h"
Modbus_Prog mainModbusCom;
  #ifdef TCP_ETH
  #include "ModbusTCP.h"
  Modbus_TCP_Prog iMagModbusTCPCom;
  #endif//TCP_ETH

#endif//USE_Modbus

#include "TskMQTT.h"
WifiMqttConfig MQTTwifiConfig; // Ensure this is declared only if USE_Modbus is defined

//////////////////////// Task handles ////////////////////////
TaskHandle_t TaskMQTT;


#define RTC_Onl
#include "RTC_Online.h"
RTCTimeOnline rtcTimeOnl; // Ensure this is declared only if USE_Modbus is defined
#include "LoRa.h"
LoRaFunction mainLoRa; // Ensure this is declared only if USE_Modbus is defined
// bool LoRanIit = false; // Added missing variable

 
#include "Wifi_conf.h"
// #include "Project/lookline.h"



#include "DataTranfer.h"

#include "monitor.h"

#include "PinMapping.h"
// #include "DataMapping.h"
#include "StoreData.h"

std::map<int, NodeDatas> nodeDataMaps;

JSONVar mainModbusSetting;
  

#include <DNSServer.h>
DNSServer dnsServer;
const byte DNS_PORT = 53;
IPAddress apIP(192,168,4,1);
IPAddress subnet(255, 255, 255, 0);
bool configMode = false;


#include <EEPROM.h>
long resetcounter = 0;
// void convertDataPacketToDataLookline(const dataPacket &packet, struct_Parameter_messageOld &dataLookline);
  

void MainLoop();

#ifdef USE_Modbus
void SupendTask(){
    vTaskSuspend(TaskModbus);
}
void ResumeTask(){
    vTaskResume(TaskModbus);
}

void ModbusInit(String ModbusParameter, String DataBlockParameter)
{
    #ifdef RTU_RS485
    mainModbusCom.modbus_setup(ModbusParameter, DataBlockParameter, Ser_2RX, Ser_2TX);
    #endif//RTU_RS485
    #ifdef TCP_ETH
        if((const char*)mainModbusSetting["role"] == "master"){TCP_setup(0);}
        if((const char*)mainModbusSetting["role"] == "slave"){TCP_setup(1);}
    #endif//TCP_ETH
}
void ModbusLoop(int Timeout)
{
    #ifdef RTU_RS485
    mainModbusCom.modbus_loop(Timeout);
    #endif//RTU_RS485
    #ifdef TCP_ETH
        // if((const char*)mainModbusSetting["role"] == "master"){TCP_loop(0, (int)mainModbusSetting["Value"]);}
        // if((const char*)mainModbusSetting["role"] == "slave"){TCP_loop(1);}
    #endif//TCP_ETH
}

#endif//USE_Modbus

#include "Initialize.h"

void waitSerialUSB(unsigned long timeoutMs = 10000) {
    unsigned long start = millis();
    while (!Serial && (millis() - start < timeoutMs)) {
        delay(10);
    }
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

// void convertDataPacketToDataLookline(const dataPacket &packet, struct_Parameter_messageOld &dataLookline) {
//     dataLookline.PLAN = (packet.data[0] << 8) | packet.data[1];
//     dataLookline.RESULT = (packet.data[2] << 8) | packet.data[3];
//     dataLookline.state = packet.data[4];
//     dataLookline.Mode = packet.data[5];
// }


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


#ifndef Gateway
// load mesh config from file
void loadConfig() {
    if (!FileSystem.exists(CONFIG_FILE)) {
        Serial.println("Config file not found, creating default MeshConfig.");
        // Set default values
        static const uint8_t defaultBrokerAddress[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        memcpy(MeshConfig.BrokerAddress, defaultBrokerAddress, 6);
        MeshConfig.wifiChannel = 1;
        MeshConfig.id = 1;
        MeshConfig.netId = 1;
        strlcpy(MeshConfig.role, "Node", sizeof(MeshConfig.role)); // Default role
        MeshConfig.debug = true;  // Default debug enabled
        MeshConfig.LoRaEnable = false; //Default LoRa disabled
        MeshConfig.BuzzEnable = false; //Default Buzz disabled
        DynamicJsonDocument doc(512);
        MeshConfig.macSlaves = doc.createNestedArray("macSlaves");
        MeshConfig.dataVersion = 0; // Default dataVersion
        MeshConfig.boardModel = 1;
        MeshConfig.MeshEnable = true; // Mặc định bật Mesh
        saveConfig(); // Save the configuration
        return;
    }

    File file = FileSystem.open(CONFIG_FILE, "r");
    if (!file) {
        Serial.println("Failed to open config file for reading.");
        return;
    }
    // while (file.available()) {
    //     String line = file.readStringUntil('\n');
    //     Serial.println(line);
    // }
    // file.seek(0); // Đặt lại con trỏ file về đầu để deserializeJson phía dưới vẫn hoạt động
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
    strlcpy(MeshConfig.role, doc["role"] | "Node", sizeof(MeshConfig.role)); // Default to "Node" if not specified
    MeshConfig.debug = doc["debug"] | true; // Default debug enabled
    MeshConfig.macSlaves = doc["macSlaves"].as<JsonArray>();
    MeshConfig.LoRaEnable = doc["loraEnb"] | false;
    MeshConfig.BuzzEnable = doc["buzzEnb"] | false; // Default Buzz disabled
    MeshConfig.MeshEnable = doc["meshEnable"] | true; // Mặc định bật Mesh
    file.close();
    set_Pinout(MeshConfig.boardModel);
    if (MeshConfig.debug) Serial.println("Config loaded.");
    if (MeshConfig.LoRaEnable) {
        mainLoRa.LoRaSetup(MeshConfig.id);
    } else {
        Serial.println("LoRa is disabled.");
    }
}

//save mesh config to file
void saveConfig() {
    File file = FileSystem.open(CONFIG_FILE, "w");
    if (!file) {
        if (MeshConfig.debug) Serial.println("❌  Failed to open config file for writing.");
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
    doc["BuzzEnable"] = MeshConfig.BuzzEnable; // Save BuzzEnable
    doc["meshEnable"] = MeshConfig.MeshEnable;
    JsonArray macArray = doc.createNestedArray("macSlaves");
    for (JsonVariant value : MeshConfig.macSlaves) {
        macArray.add(value.as<const char*>());
    }

    if (serializeJson(doc, file) == 0) {
        if (MeshConfig.debug) Serial.println("❌  Failed to write to config file.");
    } else {
        if (MeshConfig.debug) Serial.println("✅  Config saved.");
    }
    file.close();
}

void sendBindRequest() {
    meshBind bindRequest;
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
    static char inputBuffer[512];
    static size_t inputLen = 0;
    while (Serial.available() > 0) {
        char incomingChar = Serial.read();
        if (incomingChar == '\n') {
            inputBuffer[inputLen] = '\0'; // Null-terminate
            DynamicJsonDocument doc(512);
            DeserializationError error = deserializeJson(doc, inputBuffer);
            if (error) {
                if (MeshConfig.debug) Serial.println("Failed to parse JSON input.");
                inputLen = 0;
                return;
            }

            // Handle binding mode activation
            if (doc.containsKey("cmd") && strcmp(doc["cmd"], "Binding") == 0) {
                if (MeshConfig.debug) Serial.println("Binding mode activated.");
                sendBindRequest();
            }

            else if (doc.containsKey("cmd") && strcmp(doc["cmd"], "PrintMac") == 0) {
                if (MeshConfig.debug) Serial.println("MAC list");
                printMacList();
            }

            #ifdef USE_Modbus
            else if (doc.containsKey("Modbus") && strcmp(doc["Modbus"], "Config") == 0) {
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
                    strlcpy(MeshConfig.role, doc["role"], sizeof(MeshConfig.role));
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
                
            inputLen = 0;
        } else {
            if (inputLen < sizeof(inputBuffer) - 1) {
                inputBuffer[inputLen++] = incomingChar;
            }
        }
    }
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

// #ifdef ESP32    
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

    } else if (dataLen == sizeof(meshBind)) {
        meshBind receivedBind;
        memcpy(&receivedBind, data, sizeof(meshBind));

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
        if(MeshConfig.dataVersion == 4){
            Led_setColor(0x0000ff);
            delay(100);
            Led_setColor(0x00000); // Toggle LED color
        }else{
            digitalWrite(LED_STT, !digitalRead(LED_STT)); // Toggle LED state
            delay(100);
            digitalWrite(LED_STT, !digitalRead(LED_STT)); // Toggle LED state
        }

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
                    if(MeshConfig.BuzzEnable){digitalWrite(BUZZ, HIGH);delay(2);digitalWrite(BUZZ, LOW);}
                    if (MeshConfig.debug) Serial.println("✅  Repeater forwarded data to Broker successfully.");
                } else {
                    if (MeshConfig.debug) Serial.println("❌  Repeater failed to forward data to Broker.");
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
    if (dataLen == sizeof(meshBind)) {
        meshBind receivedBind;
        memcpy(&receivedBind, data, sizeof(meshBind));

        if (strncmp(receivedBind.cmd, "BIND", sizeof(receivedBind.cmd)) == 0) {
            if (MeshConfig.debug) Serial.println("Bind request received.");
            saveMacToMacList(senderMac, receivedBind.id);

            // Send acknowledgment back to the sender
            char ackMessage[64];
            snprintf(ackMessage, sizeof(ackMessage), "{\"id\":%d,\"cmd\":\"BindAck\"}", MeshConfig.id);
            esp_now_send(senderMac, (const uint8_t *)ackMessage, sizeof(ackMessage)-1);
        }
    }
    jsonDoc.clear(); // Clear the JSON document for the next use
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
                    if(packet.data[dataIndex++] > 1){1, mainModbusCom.SetCoilReg((int)mainModbusSetting["id"] , (int)mainModbusSetting["Value"][i],1);}else{mainModbusCom.SetCoilReg((int)mainModbusSetting["id"] , (int)mainModbusSetting["Value"][i],0);}
                }
            } else if ((int)mainModbusSetting["Type"][i] == 1) {
                // Type 1: Holding Register
                if (dataIndex + 1 < sizeof(packet.data)) {
                    uint16_t value = packet.data[dataIndex] << 8 | packet.data[dataIndex + 1];dataIndex = dataIndex + 2;
                    // Serial.println("holdingReg[" + String((int)mainModbusSetting["Value"][i]) + "]:" + String(value));
                    mainModbusCom.SetHoldingReg((int)mainModbusSetting["id"] , (int)mainModbusSetting["Value"][i], value);
                }
            } else if ((int)mainModbusSetting["Type"][i] == 2) {
                // Type 2: DWORD
                if (dataIndex + 3 < sizeof(packet.data)) {
                    uint32_t dwordValue = mainModbusCom.DWORD(
                        packet.data[dataIndex] << 8 | packet.data[dataIndex + 1],
                        packet.data[dataIndex + 2] << 8 | packet.data[dataIndex + 3]); dataIndex = dataIndex + 4;
                    // Serial.println("DWORD[" + String((int)mainModbusSetting["Value"][i]) + "]:" + String(dwordValue));
                    mainModbusCom.SetHoldingReg((int)mainModbusSetting["id"] , (int)mainModbusSetting["Value"][i], packet.data[dataIndex] << 8 | packet.data[dataIndex + 1]);
                    mainModbusCom.SetHoldingReg((int)mainModbusSetting["id"] , (int)mainModbusSetting["Value"][i+1], packet.data[dataIndex + 2] << 8 | packet.data[dataIndex + 3]);
                }
            } else if ((int)mainModbusSetting["Type"][i] == 3) {
                // Type 3: Float
                if (dataIndex + 3 < sizeof(packet.data)) {
                    float floatValue = (float)(mainModbusCom.DWORD(
                        packet.data[dataIndex] << 8 | packet.data[dataIndex + 1],
                        packet.data[dataIndex + 2] << 8 | packet.data[dataIndex + 3])) / 1000.0;dataIndex = dataIndex + 4;
                    // Serial.println("Float[" + String((int)mainModbusSetting["Value"][i]) + "]:" + String(floatValue));
                    mainModbusCom.SetHoldingReg((int)mainModbusSetting["id"] , (int)mainModbusSetting["Value"][i], packet.data[dataIndex] << 8 | packet.data[dataIndex + 1]);
                    mainModbusCom.SetHoldingReg((int)mainModbusSetting["id"] , (int)mainModbusSetting["Value"][i+1], packet.data[dataIndex + 2] << 8 | packet.data[dataIndex + 3]);
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
 
    // Dừng ESP-NOW nếu đang chạy
    if (esp_now_deinit() == ESP_OK) {
        if (MeshConfig.debug) Serial.println("✅  Mesh deInit Success");
    } else {
        if (MeshConfig.debug) Serial.println("❌  Mesh deInit Failed");
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
        if (MeshConfig.debug) Serial.println("✅  WiFi protocol set successfully.");
    } else {
        if (MeshConfig.debug) Serial.print("❌  Failed to set WiFi protocol. Error: ");
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
        if (MeshConfig.debug) Serial.println("🚀 Configuration portal started.");
        mainwebInterface.setupWebConfig();

        APname = "";
        #ifdef USE_Modbus
    // }
    #endif//USE_Modbus
}

void checkConfigButton() {
    if(digitalRead(SETUP_BUTTON) == LOW || digitalRead(0) == LOW) {//Use SETUP_BUTTON or GPIO0
        if(buttonPressTime == 0) {
            buttonPressTime = millis();
        }
        else if(millis() - buttonPressTime > 3000 && millis() - buttonPressTime < 5000) { // Nhấn giữ 3s
            if(!configMode) {
                if(BUZZ == -1){

                    Serial.println("⚠️    BUZZ pin not set");
                    if(MeshConfig.dataVersion == 4){
                        Led_setColor(0x0000ff);delay(100);Led_setColor(0x00000);delay(100); // Toggle LED color
                        Led_setColor(0x0000ff);delay(100);Led_setColor(0x00000);delay(100); // Toggle LED color
                    }else{
                        digitalWrite(LED_STT, LOW);delay(100);digitalWrite(LED_STT, HIGH);delay(100);
                        digitalWrite(LED_STT, LOW);delay(100);digitalWrite(LED_STT, HIGH);delay(100);
                    }

                }else if(BUZZ > 0){

                    digitalWrite(BUZZ, HIGH);delay(5);digitalWrite(BUZZ, LOW);delay(100);
                    digitalWrite(BUZZ, HIGH);delay(5);digitalWrite(BUZZ, LOW);
                }
                startConfigPortal();
            }
        }
        else if(millis() - buttonPressTime > 5000) { // Nhấn giữ 5s

                if(BUZZ == -1){

                    Serial.println("⚠️    BUZZ pin not set");
                    if(MeshConfig.dataVersion == 4){
                        Led_setColor(0x0000ff);delay(100);Led_setColor(0x00000);delay(100); // Toggle LED color
                        Led_setColor(0x0000ff);delay(100);Led_setColor(0x00000);delay(100); // Toggle LED color
                        Led_setColor(0x0000ff);delay(100);Led_setColor(0x00000);delay(100); // Toggle LED color
                    }else{
                        digitalWrite(LED_STT, LOW);delay(100);digitalWrite(LED_STT, HIGH);delay(100);
                        digitalWrite(LED_STT, LOW);delay(100);digitalWrite(LED_STT, HIGH);delay(100);
                        digitalWrite(LED_STT, LOW);delay(100);digitalWrite(LED_STT, HIGH);delay(100);
                    }

                }else if(BUZZ > 0){

                    digitalWrite(BUZZ, HIGH);delay(5);digitalWrite(BUZZ, LOW);delay(100);
                    digitalWrite(BUZZ, HIGH);delay(5);digitalWrite(BUZZ, LOW);delay(100);
                    digitalWrite(BUZZ, HIGH);delay(5);digitalWrite(BUZZ, LOW);

                }
            ESP.restart();
        }
    }
    else {
        buttonPressTime = 0;
    }
}


///////////////////////////////////////////////////////// Serial TTL -> Mesh /////////////////////////////////////////////////////

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
            esp_now_peer_info_t peerInfo = {};
            memcpy(&peerInfo.peer_addr, MeshConfig.BrokerAddress, 6);
            if (!esp_now_is_peer_exist(MeshConfig.BrokerAddress)) {
                peerInfo.channel = MeshConfig.wifiChannel;
                esp_now_add_peer(&peerInfo);
            }

        //     if(MeshConfig.debug) {
        //         // In ra dữ liệu đã chuyển đổi
        //         Serial.println("DataLookline:");
        //         Serial.print("Network ID: ");
        //         Serial.println(DataLookline.networkID);
        //         Serial.print("Node ID: ");
        //         Serial.println(DataLookline.nodeID);
        //         Serial.print("PLAN: ");
        //         Serial.println(DataLookline.PLAN);
        //         Serial.print("RESULT: ");
        //         Serial.println(DataLookline.RESULT);
        //         Serial.print("State: ");
        //         Serial.println(DataLookline.state);
        //         Serial.print("Mode: ");
        //         Serial.println(DataLookline.Mode);
        //     }

        //     String id = "";
        //     id += String((DataLookline.nodeID / 1000) % 10);
        //     id += String((DataLookline.nodeID / 100) % 10);
        //     id += String((DataLookline.nodeID / 10) % 10);
        //     id += String((DataLookline.nodeID / 1) % 10);

        //     String StringPlan = "";
        //     StringPlan += (DataLookline.PLAN / 1000) % 10;
        //     StringPlan += (DataLookline.PLAN / 100) % 10;
        //     StringPlan += (DataLookline.PLAN / 10) % 10;
        //     StringPlan += (DataLookline.PLAN / 1) % 10;

        //     String StringResult = "";
        //     StringResult += (DataLookline.RESULT / 1000) % 10;
        //     StringResult += (DataLookline.RESULT / 100) % 10;
        //     StringResult += (DataLookline.RESULT / 10) % 10;
        //     StringResult += (DataLookline.RESULT / 1) % 10;
        //     String State = "";
        //   if(DataLookline.state){State ="1" + String(DataLookline.Mode);}else{State = "0" + String(DataLookline.Mode);}
        //   String sentData = id + "04" + "18" + StringPlan + StringResult + State;
          esp_err_t result;
            // if(MeshConfig.dataVersion == 3){  result = esp_now_send(MeshConfig.BrokerAddress, (uint8_t *) &DataLookline, sizeof(DataLookline));}
            // if(MeshConfig.dataVersion == 0){  result = esp_now_send(MeshConfig.BrokerAddress, (const uint8_t *)sentData.c_str(), sentData.length());}
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

void TskEthernet(void *pvParameter)
{
    LOG("TskEthernet Run in core ");
    LOGLN(xPortGetCoreID());
    initializeEthernet();
    for (;;)
    {
        #ifdef USE_W5500
        W5500loop();
        #endif//USE_W5500
        MQTTwifiConfig.loop();
        static long lastReceiveTime = millis();
        if (millis() - lastReceiveTime >= 5000) {
            lastReceiveTime = millis();
            if (MeshConfig.debug) Serial.println("TskEthernet MQTTwifiConfig.loop()  is running...");
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}


void TskApp(void *pvParameter)
{
    LOG("TskApp Run in core ");
    LOGLN(xPortGetCoreID());
    
    for (;;)
    {
        if(MeshConfig.dataVersion == 4){
            VehicleLoop();
        }   

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

bool LedState = false;
void TskModbus(void *pvParameter)
{
    LOG("TaskModbus Run in core ");
    LOGLN(xPortGetCoreID());
    
    for (;;)
    {
        // Log heap before main operations to track memory usage
        static size_t lastHeap = ESP.getFreeHeap();
        size_t currentHeap = ESP.getFreeHeap();
        if (currentHeap < lastHeap) {
            Serial.print("⚠️  Heap decreased on task Modbus: ");
            Serial.print(lastHeap);
            Serial.print(" -> ");
            Serial.println(currentHeap);
            lastHeap = currentHeap;
        }

        if((!configMode) && MeshConfig.MeshEnable){
        #ifdef USE_Modbus
            ModbusLoop(3000);  
        #endif//USE_Modbus
            MainLoop();
        }

        if(configMode){
            mainwebInterface.SocketLoop();
            dnsServer.processNextRequest();
            static long timeCount = 0;
            if (millis() - timeCount >= 100) {
                timeCount = millis();
                LedState = !LedState;
                if(MeshConfig.dataVersion == 4){
                    LedState?  Led_setColor(0x00ff00) : Led_setColor(0x000000); // Set LED to green or off
                }else{
                    digitalWrite(LED_STT, LedState); // Toggle LED state
                }
            }
        } 
        checkConfigButton();
        processSerialInput();
        
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void TaskWifiMQTT(void *pvParameter)
{
    LOG("TaskMQTT Run in core ");
    LOGLN(xPortGetCoreID());
    
    for (;;)
    {
        // Monitor heap memory and print if it decreases
        static size_t lastHeap = ESP.getFreeHeap();
        size_t currentHeap = ESP.getFreeHeap();
        if (currentHeap < lastHeap) {
            Serial.print("⚠️   Heap decreased on TaskMQTT: ");
            Serial.print(lastHeap);
            Serial.print(" -> ");
            Serial.println(currentHeap);
            lastHeap = currentHeap;
        }

        if (!MeshConfig.MeshEnable) {
            if(socketConnected && WebConnected){
            static long timeSocket = 0;
            if (millis() - timeSocket >= 2000) {
                timeSocket = millis();
            #ifdef USE_Modbus
                String JsonData = mainModbusCom.GetJson();
                mainwebInterface.SendMessageToClient(JsonData);
                JsonData = "";
                Serial.println("bulid json");
            #endif//USE_Modbus
            }  
            #ifdef USE_Modbus
            ModbusLoop(2000);
            #endif//USE_Modbus
            
            MainLoop();
            }
        }
        if(MeshConfig.dataVersion == 4){
            MQTTwifiConfig.loop();
        }
        else{
            if(!MeshConfig.MeshEnable && !configMode) {
                MQTTwifiConfig.loop();
            }
        }

        static long InfoCurrentMillis = millis();
        if(millis() -  InfoCurrentMillis >= 10000) {InfoCurrentMillis = millis();
            if (MeshConfig.debug){
                char *messenger = "========================================================================\n                          Loop Function\n========================================================================\n";
                Serial.println(messenger);Serial.flush();
                Serial.println("🎞   Free Ram: " + String((float)ESP.getFreeHeap()/1024.0) + "Kb");
                static size_t lastHeapInfo = ESP.getFreeHeap();
                static bool heapDecreased = false;
                size_t currentHeapInfo = ESP.getFreeHeap();
                if (currentHeapInfo < lastHeapInfo) {
                    Serial.print("⚠️   Heap decreased (Info): ");
                    Serial.print(lastHeapInfo);
                    Serial.print(" -> ");
                    Serial.println(currentHeapInfo);
                    heapDecreased = true;
                    lastHeapInfo = currentHeapInfo;
                } else if (heapDecreased && currentHeapInfo > lastHeapInfo) {
                    Serial.print("✅   Heap recovered (Info): ");
                    Serial.print(lastHeapInfo);
                    Serial.print(" -> ");
                    Serial.println(currentHeapInfo);
                    heapDecreased = false;
                    lastHeapInfo = currentHeapInfo;
                } else {
                    lastHeapInfo = currentHeapInfo;
                }
                Serial.println("🎞   Free Psram: " +  String(ESP.getFreePsram()/1024) + "Kb");
                // In nhiệt độ chip ra Serial
                #ifdef ESP32
                Serial.println("🌡️   Chip Temperature: " + String(temperatureRead()) + " °C");
                #endif
                
                MQTTwifiConfig.MQTTPush("iSoft/LEDBoard/ping", "ping");
            }
            if (socketConnected) {
                StaticJsonDocument<256> doc;
                doc["freeRam"] = ESP.getFreeHeap() / 1024;
                doc["freeSram"] = ESP.getFreePsram() / 1024;
            #ifdef ESP32
                doc["chipTemp"] = temperatureRead();
            #else
                doc["chipTemp"] = 0;
            #endif
                doc["mqttState"] = mqttEnable ? (mqttIsConnected ? "connected" : "disconnected") : "disabled";
                doc["wifiMode"] = wifiMode;
                doc["meshEnable"] = MeshConfig.MeshEnable;
                doc["loraEnable"] = MeshConfig.LoRaEnable;
                doc["runTime"] = millis() / 1000;
                doc["resetCounter"] = resetcounter;
                String boardInfo;
                serializeJson(doc, boardInfo);
                mainwebInterface.SendMessageToClient(boardInfo);
                doc.clear();
                boardInfo = "";
            }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}


bool EthernetAvilable = false;
int n_elements = 20000;
unsigned char * acc_data_all;

#include "PSRam.h"
//////////////////////////////////////////////////////////////////////////////////////////////////
void setup()
{
    Serial.begin(115200);

    EEPROM.begin(512);
    resetcounter = EEPROM.readLong(0);
    if (resetcounter == -1 || resetcounter == 0xFFFFFFFF) {
        resetcounter = 0;
        EEPROM.writeLong(0, 0);
        EEPROM.commit();
    }
    resetcounter++;
    EEPROM.writeLong(0, resetcounter);
    EEPROM.commit();


    #ifdef ESP32S1 
    while (!Serial) {
        delay(10); // Wait for serial monitor to open
        Serial.println("wait Serial Port");
    }
    #else
        waitSerialUSB();
    #endif//ESP32

    Serial.println("===== Start System =====");
    SerialInit();
    initializeFileSytem();
    loadConfig();
    printConfig(MeshConfig);
    
    if(MeshConfig.role == "Broker"){
        // loadDataMapping();
    }

    initializeSPI();

    initializeSDCard();

    #ifdef USE_Modbus
        initializeModbus();
    #endif//USE_Modbus

    delay(1000);
    MQTTwifiConfig.setup();// read MQTT & WiFi config

    initializeWifi();
    
    if (!MeshConfig.MeshEnable){
        if(wifiMode == "STA"){
        }
        
        if(wifiMode == "AP"){
            startConfigPortal();
        }
    }

    if (MeshConfig.MeshEnable) {
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
            if (MeshConfig.debug) Serial.println("✅  Mesh Init Success");
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
            if (MeshConfig.debug) Serial.println("❌  Mesh Init Failed");
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
            esp_now_register_send_cb(transmissionComplete);         
            esp_now_register_recv_cb(dataReceived);               
            esp_now_add_peer(MeshConfig.BrokerAddress, RECEIVER_ROLE, MeshConfig.wifiChannel, NULL, 0);

        #endif//ESP32

    }
    

    if(SETUP_BUTTON < 0){
        Serial.println("⚠️    SETUP_BUTTON pin not set");
        SETUP_BUTTON = 0; // Set to 0 to disable button functionality
    }
    // use the built in button
    pinMode(SETUP_BUTTON, INPUT_PULLUP);
    LedState = false; // Initialize LED state
    if(MeshConfig.dataVersion == 4){
        VehicleSetup();
        if(MeshConfig.debug)Serial.println("  Using external RGB LED");
        Led_setColor(0x000000); // Set LED to off
        delay(100);
        Led_setColor(0x00ff00); // Set LED to green
        delay(100);
        Led_setColor(0x000000); // Set LED to off
        delay(100);
        Led_setColor(0x00ff00); // Set LED to green
        delay(100);
        Led_setColor(0x000000); // Set LED to off
        delay(100);
        Led_setColor(0x00ff00); // Set LED to green
        delay(100);
    }else{
        if(MeshConfig.debug)Serial.println("  Using external LED");
        // use the built in LED
        pinMode(LED_STT, OUTPUT);
        digitalWrite(LED_STT, HIGH);delay(100);
        digitalWrite(LED_STT, LOW);delay(100);
        digitalWrite(LED_STT, HIGH);delay(100);
        digitalWrite(LED_STT, LOW);delay(100);
    }

    #ifdef USE_Modbus
        xTaskCreatePinnedToCore(TskModbus, "TaskModbus", 16384, NULL, 2, &TaskModbus, 1);
    #endif//USE_Modbus

    if (MeshConfig.debug) Serial.println("Creating Task Application ");
    xTaskCreatePinnedToCore(TskApp, "TskApp", 8000, NULL, 1, &TaskApp, 0);

    if (MeshConfig.debug) Serial.println("🌎   Creating Task Ethernet");
    xTaskCreatePinnedToCore(TskEthernet, "TskEthernet", 8000, NULL, 1, &TaskEthernet, 1);

    if (MeshConfig.debug) Serial.println("\n🌎 Timezone: " + String(timezone));
    rtcTimeOnl.Time_setup(timezone);

    initHardware();
        
    // #endif
    // LittleFS.remove("/index.html");
    // setupWebSocket();

    if (MeshConfig.debug) Serial.println("\n\n================================================================");
    if (MeshConfig.debug) Serial.println("PSRAM available: " + String(psramFound() ? "Yes" : "No"));
    if (psramInit()) {
        if (MeshConfig.debug) Serial.println("✅  PSRAM initialized successfully.");
        acc_data_all = (unsigned char *) ps_malloc (n_elements * sizeof (unsigned char)); 
    } else {
        if (MeshConfig.debug) Serial.println("❌  PSRAM initialization failed or not available.");
    }
    if (MeshConfig.debug) Serial.println("================================================================\n\n");
    Serial.println("===== System Setup Completed =====");
    if (MeshConfig.debug) Serial.println("Free Heap: " + String(ESP.getFreeHeap() / 1024) + "Kb");
    if (MeshConfig.debug) Serial.println("Free PSRAM: " + String(ESP.getFreePsram() / 1024) + "Kb");
    if (MeshConfig.debug) Serial.println("Reset Counter: " + String(resetcounter));
    if (MeshConfig.debug) Serial.println("===================================");
    
    xTaskCreatePinnedToCore(TaskWifiMQTT, "TaskWifiMQTT", 16384, NULL, 1, &TaskMQTT, 0);
/////////////////////////////////////////////////////////////////////////////////////////////////////////

}
long timeCount = 0;
bool once1 = false;
void loop()
{      

    static long ModbusCurrentMillis = millis();
    if(millis() -  ModbusCurrentMillis >= 25000) {
        ModbusCurrentMillis = millis();
        
        if (psramFound()){
            Serial.println(acc_data_all[1]);
            acc_data_all[1] = 'a';
        }  
        // printNodeDataWithMapping(); // In dữ liệu node
        printNodeData() ;
        if (MeshConfig.MeshEnable) {
            StaticJsonDocument<1024> jsonDoc;
            char jsonBuffer[1024];
            createJsonForWebSocket(jsonBuffer, sizeof(jsonBuffer)); 
            size_t len = serializeJson(jsonDoc, jsonBuffer, sizeof(jsonBuffer));
            if(socketConnected){
                mainwebInterface.SendMessageToClient(String(jsonBuffer));
            }
            memset(jsonBuffer, 0, sizeof(jsonBuffer));
            jsonDoc.clear();
        }
    }

    MQTTwifiConfig.loop();
    // MainLoop();
    processQueue();
}

// void GetModbusData(int i, uint8_t type) {
//     if(type == 0){
//         mainModbusCom.GetCoilReg((int)mainModbusSetting["Value"][i]);
//     }

//     if(type == 1){
//         mainModbusCom.GetHoldingReg((int)mainModbusSetting["Value"][i]);
//     }

//     if(type == 2){
//         uint32_t dwordValue = mainModbusCom.DWORD(mainModbusCom.GetHoldingReg((int)mainModbusSetting["Value"][i]), mainModbusCom.GetHoldingReg(((int)mainModbusSetting["Value"][i]) + 1));
//     }

//     if(type == 3){
//         (float)(mainModbusCom.DWORD(mainModbusCom.GetHoldingReg((int)mainModbusSetting["Value"][i]), mainModbusCom.GetHoldingReg(((int)mainModbusSetting["Value"][i])+ 1))); 
//     }
// }

void MainLoop()
{
    // In ra heap trước/sau các đoạn code nghi ngờ để theo dõi mức sử dụng bộ nhớ
    static size_t lastHeap = ESP.getFreeHeap();
    size_t currentHeap = ESP.getFreeHeap();
    if (currentHeap < lastHeap) {
        Serial.print("⚠️  Heap decreased in MainLoop: ");
        Serial.print(lastHeap);
        Serial.print(" -> ");
        Serial.println(currentHeap);
        lastHeap = currentHeap;
    }


if(once1 == false){
    if (MeshConfig.debug){
        char *messenger = "========================================================================\n                          Start Main Loop\n========================================================================\n";
        Serial.println(messenger);Serial.flush();messenger = "";
    }
    once1 = true;
}


// if (!MeshConfig.MeshEnable) return;

if(!configMode && MeshConfig.MeshEnable){
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
        char payloadSent[512] = {0};
        char dataAt[256] = {0};
        char timeAt[32] = {0};
        rtcTimeOnl.Time_loop(); rtcTimeOnl.GetTime();
        snprintf(timeAt, sizeof(timeAt), "%04d-%02d-%02d %02d:%02d:%02d", Getyear, Getmonth, Getday, Gethour, Getmin, Getsec);
        createJsonForMqttt(dataAt, sizeof(dataAt)); // Giả sử hàm này hỗ trợ buffer
        snprintf(payloadSent, sizeof(payloadSent),
            "{\"connId\":\"%s\",\"data\":[%s],\"exeAt\":\"%s\"}",
            conId, dataAt, timeAt);
        MQTTwifiConfig.MQTTPush(mqttTopicPub, payloadSent);
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
        if(MeshConfig.dataVersion == 2 && !configMode && MeshConfig.MeshEnable){
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
                    if (MeshConfig.debug) Serial.println("✅  Node message success");
                }
                else
                {
                    if (MeshConfig.debug) Serial.println("❌  Unknown error");
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
#else
#include "Project/DoorLocker.h"
#define DoorUpState     15
#define DoorStopState   16
#define DoorDownState   17
bool debug = true; // Set to true for debugging output
#include <WiFi.h>
#include <PubSubClient.h>

const char* mqtt_server = "test.mosquitto.org"; // Replace with your MQTT broker address
const char* mqtt_user = "";
const char* mqtt_password = "";
const char* mqtt_topic = "door/state";

WiFiClient espClient;
PubSubClient client(espClient);

void publishDoorState() {
    String state = String(DoorState ? "Open" : "Closed");
    client.publish(mqtt_topic, state.c_str(), true); // retain = true
}

void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
        if (client.connect("DoorLockerClient")) {
            client.subscribe(mqtt_topic);
        } else {
            delay(2000);
        }
    }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    // Optionally handle incoming messages if needed
    const char* message = (const char*)payload;
    if (debug) {
        Serial.print("Message arrived [");
        Serial.print(topic);
        Serial.print("]: ");
        for (unsigned int i = 0; i < length; i++) {
            Serial.print((char)payload[i]);
        }
        Serial.println();
    }
    //Door ID with state message for update client state
    
}

void setup_mqtt_wifi() {
    if(WiFi.status() != WL_CONNECTED) {
        WiFi.begin(SSID, PASS); // Replace with your WiFi credentials
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
        }
        Serial.println("Connected to WiFi");
        Serial.print(" 🌐   IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("Already connected to WiFi");
        client.setServer(mqtt_server, 1883);
        client.setCallback(mqttCallback);
    }
}


void DoorStates(){
    if (DoorState == 0) { // Closed
        if (digitalRead(DoorUpState) == HIGH) {
            DoorState = 1; // Open
            Serial.println("Door is now Open");
        }
    } else if (DoorState == 1) { // Open
        if (digitalRead(DoorDownState) == HIGH) {
            DoorState = 0; // Closed
            Serial.println("Door is now Closed");
        } else if (digitalRead(DoorStopState) == HIGH) {
            DoorState = 2; // Stopped
            Serial.println("Door is Stopped");
        }
    } else if (DoorState == 2) { // Stopped
        if (digitalRead(DoorUpState) == HIGH) {
            DoorState = 1; // Open
            Serial.println("Door is now Open");
        } else if (digitalRead(DoorDownState) == HIGH) {
            DoorState = 0; // Closed
            Serial.println("Door is now Closed");
        }
    }
}
#include <EEPROM.h>
long resetcounter = 0;
void setup() {
    EEPROM.begin(512);
    if(EEPROM.readLong(0) == 0xFFFFFFFF || EEPROM.readLong(0) == -1) {
        EEPROM.writeLong(0, 0);
        EEPROM.commit();
    }
    resetcounter = EEPROM.readLong(0);
    Serial.begin(115200);
    setup_mqtt_wifi();
    DoorSetup();
    Serial.println("DoorLocker setup completed.");
    pinMode(DoorUpState, OUTPUT);
    pinMode(DoorStopState, OUTPUT);
    pinMode(DoorDownState, OUTPUT);
    digitalWrite(DoorUpState, LOW);
    digitalWrite(DoorStopState, LOW);
    digitalWrite(DoorDownState, LOW);
    client.setServer(mqtt_server, 1883);
    client.setCallback(mqttCallback);
    if (!client.connected()) {
        reconnect();
    }
    if (debug) {
        Serial.println("DoorLocker initialized with reset counter: " + String(resetcounter));
    }
}

void loop() {
    client.loop();

    DoorLoop();
    if (!client.connected()) {
        reconnect();
    }

    static int lastState = DoorState;
    if (DoorState != lastState) {
        publishDoorState();
        lastState = DoorState;
    }

    DoorLoop(); // Call the loop function from DoorLocker.h


    static long lastLoopTime = millis();
    if (millis() - lastLoopTime >= 10000) { // Run every second
        lastLoopTime = millis();
        Serial.println("\n=================================================");
        Serial.println("DoorLocker loop running...");
        Serial.println("  🖥️   Reset Counter: " + String(resetcounter));
        Serial.println("  🚪   Door state: " + String(DoorState ? "Open" : "Closed"));
        Serial.println("  💾   Free Heap: " + String(ESP.getFreeHeap() / 1024) + "Kb");
        Serial.println("  🎞   Free PSRAM: " + String(ESP.getFreePsram() / 1024) + "Kb");
        Serial.println("  🌡️  Chip : " + String(temperatureRead()) + " °C");
        Serial.println("=================================================\n");
        DoorStates();
    }   
    
}
#endif//USE_DoorLoker
//580 - 6-15 chỉ
