#include "main.h"
#include <ArduinoJson.h>

#include <Arduino.h>

// int timezone = 7;

//#define TCP_ETH
#define RTU_RS485

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
    bool BuzzEnable;
    bool MeshEnable; // Thêm dòng này
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

#ifdef USE_Modbus
#include "TskModbus.h"
ModbusConfig mainModbusConfig;
TaskHandle_t TaskMQTT;
TaskHandle_t TaskModbus;

#include "WebInterface.h"
WebinterFace mainwebInterface;

#ifdef RTU_RS485
#include "Modbus_RTU.h"
Modbus_Prog mainModbusCom;
#endif//RTU_RS485
  #ifdef TCP_ETH
  #include "ModbusTCP.h"
  Modbus_TCP_Prog iMagModbusTCPCom;
  #endif//TCP_ETH

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
 
#include "Wifi_conf.h"
// #include "Project/lookline.h"

#ifdef USE_W5500
// EthernetClient client;
// // EthernetServer serverEth(80);
// IPAddress ip(192, 168, 5, 250); // Chỉnh lại theo mạng của bạn
// byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
#endif//USE_W5500

#include "W5500Ethernet.h"

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


// void convertDataPacketToDataLookline(const dataPacket &packet, struct_Parameter_messageOld &dataLookline);
  

void MainLoop();

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
        MeshConfig.role = "Node"; // Default role
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
    MeshConfig.role = doc["role"] | "Node"; // Default to "Node" if not specified
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
        if (MeshConfig.debug) Serial.println("❌ Failed to open config file for writing.");
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
        if (MeshConfig.debug) Serial.println("❌ Failed to write to config file.");
    } else {
        if (MeshConfig.debug) Serial.println("✅ Config saved.");
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
                    if(MeshConfig.BuzzEnable){digitalWrite(BUZZ, HIGH);delay(2);digitalWrite(BUZZ, LOW);}
                    if (MeshConfig.debug) Serial.println("✅ Repeater forwarded data to Broker successfully.");
                } else {
                    if (MeshConfig.debug) Serial.println("❌ Repeater failed to forward data to Broker.");
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
        if (MeshConfig.debug) Serial.println("✅ Mesh deInit Success");
    } else {
        if (MeshConfig.debug) Serial.println("❌ Mesh deInit Failed");
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
        if (MeshConfig.debug) Serial.println("✅ WiFi protocol set successfully.");
    } else {
        if (MeshConfig.debug) Serial.print("❌ Failed to set WiFi protocol. Error: ");
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
                if(BUZZ < 0){
                    Serial.println("⚠️   BUZZ pin not set");
                    digitalWrite(LED_STT, LOW);delay(100);digitalWrite(LED_STT, HIGH);delay(100);
                    digitalWrite(LED_STT, LOW);delay(100);digitalWrite(LED_STT, HIGH);delay(100);
                }else{
                    digitalWrite(BUZZ, HIGH);delay(5);digitalWrite(BUZZ, LOW);delay(100);
                    digitalWrite(BUZZ, HIGH);delay(5);digitalWrite(BUZZ, LOW);
                }
                startConfigPortal();
            }
        }
        else if(millis() - buttonPressTime > 5000) { // Nhấn giữ 5s
                if(BUZZ < 0){
                    Serial.println("⚠️   BUZZ pin not set");
                    digitalWrite(LED_STT, LOW);delay(100);digitalWrite(LED_STT, HIGH);delay(100);
                    digitalWrite(LED_STT, LOW);delay(100);digitalWrite(LED_STT, HIGH);delay(100);
                    digitalWrite(LED_STT, LOW);delay(100);digitalWrite(LED_STT, HIGH);delay(100);
                }else{
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

String createModbusJson() {
    String json = "{\"masterData\":{";
        JSONVar DataBlocks = mainModbusCom.getDataBlocks();
        for (int b = 0; b < (int)DataBlocks["block"].length(); b++) {
            int amount = (int)DataBlocks["block"][b]["amount"];
            for (int j = 0; j < amount; j++) {
                if (j > 0 || b > 0) json += ",";
                int addr = (int)DataBlocks["block"][b]["addr"] + j;
                int type = (int)DataBlocks["block"][b]["type"];
                json += "\"";
                json += String(addr);
                json += "\":";
                if (type == 0) {
                    json += mainModbusCom.GetCoilReg(addr) ? 1 : 0;
                } else if (type == 1) {
                    json += String(mainModbusCom.GetInputHoldingReg(addr));
                } else if (type == 2) {
                    uint32_t dwordValue = mainModbusCom.DWORD(
                        mainModbusCom.GetInputHoldingReg(addr),
                        mainModbusCom.GetInputHoldingReg(addr + 1)
                    );
                    json += String(dwordValue);
                } else if (type == 3) {
                    float floatValue = (float)mainModbusCom.DWORD(
                        mainModbusCom.GetInputHoldingReg(addr),
                        mainModbusCom.GetInputHoldingReg(addr + 1)
                    );
                    json += String(floatValue);
                }
            }
        }
    
    json += "}}";
    return json;
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
            mainwebInterface.SocketLoop();
            dnsServer.processNextRequest();
            static long timeCount = 0;
            if (millis() - timeCount >= 100) {
                timeCount = millis();
                digitalWrite(LED_STT, !digitalRead(LED_STT)); // Toggle LED state
            }
        } 
        checkConfigButton();
        processSerialInput();
        
        if (!MeshConfig.MeshEnable) {
                ModbusLoop(2000);
            if(socketConnected && WebConnected){
                static long timeSocket = 0;
                if (millis() - timeSocket >= 2000) {
                    timeSocket = millis();
                    mainwebInterface.SendMessageToClient(mainModbusCom.GetJson());
                }  
            }
            MainLoop();
        }
    }
}
void TaskWifiMQTT(void *pvParameter)
{
    LOG("TaskMQTT Run in core ");
    LOGLN(xPortGetCoreID());
    initializeEthernet();
    
    for (;;)
    {
        
        #ifdef USE_Modbus
        MQTTwifiConfig.loop();
        #endif//USE_Modbus
    }
}
bool EthernetAvilable = false;


