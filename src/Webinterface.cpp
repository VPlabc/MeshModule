#include "WebInterface.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "LoRa.h"
LoRaFunction configLoRa;

#include "Modbus_RTU.h"
Modbus_Prog WebModbusCom;

const String localIPURL = "http://192.168.4.1";	 // a string version of the local IP with http, used for redirecting clients to your webpage

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

String mqttHost = "broker.hivemq.com";
int mqttPort = 1883;
String mqttUser = "username";
String mqttPass = "password";
bool mqttEnable = true;

String wifiMode = "AP";
String ssid = "I-Soft";
String pass = "i-soft@2023";
String conId = "b8e54d33-b34a-45ab-b76f-62c8a9abc6c4";
String mqttTopicStat = "iSoftMesh/Status";
String mqttTopic = "iSoftMesh/data";
String mqttTopicSub = "iSoftMesh/data/sub";
int mqttKeepAlive = 60;
bool mqttCleanSession = true;
int mqttQos = 0;
bool mqttRetain = false;
String mqttLwtTopic = "iSoftMesh/LWT";
String mqttLwtMessage = "Offline";
int mqttLwtQos = 1;
bool mqttLwtRetain = false;
bool mqttLwtEnabled = false;
int timezone = 7;
bool mqttIsConnected = false;
bool socketConnected = false;
bool WebConnected = false;

////////////////////////// permission /////////////////////////

            // Default passwords
            static const String USER_PASS = "user123";
            static const String STAFF_PASS = "staff123";
            static const String ADMIN_PASS = "27XuanQuynh@2025";

            // Global variable for permission level
            // 0: guest, 1: user, 2: staff, 3: admin
            static int permissionLevel = 0;
//////////////////////////////////////////////////////////////
const char* BoardCustomJson1 = R"json({
    "BUZZ": -1,
    "SETUP_BUTTON": 0,
    "LED_STT": 14,
    "I2C_SDA": 41,
    "I2C_SCL": 42,
    "Y8": -1,
    "Y9": -1,
    "DA0": -1,
    "DA1": -1,
    "InPut0": -1,
    "InPut1": -1,
    "InPut2": -1,
    "InPut3": -1,
    "InPut4": -1,
    "Ser_1RX": 18,
    "Ser_1TX": 17,
    "M0_PIN": 16,
    "M1_PIN": -1,
    "CS_PIN": 20,
    "RST_PIN": 8,
    "INIT_PIN": 19,
    "Ser_2RX": -1,
    "Ser_2TX": -1,
    "MOSI_PIN": 35,
    "MISO_PIN": 37,
    "SCK_PIN": 36,
    "SD_CS_PIN": 21
})json";

String processors(const String& var) {
    if (var == "HELLO_FROM_TEMPLATE") return F("Hello world!");
    return String();
  }
 
  
class CaptiveRequestHandler : public AsyncWebHandler {
    public:
        CaptiveRequestHandler() {}
        virtual ~CaptiveRequestHandler() {}

        bool canHandle(AsyncWebServerRequest *request){
        //request->addInterestingHeader("ANY");
        return true;
        }
};

#include <vector>

// Helper: Convert String password to binary vector
std::vector<uint8_t> passwordToBin(const String& password) {
    std::vector<uint8_t> bin(password.length());
    for (size_t i = 0; i < password.length(); ++i) {
        bin[i] = static_cast<uint8_t>(password[i]);
    }
    return bin;
}

// Helper: Convert binary vector to String password
String binToPassword(const std::vector<uint8_t>& bin) {
    String password;
    for (uint8_t b : bin) {
        password += static_cast<char>(b);
    }
    return password;
}

// Save password to certification.bin (overwrite)
bool savePasswordToBinFile(const String& password, byte Level) {
    if (!LittleFS.begin()) {
        Serial.println("Failed to mount LittleFS");
        return false;
    }
    File file = LittleFS.open("/certification" + String(Level) + ".bin", "w");
    if (!file) return false;
    std::vector<uint8_t> bin = passwordToBin(password);
    file.write(bin.data(), bin.size());
    file.close();
    return true;
}

