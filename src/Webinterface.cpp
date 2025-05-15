#include "WebInterface.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "LoRa.h"
LoRaFunction configLoRa;

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
    }
  }
  void WebinterFace::SendMessageToClient(const String& message) {
    ws.textAll(message);
  }
  void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
               void *arg, uint8_t *data, size_t len) {
    switch (type) {
      case WS_EVT_CONNECT:
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        socketConnected = true;
        break;
      case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
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
/////////////////////////////////////////
//////////////////////////////////////////////////////////////////
////////////////////////////////////////// Data Mapping //////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
    server.on("/data-mapping", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (LittleFS.exists(DATA_MAPPING_FILE)) {
            File file = LittleFS.open(DATA_MAPPING_FILE, "r");
            if (file) {
                String json = file.readString();
                file.close();
                request->send(200, "application/json", json);
            } else {
                request->send(500, "application/json", "{\"error\":\"Failed to open data-mapping.json\"}");
            }
        } else {
            request->send(404, "application/json", "{\"error\":\"data-mapping.json not found\"}");
        }
    });

    server.on("/save-data-mapping", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        String body = String((char*)data).substring(0, len);
        DynamicJsonDocument doc(1024);

        DeserializationError error = deserializeJson(doc, body);
        if (error) {
            request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
            return;
        }

        // Save the configuration to data-mapping.json
        File configFile = LittleFS.open(DATA_MAPPING_FILE, "w");
        if (!configFile) {
            request->send(500, "application/json", "{\"error\":\"Failed to save configuration\"}");
            return;
        }

        serializeJson(doc, configFile);
        configFile.close();
        request->send(200, "application/json", "{\"status\":\"Configuration saved\"}");
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
        request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
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

    request->send(200, "application/json", "{\"status\":\"LoRa configuration applied\"}");
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
            request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
            return;
        }

        // Convert JSON document to string
        String jsonConfig;
        serializeJson(doc, jsonConfig);

        // Call the configureE32 function with the JSON string
        // configLoRa.configureE32(jsonConfig);

        request->send(200, "application/json", "{\"status\":\"Configuration applied\"}");
    });
///////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////// Mesh netwwork //////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Config {
    uint8_t BrokerAddress[6];// Array to store the Broker MAC address
    int wifiChannel;    // WiFi channel for ESP-NOW communication
    int id;             // Node ID
    int netId;          // Network ID
    String role;        // "Broker", "Node", or "Repeater"
    bool debug;         // Enable/disable debug prints
    JsonArray macSlaves;// Array of MAC addresses for slaves
    int dataVersion; // Data version 0: Lookline v1, 1: Lookline v2, 2: Modbus Register, 3: Serial TTL
    byte boardModel;
} MeshConfig;

// Endpoint: Load Mesh configuration from file
server.on("/load-mesh-config", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (LittleFS.exists(CONFIG_FILE)) {
        File file = LittleFS.open(CONFIG_FILE, "r");
        if (file) {
            String json = file.readString();
            file.close();
            request->send(200, "application/json", json);
        } else {
            request->send(500, "application/json", "{\"error\":\"Failed to open mesh.json\"}");
        }
    } else {
        request->send(404, "application/json", "{\"error\":\"mesh.json not found\"}");
    }
});