//////////////////////////////////////////////////////////////////////////////////////////////////
void setup()
{
    Serial.begin(115200);
    while (!Serial) {
        delay(10); // Wait for serial monitor to open
    }
    Serial.println("===== Start System =====");
    SerialInit();
    initializeFileSytem();
    loadConfig();
    printConfig(MeshConfig);
    
    if(mqttEnable){
        MQTTwifiConfig.setup();
    }
    if(MeshConfig.role == "Broker"){
        // loadDataMapping();
    }

    initializeSPI();
    initializeSDCard();
    initializeModbus();
    delay(1000);
    initializeWifi();
    if (!MeshConfig.MeshEnable) {
        if(mqttEnable == false && wifiMode == "AP"){
            startConfigPortal();
        }
    }else{
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
            if (MeshConfig.debug) Serial.println("✅ Mesh Init Success");
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
            if (MeshConfig.debug) Serial.println("❌ Mesh Init Failed");
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
    
    #ifdef USE_Modbus
    xTaskCreatePinnedToCore(TaskWifiMQTT, "TaskMain", 10000, NULL, 1, &TaskMQTT, 1);
    #endif//USE_Modbus
    if(SETUP_BUTTON < 0){
        Serial.println("⚠️   SETUP_BUTTON pin not set");
        SETUP_BUTTON = 0; // Set to 0 to disable button functionality
    }
    // use the built in button
    pinMode(SETUP_BUTTON, INPUT_PULLUP);
    pinMode(LED_STT, OUTPUT);
    digitalWrite(LED_STT, HIGH);delay(100);
    digitalWrite(LED_STT, LOW);delay(100);
    digitalWrite(LED_STT, HIGH);delay(100);
    digitalWrite(LED_STT, LOW);delay(100);

    xTaskCreatePinnedToCore(TskModbus, "TaskModbus", 10000, NULL, 2, &TaskModbus, 1);

    if (MeshConfig.debug) Serial.println("\n🌎 Timezone: " + String(timezone));
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
        if (MeshConfig.debug){
            char *messenger = "========================================================================\n                          Loop Function\n========================================================================\n";
            Serial.println(messenger);Serial.flush();
            Serial.println("🎞   Free Ram: " + String(ESP.getFreeHeap()/1024) + "Kb");
            Serial.println("🎞   Free SRam: " +  String(ESP.getFreePsram()/1024) + "Kb");
            // In nhiệt độ chip ra Serial
            #ifdef ESP32
            Serial.println("🌡️   Chip Temperature: " + String(temperatureRead()) + " °C");
            #endif
        }
        // printNodeDataWithMapping(); // In dữ liệu node
        printNodeData() ;

        

        if (MeshConfig.MeshEnable) {
            String jsonString = createJsonForWebSocket();
                // Serial.println("Data: " + jsonString);
            if(socketConnected){
                mainwebInterface.SendMessageToClient(jsonString);
            }
        }
    }
    if(!configMode && mqttEnable){
        MQTTwifiConfig.loop();
    }

    MainLoop();
    
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
    if(once1 == false){
        if (MeshConfig.debug){
            char *messenger = "========================================================================\n                          Start Main Loop\n========================================================================\n";
            Serial.println(messenger);Serial.flush();
        }
        once1 = true;
    }

    #ifdef USE_W5500
        W5500loop();
    #endif//USE_W5500
    
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
                        if (MeshConfig.debug) Serial.println("✅ Node message success");
                    }
                    else
                    {
                        if (MeshConfig.debug) Serial.println("❌ Unknown error");
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