// Read password from certification.bin
String readPasswordFromBinFile(byte Level) {
    if (!LittleFS.begin()) {
        Serial.println("Failed to mount LittleFS");
        return "";
    }
    File file = LittleFS.open("/certification" + String(Level) + ".bin", "r");
    if (!file) return "";
    size_t len = file.size();
    std::vector<uint8_t> bin(len);
    file.read(bin.data(), len);
    file.close();
    return binToPassword(bin);
}
////////////////////////////////////////// Web Socket ///////////////////////////////////////////////////

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
      data[len] = 0;
    //   if (strcmp((char*)data, "toggle") == 0) {
    //     ledState = !ledState;
    //     notifyClients();
    //   }
        Serial.println("Data Recive:" + String((char*)data));
        DynamicJsonDocument doc(256);
        DeserializationError error = deserializeJson(doc, (char*)data);
        if (!error && doc.containsKey("login")) {
            String user = doc["user"] | "";
            String password = doc["password"] | "";
            if (user == "admin" && password == readPasswordFromBinFile(3)) {
                permissionLevel = 3;
                ws.textAll("{\"login\":\"success\",\"role\":\"admin\"}");
            } else if (user == "staff" && password == readPasswordFromBinFile(2)) {
                permissionLevel = 2;
                ws.textAll("{\"login\":\"success\",\"role\":\"staff\"}");
            } else if (user == "user" && password == readPasswordFromBinFile(1)) {
                permissionLevel = 1;
                ws.textAll("{\"login\":\"success\",\"role\":\"user\"}");
            } else {
                permissionLevel = 0;
                ws.textAll("{\"login\":\"fail\"}");
            }
        }

        if (permissionLevel > 0 && !error && doc.containsKey("cmnd")) {
            String cmnd = doc["cmnd"].as<String>();

            if (cmnd == "getPermissionLevel") {
                String resp = "{\"permissionLevel\":" + String(permissionLevel) + "}";
                ws.textAll(resp);
            } else if (cmnd == "logout") {
                permissionLevel = 0;
                ws.textAll("{\"logout\":\"success\"}");
            } else if (cmnd == "changePassword") {
                String newPass = doc["data"] | "";
                
                if (permissionLevel == 3) {
                    savePasswordToBinFile(newPass, 3);
                    ws.textAll("{\"changePassword\":\"admin updated\"}");
                } else if (permissionLevel == 2) {
                    savePasswordToBinFile(newPass, 2);
                    ws.textAll("{\"changePassword\":\"staff updated\"}");
                } else if (permissionLevel == 1) {
                    savePasswordToBinFile(newPass, 1);
                    ws.textAll("{\"changePassword\":\"user updated\"}");
                } else {
                    ws.textAll("{\"changePassword\":\"fail\"}");
                }
            }
            else if (cmnd == "setModbusValue") {
                int modbusId = doc["id"] | 1;
                String regType = doc["type"] | "";
                uint16_t address = doc["address"] | -1;
                if (address < 0 || regType == "") {
                    ws.textAll("{\"setModbusValue\":\"fail\",\"error\":\"Invalid address or type\"}");
                    return;
                }
                if (!doc.containsKey("value")) {
                    ws.textAll("{\"setModbusValue\":\"fail\",\"error\":\"Missing value\"}");
                    return;
                }
                bool success = false;
                if (regType == "coil") {
                    bool val = doc["value"];
                    WebModbusCom.SetCoilReg(modbusId, address, val);
                    success = true;
                } else if (regType == "word") {
                    uint16_t val = doc["value"];
                    WebModbusCom.SetHoldingReg(modbusId, address, val);
                    success = true;
                }
                if (success) {
                    ws.textAll("{\"setModbusValue\":\"ok\"}");
                } else {
                    ws.textAll("{\"setModbusValue\":\"fail\"}");
                }
            }
        }
    }
  }
  void WebinterFace::SendMessageToClient(const String& message) {
    ws.textAll(message);
  }
  void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
               void *arg, uint8_t *data, size_t len) {
    switch (type) {
      case WS_EVT_CONNECT:
        Serial.printf("🔗 WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        socketConnected = true;
        break;
      case WS_EVT_DISCONNECT:
        Serial.printf("❌ WebSocket client #%u disconnected\n", client->id());
        socketConnected = false;
        break;
      case WS_EVT_DATA:
        handleWebSocketMessage(arg, data, len);
        break;
      case WS_EVT_PONG:
      case WS_EVT_ERROR:
        break;
    }
  }
  
  void initWebSocket() {
    ws.onEvent(onEvent);
    server.addHandler(&ws);
  }
void WebinterFace::SocketLoop() {
    ws.cleanupClients();
  }

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void WebinterFace::setupWebConfig() {
    initWebSocket();
    // API: Reset ESP
    server.on("/reset-esp", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200, "application/json", "{\"status\":\"✅ ESP will reset\"}");
        delay(500);
        ESP.restart();
    });