// Endpoint: Save Mesh configuration to file
server.on("/save-mesh-config", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
[](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    String body = String((char*)data).substring(0, len);
    DynamicJsonDocument doc(1024);

    DeserializationError error = deserializeJson(doc, body);
    if (error) {
        request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }

    // Save the configuration to mesh.json
    File configFile = LittleFS.open(CONFIG_FILE, "w");
    if (!configFile) {
        request->send(500, "application/json", "{\"error\":\"Failed to save configuration\"}");
        return;
    }

    serializeJson(doc, configFile);
    configFile.close();
    request->send(200, "application/json", "{\"status\":\"Configuration saved\"}");
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
            request->send(500, "application/json", "{\"error\":\"Failed to open config.json\"}");
        }
    } else {
        request->send(404, "application/json", "{\"error\":\"config.json not found\"}");
    }
});
server.on("/save-wifi-mqtt-config", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
[](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    String body = String((char*)data).substring(0, len);
    DynamicJsonDocument doc(1024);

    DeserializationError error = deserializeJson(doc, body);
    if (error) {
        request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }

    // Lưu cấu hình vào file
    File configFile = LittleFS.open(WIFIMQTT_FILE, "w");
    if (!configFile) {
        request->send(500, "application/json", "{\"error\":\"Failed to save configuration\"}");
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
    request->send(200, "application/json", "{\"status\":\"Configuration saved\"}");
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
                request->send(500, "application/json", "{\"error\":\"Failed to open modbus.json\"}");
            }
        } else {
            request->send(404, "application/json", "{\"error\":\"modbus.json not found\"}");
        }
    });
    server.on("/modbus-config", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        String body = String((char*)data).substring(0, len); // Chuyển dữ liệu thành chuỗi
        DynamicJsonDocument doc(1024);

        DeserializationError error = deserializeJson(doc, body);
        if (error) {
            request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
            return;
        }

        // Xử lý dữ liệu JSON
        String role = doc["role"] | "slave";
        String com = doc["Com"] | "RS485";
        int id = doc["id"] | 1;
        JsonArray slaveip = doc["slaveip"];
        JsonArray tags = doc["Tag"];
        JsonArray values = doc["Value"];
        JsonArray types = doc["Type"];

        if (!slaveip || !tags || !values || !types) {
            request->send(400, "application/json", "{\"error\":\"Missing required fields\"}");
            return;
        }

        // Log cấu hình
        // Serial.println("Modbus Configuration:");
        // Serial.println("Role: " + role);
        // Serial.println("Com: " + com);
        // Serial.println("ID: " + String(id));
        // Serial.print("Slave IP: ");
        for (size_t i = 0; i < slaveip.size(); i++) {
            Serial.print(slaveip[i].as<int>());
            if (i < slaveip.size() - 1) Serial.print(".");
        }
        Serial.println();
        for (size_t i = 0; i < tags.size(); i++) {
            Serial.println("Tag: " + String(tags[i].as<int>()) + ", Value: " + String(values[i].as<int>()) + ", Type: " + String(types[i].as<int>()));
        }
        // Save the configuration to modbus.json
        File configFile = LittleFS.open(MODBUS_FILE, "w");
        if (!configFile) {
            Serial.println("Failed to open modbus.json for writing");
            request->send(500, "application/json", "{\"error\":\"Failed to save configuration\"}");
            return;
        }

        // Serialize the JSON document and write it to the file
        serializeJson(doc, configFile);
        configFile.close();
        Serial.println("Configuration saved to modbus.json");
        // Gửi phản hồi
        request->send(200, "application/json", "{\"status\":\"Configuration saved\"}");
        delay(3000);ESP.restart();
    });
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////// FILE SYSTEM /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////    
    // Khởi tạo FileSystem
    if (!LittleFS.begin()) {
        Serial.println("Failed to initialize LittleFS");
        return;
    }
    
    // Endpoint: Danh sách tệp
    server.on("/list-files", HTTP_GET, [](AsyncWebServerRequest *request) {
        String json = "[";
        bool first = true;
        File root = LittleFS.open("/");
        File file = root.openNextFile();
        while (file) {
            if (!first) json += ",";
            json += "\"" + String(file.name()) + "\"";
            first = false;
            file = root.openNextFile();
        }
        json += "]";
        request->send(200, "application/json", json);
    });

    // Endpoint: Tải lên tệp
    server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {},
        [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
            if (!index) {
                Serial.printf("Uploading file: %s\n", filename.c_str());
                File file = LittleFS.open("/" + filename, "w");
                if (!file) {
                    Serial.println("Failed to open file for writing");
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
                Serial.printf("File upload complete: %s\n", filename.c_str());
                
            }
        });

    // Endpoint: Xóa tệp
    server.on("/delete", HTTP_DELETE, [](AsyncWebServerRequest *request) {
        if (request->hasParam("file")) {
            String fileName = request->getParam("file")->value();
            Serial.println("> delete file " + fileName);
            if (LittleFS.remove("/" + fileName)) {
                request->send(200, "text/plain", "File deleted");
            } else {
                request->send(500, "text/plain", "Failed to delete file");
            }
        } else {
            request->send(400, "text/plain", "File parameter missing");
        }
    });
    server.on("/tool", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream("text/html");
        response->print("<!DOCTYPE html><html><head><title>Captive Portal</title></head><style>");
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
        response->print("<h1>Captive Portal</h1>");
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
        request->send(LittleFS, "/index.html", String(), false, processors);
        // connectClinent = true;
      });
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/index.html", String(), false, processors);
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
    Serial.println("Web server started.");
  }

  