/////////////////////////////////////////
    // API: Set initial passwords for user, staff, admin
    server.on("/set-initial-passwords", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        String body = String((char*)data).substring(0, len);
        DynamicJsonDocument doc(512);
        DeserializationError error = deserializeJson(doc, body);
        if (error) {
            request->send(400, "application/json", "{\"status\":\"fail\",\"error\":\"Invalid JSON\"}");
            return;
        }
        String userPass = doc["user"] | "";
        String staffPass = doc["staff"] | "";
        String adminPass = doc["admin"] | "";
        if (userPass.length() == 0 || staffPass.length() == 0 || adminPass.length() == 0) {
            request->send(400, "application/json", "{\"status\":\"fail\",\"error\":\"All fields required\"}");
            return;
        }
        bool ok1 = savePasswordToBinFile(userPass, 1);
        bool ok2 = savePasswordToBinFile(staffPass, 2);
        bool ok3 = savePasswordToBinFile(adminPass, 3);
        if (ok1 && ok2 && ok3) {
            request->send(200, "application/json", "{\"status\":\"ok\"}");
        } else {
            request->send(500, "application/json", "{\"status\":\"fail\",\"error\":\"Failed to save passwords\"}");
        }
    });
    // Ethernet Settings API
    server.on("/load-ethernet-config", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (LittleFS.exists(ETHERNET_FILE)) {
            File file = LittleFS.open(ETHERNET_FILE, "r");
            if (file) {
                String json = file.readString();
                file.close();
                request->send(200, "application/json", json);
            } else {
                request->send(500, "application/json", "{\"error\":\"❌ Failed to open ethernet.json\"}");
            }
        } else {
            request->send(404, "application/json", "{\"error\":\"❌ ethernet.json not found\"}");
        }
    });

    server.on("/save-ethernet-config", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        String body = String((char*)data).substring(0, len);
        DynamicJsonDocument doc(512);

        DeserializationError error = deserializeJson(doc, body);
        if (error) {
            request->send(400, "application/json", "{\"error\":\"❌ Invalid JSON\"}");
            return;
        }

        // Optionally validate fields: dhcpEnable, ip, subnet, gateway, dns
        bool dhcpEnable = doc["dhcp"] | true;
        String ip = doc["ip"] | "";
        String subnet = doc["subnet"] | "";
        String gateway = doc["gateway"] | "";
        String dns = doc["dns"] | "";

        // Save to file
        File configFile = LittleFS.open(ETHERNET_FILE, "w");
        if (!configFile) {
            request->send(500, "application/json", "{\"error\":\"❌ Failed to save configuration\"}");
            return;
        }
        serializeJson(doc, configFile);
        configFile.close();
        request->send(200, "application/json", "{\"status\":\"✅ Configuration saved\"}");
    });
//////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// Oin Mapping //////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

// save-data-viewer
    server.on("/save-data-viewer", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        String body = String((char*)data).substring(0, len);
        DynamicJsonDocument doc(1024);

        DeserializationError error = deserializeJson(doc, body);
        if (error) {
            request->send(400, "application/json", "{\"error\":\"❌ Invalid JSON\"}");
            return;
        }

        // Save the configuration to data-mapping.json
        File configFile = LittleFS.open(DATA_VIEWER_FILE, "w");
        if (!configFile) {
            request->send(500, "application/json", "{\"error\":\"❌ Failed to save configuration\"}");
            return;
        }

        serializeJson(doc, configFile);
        configFile.close();
        request->send(200, "application/json", "{\"status\":\"✅ Configuration saved\"}");
    });
    //load-data-viewer
    server.on("/load-data-viewer", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (LittleFS.exists(DATA_VIEWER_FILE)) {
            // File does not exist, create with default content
            File file = LittleFS.open("/PinMap.json", "w");
            if (file) {
                file.print(BoardCustomJson1);
                file.close();
            }
            file = LittleFS.open("/PinMap.json", "r");
            if (!file) {
                Serial.println("Failed to create PinMap.json");
                return;
            }
        }
        if (LittleFS.exists(DATA_VIEWER_FILE)) {
            File file = LittleFS.open(DATA_VIEWER_FILE, "r");
            if (file) {
                String json = file.readString();
                file.close();
                request->send(200, "application/json", json);
            } else {
                request->send(500, "application/json", "{\"error\":\"❌ Failed to open data-mapping.json\"}");
            }
        } else {
            request->send(404, "application/json", "{\"error\":\"❌ data-mapping.json not found\"}");
        }
    });
//////////////////////////////////////////////////////////////////
////////////////////////////////////////// Data Mapping //////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
    server.on("/data-mapping", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (!LittleFS.exists(DATA_MAPPING_FILE)) {
            File file = LittleFS.open(DATA_MAPPING_FILE, "w");
            if (file) {
                file.print("{}"); // Tạo file mặc định rỗng (hoặc nội dung mặc định khác nếu muốn)
                file.close();
            }
        }
        if (LittleFS.exists(DATA_MAPPING_FILE)) {
            File file = LittleFS.open(DATA_MAPPING_FILE, "r");
            if (file) {
                String json = file.readString();
                file.close();
                request->send(200, "application/json", json);
            } else {
                request->send(500, "application/json", "{\"error\":\"❌ Failed to open data-mapping.json\"}");
            }
        } else {
            request->send(404, "application/json", "{\"error\":\"❌ data-mapping.json not found\"}");
        }
        WebConnected = true;
    });

    server.on("/save-data-mapping", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        String body = String((char*)data).substring(0, len);
        DynamicJsonDocument doc(1024);

        DeserializationError error = deserializeJson(doc, body);
        if (error) {
            request->send(400, "application/json", "{\"error\":\"❌ Invalid JSON\"}");
            return;
        }

        // Save the configuration to data-mapping.json
        File configFile = LittleFS.open(DATA_MAPPING_FILE, "w");
        if (!configFile) {
            request->send(500, "application/json", "{\"error\":\"❌ Failed to save configuration\"}");
            return;
        }

        serializeJson(doc, configFile);
        configFile.close();
        request->send(200, "application/json", "{\"status\":\"✅ Configuration saved\"}");
    });
///////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////// LoRa netwwork //////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Endpoint: Load LoRa configuration
// void configureLoRaModule(int address, int networkID, int power, long frequency, int spreadingFactor, int bandwidth, int codingRate, int preamble);
// String getLoRaModuleConfig();
// Endpoint: Configure LoRa module
server.on("/configure-lora", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
[](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    String body = String((char*)data).substring(0, len);
    DynamicJsonDocument doc(1024);

    DeserializationError error = deserializeJson(doc, body);
    if (error) {
        request->send(400, "application/json", "{\"error\":\"❌ Invalid JSON\"}");
        return;
    }


    int address = doc["address"] | 0;
    int networkID = doc["networkID"] | 0;
    int power = doc["power"] | 0;
    long frequency = doc["frequency"] | 0;
    int spreadingFactor = doc["spreadingFactor"] | 0;
    int bandwidth = doc["bandwidth"] | 0;
    int codingRate = doc["codingRate"] | 0;
    int preamble = doc["preamble"] | 0;

    // Call the configureLoRaModule function with the provided parameters
    configLoRa.configureLoRaModule(address, networkID, power, frequency, spreadingFactor, bandwidth, codingRate, preamble);

    request->send(200, "application/json", "{\"status\":\"✅ LoRa configuration applied\"}");
});


// Endpoint: Load LoRa module configuration
server.on("/load-lora-config", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Call the getLoRaModuleConfig function to retrieve the configuration
    String config = configLoRa.getLoRaModuleConfig();
    Serial.println(config);
    request->send(200, "application/json", config);
});
    // configureE32(String json)
    // Endpoint: Configure E32 with JSON
    server.on("/configure-e32", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        String body = String((char*)data).substring(0, len);
        DynamicJsonDocument doc(1024);

        DeserializationError error = deserializeJson(doc, body);
        if (error) {
            request->send(400, "application/json", "{\"error\":\"❌ Invalid JSON\"}");
            return;
        }

        // Convert JSON document to string
        String jsonConfig;
        serializeJson(doc, jsonConfig);

        // Call the configureE32 function with the JSON string
        // configLoRa.configureE32(jsonConfig);

        request->send(200, "application/json", "{\"status\":\"✅ Configuration applied\"}");
    });
///////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////// Mesh netwwork //////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////


// Endpoint: Load Mesh configuration from file
server.on("/load-mesh-config", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (LittleFS.exists(CONFIG_FILE)) {
        File file = LittleFS.open(CONFIG_FILE, "r");
        if (file) {
            String json = file.readString();
            file.close();
            request->send(200, "application/json", json);
        } else {
            request->send(500, "application/json", "{\"error\":\"❌ Failed to open mesh.json\"}");
        }
    } else {
        request->send(404, "application/json", "{\"error\":\"❌ mesh.json not found\"}");
    }
});

// Endpoint: Save Mesh configuration to file
server.on("/save-mesh-config", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
[](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    String body = String((char*)data).substring(0, len);
    DynamicJsonDocument doc(1024);

    DeserializationError error = deserializeJson(doc, body);
    if (error) {
        request->send(400, "application/json", "{\"error\":\"❌ Invalid JSON\"}");
        return;
    }

    // Save the configuration to mesh.json
    File configFile = LittleFS.open(CONFIG_FILE, "w");
    if (!configFile) {
        request->send(500, "application/json", "{\"error\":\"❌ Failed to save configuration\"}");
        return;
    }

    serializeJson(doc, configFile);
    configFile.close();
    request->send(200, "application/json", "{\"status\":\"✅ Configuration saved\"}");
});
///////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////// WIFI MQTT //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
server.on("/load-wifi-mqtt-config", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (LittleFS.exists(WIFIMQTT_FILE)) {
        File file = LittleFS.open(WIFIMQTT_FILE, "r");
        if (file) {
            String json = file.readString();
            file.close();
            request->send(200, "application/json", json);
        } else {
            request->send(500, "application/json", "{\"error\":\"❌ Failed to open config.json\"}");
        }
    } else {
        request->send(404, "application/json", "{\"error\":\"❌ config.json not found\"}");
    }
});
server.on("/save-wifi-mqtt-config", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
[](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    String body = String((char*)data).substring(0, len);
    DynamicJsonDocument doc(1024);

    DeserializationError error = deserializeJson(doc, body);
    if (error) {
        request->send(400, "application/json", "{\"error\":\"❌ Invalid JSON\"}");
        return;
    }

    // Lưu cấu hình vào file
    File configFile = LittleFS.open(WIFIMQTT_FILE, "w");
    if (!configFile) {
        request->send(500, "application/json", "{\"error\":\"❌ Failed to save configuration\"}");
        return;
    }

    serializeJson(doc, configFile);
    configFile.close();
#ifdef USE_MQTT
    // Cập nhật cấu hình toàn cục
    mqttEnable = doc["mqttEnable"] | true;
    mqttHost = doc["mqttHost"] | "broker.hivemq.com";
    mqttPort = doc["mqttPort"] | 1883;
    mqttUser = doc["mqttUser"] | "username";
    mqttPass = doc["mqttPass"] | "password";
    wifiMode = doc["wifiMode"] | "STA";
    ssid = doc["ssid"] | "yourSSID";
    pass = doc["pass"] | "yourPassword";
    conId = doc["conId"] | "b8e54d33-b34a-45ab-b76f-62c8a9abc6c4";
    mqttTopic = doc["mqttTopic"] | "test/topic";
    mqttTopicSub = doc["mqttTopicSub"] | "test/topic/sub";
    mqttKeepAlive = doc["mqttKeepAlive"] | 60;
    mqttCleanSession = doc["mqttCleanSession"] | true;
    mqttQos = doc["mqttQos"] | 0;
    mqttRetain = doc["mqttRetain"] | false;
    mqttLwtTopic = doc["mqttLwtTopic"] | "iSoftMesh/LWT";
    mqttLwtMessage = doc["mqttLwtMessage"] | "Offline";
    mqttLwtQos = doc["mqttLwtQos"] | 1;
    mqttLwtRetain = doc["mqttLwtRetain"] | false;
    mqttLwtEnabled = doc["mqttLwtEnabled"] | false;
#endif//USE_MQTT
    request->send(200, "application/json", "{\"status\":\"✅ Configuration saved\"}");
});
///////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////// MODBUS /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////   
    // Endpoint: Load Modbus configuration from file
    server.on("/load-modbus-config", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (LittleFS.exists(MODBUS_FILE)) {
            File file = LittleFS.open(MODBUS_FILE, "r");
            if (file) {
                String json = file.readString();
                file.close();
                request->send(200, "application/json", json);
            } else {
                request->send(500, "application/json", "{\"error\":\"❌ Failed to open modbus.json\"}");
            }
        } else {
            request->send(404, "application/json", "{\"error\":\"❌ modbus.json not found\"}");
        }
    });
    server.on("/modbus-config", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        String body = String((char*)data).substring(0, len); // Chuyển dữ liệu thành chuỗi
        // Ghi toàn bộ nội dung từ body vào file MODBUS_FILE
        File configFile = LittleFS.open(MODBUS_FILE, "w");
        if (!configFile) {
            request->send(500, "application/json", "{\"error\":\"❌ Failed to save configuration\"}");
            return;
        }
        configFile.print(body);
        configFile.close();
        request->send(200, "application/json", "{\"status\":\"✅ Configuration saved\"}");
        
    });
    // API: Save Modbus Data Block
    server.on("/save-datablocks", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        String body = String((char*)data).substring(0, len);
        File file = LittleFS.open("/ModbusDataBlock.json", "w");
        if (!file) {
            request->send(500, "application/json", "{\"error\":\"❌ Failed to save data block\"}");
            return;
        }
        file.print(body);
        file.close();
        request->send(200, "application/json", "{\"status\":\"✅ Data block saved\"}");
    });

    // API: Read Modbus Data Block
    server.on("/datablocks", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (LittleFS.exists("/ModbusDataBlock.json")) {
            File file = LittleFS.open("/ModbusDataBlock.json", "r");
            if (file) {
                String json = file.readString();
                file.close();
                request->send(200, "application/json", json);
            } else {
                request->send(500, "application/json", "{\"error\":\"❌ Failed to open data block\"}");
            }
        } else {
            request->send(404, "application/json", "{\"error\":\"❌ MobusDataBlock.json not found\"}");
        }
    });

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////// FILE SYSTEM /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////    
    // Khởi tạo FileSystem
    if (!LittleFS.begin()) {
        Serial.println("❌ Failed to initialize LittleFS");
        return;
    }
    
    // Endpoint: Danh sách tệp
    server.on("/list-files", HTTP_GET, [](AsyncWebServerRequest *request) {
        String json = "[";
        bool first = true;
        File root = LittleFS.open("/");
        File file = root.openNextFile();
        while (file) {
            if (permissionLevel == 2) { // staff
                if (String(file.name()) != "/dashboard.html") {
                    file = root.openNextFile();
                    continue;
                }
            } else if (permissionLevel == 3) { // admin
                if (!first) json += ",";
                json += "\"" + String(file.name()) + "\"";
                first = false;
                file = root.openNextFile();
            } else {
                file = root.openNextFile();
                continue;
            }
        }
        json += "]";
        // Add file size for each file
        if (permissionLevel == 3) {
            file = root.openNextFile();
            while (file) {
            if (!first) json += ",";
            json += "{\"name\":\"" + String(file.name()) + "\",\"size\":" + String(file.size()) + "}";
            first = false;
            file = root.openNextFile();
            }
        }
        json += ",{\"used_kb\":" + String(LittleFS.usedBytes() / 1024) +
            ",\"free_kb\":" + String((LittleFS.totalBytes() - LittleFS.usedBytes()) / 1024) + "}";
        request->send(200, "application/json", json);
    });

    // Endpoint: Tải lên tệp
    server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {},
        [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
            if (!index) {
                Serial.printf("Uploading file: %s\n", filename.c_str());
                File file = LittleFS.open("/" + filename, "w");
                if (!file) {
                    Serial.println("❌ Failed to open file for writing");
                    return;
                }
                file.close();
            }
            File file = LittleFS.open("/" + filename, "a");
            if (file) {
                file.write(data, len);
                file.close();
            }
            if (final) {
                Serial.printf("📤✅ File upload complete: %s\n", filename.c_str());
                request->send(200, "text/plain", "File uploaded");
            }
        });

    // Endpoint: Xóa tệp
    server.on("/delete", HTTP_DELETE, [](AsyncWebServerRequest *request) {
        if (request->hasParam("file")) {
            String fileName = request->getParam("file")->value();
            Serial.println("> delete file " + fileName);
            if (LittleFS.remove("/" + fileName)) {
                request->send(200, "text/plain", "🗑️ File deleted ✅");
            } else {
                request->send(500, "text/plain", "❌ Failed to delete file");
            }
        } else {
            request->send(400, "text/plain", "❌ File parameter missing");
        }
    });
    server.on("/tool", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream("text/html");
        response->print("<!DOCTYPE html><html><head><title>System File Tool</title></head><style>");
        response->print("body { font-family: Arial, sans-serif; margin: 20px; }");
        response->print("h1 { color: #333; }");
        response->print("button { margin-top: 10px; padding: 10px; background-color: #4CAF50; color: white; border: none; cursor: pointer; }");
        response->print("button:hover { background-color: #45a049; }");
        response->print("table { width: 100%; border-collapse: collapse; margin-top: 20px; }");
        response->print("th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }");
        response->print("th { background-color: #f2f2f2; }");
        response->print("tr:hover { background-color: #f5f5f5; }");
        response->print("td button { background-color: #f44336; color: white; border: none; cursor: pointer; padding: 5px 10px; }");
        response->print("td button:hover { background-color: #d32f2f; }");
        response->print("</style></head><body><CENTER>");
        response->print("<h2>File Manager</h2>");
        response->print("<table><thead><tr><th>File Name</th><th>Action</th></tr></thead>");
        response->print("<tbody id=\"fileList\"></tbody></table>");
        response->print("<div><h3>Upload File</h3>");
        response->print("<input type='file' id='fileInput'><button onclick='uploadFile()'>Upload</button></div></CENTER>");
        response->print("<script>");
        response->print("function fetchFileList() {");
        response->print("fetch('/list-files').then(response => response.json()).then(files => {");
        response->print("const fileList = document.getElementById('fileList'); fileList.innerHTML = '';");
        response->print("files.forEach(file => {");
        response->print("const row = document.createElement('tr');");
        response->print("const fileNameCell = document.createElement('td'); fileNameCell.textContent = file;");
        response->print("const actionCell = document.createElement('td');");
        response->print("const deleteButton = document.createElement('button'); deleteButton.textContent = 'Delete';");
        response->print("deleteButton.onclick = () => deleteFile(file); actionCell.appendChild(deleteButton);");
        response->print("row.appendChild(fileNameCell); row.appendChild(actionCell); fileList.appendChild(row); }); }); }");
        response->print("function uploadFile() {");
        response->print("const fileInput = document.getElementById('fileInput'); const file = fileInput.files[0];");
        response->print("if (!file) { alert('Please select a file to upload.'); return; }");
        response->print("const formData = new FormData(); formData.append('file', file);");
        response->print("fetch('/upload', { method: 'POST', body: formData }).then(response => {");
        response->print("if (response.ok) { alert('File uploaded successfully.'); fetchFileList(); } else { alert('Failed to upload file.'); } }); }");
        response->print("function deleteFile(fileName) {");
        response->print("fetch(`/delete?file=${encodeURIComponent(fileName)}`, { method: 'DELETE' }).then(response => {");
        response->print("if (response.ok) { alert('File deleted successfully.'); fetchFileList(); } else { alert('Failed to delete file.'); } }); }");
        response->print("fetchFileList();");
        response->print("</script></body></html>");
        request->send(response);
    });
    server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request){
        if(permissionLevel < 3 && permissionLevel > 0){
            request->send(403, "text/plain", "❌ Access Denied");
            return;
        }
        if(permissionLevel == 3){
            request->send(LittleFS, "/index.html", String(), false, processors);
        }
        // connectClinent = true;
      });
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        // request->send(LittleFS, "/index.html", String(), false, processors);
        if (permissionLevel == 0) {
            // Serve login page with WebSocket
            // Check if any certification file is missing
            if (!LittleFS.exists("/certification1.bin") ||
                !LittleFS.exists("/certification2.bin") ||
                !LittleFS.exists("/certification3.bin")) {
                AsyncResponseStream *response = request->beginResponseStream("text/html");
                response->print(R"rawliteral(
                <!DOCTYPE html>
                <html>
                <head>
                    <title>Set Initial Passwords</title>
                    <meta charset="UTF-8">
                    <style>
                        body { font-family: Arial, sans-serif; background: #f2f2f2; }
                        .container { max-width: 400px; margin: 80px auto; background: #fff; padding: 30px 25px; border-radius: 8px; box-shadow: 0 2px 8px #aaa; }
                        h2 { text-align: center; margin-bottom: 20px; }
                        input[type=password] { width: 100%; padding: 10px; margin: 8px 0 16px 0; border: 1px solid #ccc; border-radius: 4px; }
                        button { width: 100%; padding: 10px; background: #4CAF50; color: #fff; border: none; border-radius: 4px; font-size: 16px; }
                        .error { color: #d32f2f; text-align: center; margin-bottom: 10px; }
                        .success { color: #388e3c; text-align: center; margin-bottom: 10px; }
                    </style>
                </head>
                <body>
                    <div class="container">
                        <h2>Set Initial Passwords</h2>
                        <div id="msg" class="error"></div>
                        <input type="password" id="userPass" placeholder="User Password">
                        <input type="password" id="staffPass" placeholder="Staff Password">
                        <input type="password" id="adminPass" placeholder="Admin Password">
                        <button onclick="setPasswords()">Save Passwords</button>
                    </div>
                    <script>
                        function setPasswords() {
                            let userPass = document.getElementById('userPass').value;
                            let staffPass = document.getElementById('staffPass').value;
                            let adminPass = document.getElementById('adminPass').value;
                            if (!userPass || !staffPass || !adminPass) {
                                document.getElementById('msg').textContent = "All fields are required!";
                                return;
                            }
                            fetch('/set-initial-passwords', {
                                method: 'POST',
                                headers: {'Content-Type': 'application/json'},
                                body: JSON.stringify({
                                    user: userPass,
                                    staff: staffPass,
                                    admin: adminPass
                                })
                            }).then(r => r.json()).then(res => {
                                if (res.status === "ok") {
                                    document.getElementById('msg').className = "success";
                                    document.getElementById('msg').textContent = "Passwords set successfully! Please reload.";
                                    setTimeout(() => { window.location.reload(); }, 1500);
                                } else {
                                    document.getElementById('msg').className = "error";
                                    document.getElementById('msg').textContent = "Failed to set passwords!";
                                }
                            });
                        }
                    </script>
                </body>
                </html>
                )rawliteral");
                request->send(response);
                return;
            }
            AsyncResponseStream *response = request->beginResponseStream("text/html");
            response->print(R"rawliteral(
                <!DOCTYPE html>
                <html>
                <head>
                    <title>Login</title>
                    <meta charset="UTF-8">
                    <style>
                        body { font-family: Arial, sans-serif; background: #f2f2f2; }
                        .login-container { max-width: 350px; margin: 80px auto; background: #fff; padding: 30px 25px; border-radius: 8px; box-shadow: 0 2px 8px #aaa; }
                        h2 { text-align: center; margin-bottom: 20px; }
                        input[type=text], input[type=password] { width: 100%; padding: 10px; margin: 8px 0 16px 0; border: 1px solid #ccc; border-radius: 4px; }
                        button { width: 100%; padding: 10px; background: #4CAF50; color: #fff; border: none; border-radius: 4px; font-size: 16px; }
                        .error { color: #d32f2f; text-align: center; margin-bottom: 10px; }
                    </style>
                </head>
                <body>
                    <div class="login-container">
                        <h2>Login</h2>
                        <div id="error" class="error"></div>
                        <input type="text" id="user" placeholder="Username" autocomplete="username">
                        <input type="password" id="password" placeholder="Password" autocomplete="current-password">
                        <button onclick="login()">Login</button>
                    </div>
                    <script>
                        let ws;
                        function connectWS() {
                            ws = new WebSocket('ws://' + location.host + '/ws');
                            ws.onmessage = function(event) {
                                let msg = JSON.parse(event.data);
                                if (msg.login === "success") {
                                    window.location.href = "/index.html";
                                } else if (msg.login === "fail") {
                                    document.getElementById('error').textContent = "Invalid username or password!";
                                }
                            };
                            ws.onclose = function() {
                                setTimeout(connectWS, 1000);
                            };
                        }
                        connectWS();
                        function login() {
                            let user = document.getElementById('user').value;
                            let password = document.getElementById('password').value;
                            if (ws && ws.readyState === 1) {
                                ws.send(JSON.stringify({login: true, user: user, password: password}));
                            }
                        }
                        document.getElementById('password').addEventListener('keyup', function(e) {
                            if (e.key === 'Enter') login();
                        });
                    </script>
                </body>
                </html>
            )rawliteral");
            request->send(response);
        }
        if (permissionLevel == 3) {
            request->send(LittleFS, "/index.html", String(), false, processors);
        }else {
            request->send(LittleFS, "/dashboard.html", String(), false, processors);
        }
        // connectClinent = true;
    });
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/style.css", "text/css");
    });
    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/script.js", "application/javascript");
    });
    server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/favicon.ico", "image/x-icon");
    });
    // server.on("/SAVE", [](AsyncWebServerRequest *request ){saveData();request->send(200, "text/plain", "OK");});
    server.on("/connecttest.txt", [](AsyncWebServerRequest *request) { request->redirect("http://logout.net"); });	// windows 11 captive portal workaround
	server.on("/wpad.dat", [](AsyncWebServerRequest *request) { request->send(404); });								// Honestly don't understand what this is but a 404 stops win 10 keep calling this repeatedly and panicking the esp32 :)

	// Background responses: Probably not all are Required, but some are. Others might speed things up?
	// A Tier (commonly used by modern systems)
	server.on("/generate_204", [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });		   // android captive portal redirect
	server.on("/redirect", [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });			   // microsoft redirect
	server.on("/hotspot-detect.html", [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });  // apple call home
	server.on("/canonical.html", [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });	   // firefox captive portal call home
	server.on("/success.txt", [](AsyncWebServerRequest *request) { request->send(200); });					   // firefox captive portal call home
	server.on("/ncsi.txt", [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });			   // windows call home
  // server.on ("/download.csv", [](AsyncWebServerRequest *request) {handle_web_command_silent(request);});
	// B Tier (uncommon)
	 server.on("/chrome-variations/seed",[](AsyncWebServerRequest *request){request->send(200);}); //chrome captive portal call home
	 server.on("/service/update2/json",[](AsyncWebServerRequest *request){request->send(200);}); //firefox?
	//  server.on("/chat",[](AsyncWebServerRequest *request){request->send(404);}); //No stop asking Whatsapp, there is no internet connection
	//  server.on("/startpage",[](AsyncWebServerRequest *request){request->redirect(localIPURL);});

	// return 404 to webpage icon
	server.on("/favicon.ico", [](AsyncWebServerRequest *request) { request->send(404); });	// webpage icon


    server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);//only when requested from AP
  	//======================== Webserver ========================
	// WARNING IOS (and maybe macos) WILL NOT POP UP IF IT CONTAINS THE WORD "Success" https://www.esp8266.com/viewtopic.php?f=34&t=4398
	// SAFARI (IOS) IS STUPID, G-ZIPPED FILES CAN'T END IN .GZ https://github.com/homieiot/homie-esp8266/issues/476 this is fixed by the webserver serve static function.
	// SAFARI (IOS) there is a 128KB limit to the size of the HTML. The HTML can reference external resources/images that bring the total over 128KB
	// SAFARI (IOS) popup browser has some severe limitations (javascript disabled, cookies disabled)

    server.begin();
    Serial.println("🔗 Web server started.");
  }

  