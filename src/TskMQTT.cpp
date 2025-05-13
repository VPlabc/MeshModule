#include "TskMQTT.h"
#include "WebInterface.h"
WebinterFace webInterface;
#define MQTT_Client
#ifdef MQTT_Client
AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;
#endif//MQTT_Client
#ifdef MQTTV1
WiFiClientSecure net;
MQTTClient mqttClient;
void messageReceived(String &topic, String &payload) {
    Serial.println("incoming: " + topic + " - " + payload);
  
    // Note: Do not use the client in the callback to publish, subscribe or
    // unsubscribe as it may cause deadlocks when other things arrive while
    // sending and receiving acknowledgments. Instead, change a global variable,
    // or push to a queue and handle it in the loop after calling `client.loop()`.
  }
void setupMQTT() {
    // net.setInsecure();

    mqttClient.begin(mqttHost.c_str(), mqttPort, net);
    if (!mqttClient.connected()) {
        if (mqttClient.connect(conId.c_str(), mqttUser.c_str(), mqttPass.c_str())) {
            Serial.println("MQTT connected.");
            mqttClient.onMessage(messageReceived);
            mqttClient.subscribe(mqttTopicSub.c_str());
        } else {
            Serial.println("MQTT connection failed.");
        }
    }
}
static int counter = 0;
void WifiMqttConfig::setupWiFi() {
    static int reconnectAttempts = 0; // Biến đếm số lần thử kết nối lại

    if (wifiMode == "STA") {
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid.c_str(), pass.c_str());
        Serial.print("Connecting to WiFi");
        while (WiFi.status() != WL_CONNECTED) {
            static int counter = 0;
            if (counter++ < 20) {
                Serial.print(".");
                delay(500); // Thêm delay để tránh vòng lặp quá nhanh
            } else {
                Serial.println("Connect to WiFi fail.");
                reconnectAttempts++;
                break;
            }
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("WiFi connected.");
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());
            reconnectAttempts = 0; // Reset số lần thử khi kết nối thành công
            webInterface.setupWebConfig();
        } else if (reconnectAttempts >= 3) {
            Serial.println("Failed to connect to WiFi after 3 attempts. Switching to AP mode.");
            // Cập nhật cấu hình sang chế độ AP
            wifiMode = "AP";
            DynamicJsonDocument doc(1024);
            doc["mqttEnable"] = mqttEnable;
            doc["mqttHost"] = mqttHost;
            doc["mqttPort"] = mqttPort;
            doc["mqttUser"] = mqttUser;
            doc["mqttPass"] = mqttPass;
            doc["wifiMode"] = wifiMode; // Lưu chế độ WiFi (STA hoặc AP)
            doc["ssid"] = ssid;
            doc["pass"] = pass;
            doc["conId"] = conId;
            doc["mqttTopic"] = mqttTopic;
            doc["mqttTopicSub"] = mqttTopicSub;
        
            File configFile = LittleFS.open(WIFIMQTT_FILE, "w");
            if (!configFile) {
                Serial.println("Failed to open config file for writing.");
                return;
            }
            serializeJson(doc, configFile);
            configFile.close();
            
        }
    }else {
        mqttEnable = false;
    }
}
void WifiMqttConfig::setup() {

    if (!LittleFS.begin()) {
        Serial.println("Failed to initialize LittleFS");
        return;
    }

    // Tải cấu hình từ file
    if (LittleFS.exists(WIFIMQTT_FILE)) {
        File configFile = LittleFS.open(WIFIMQTT_FILE, "r");
        if (configFile) {
            DynamicJsonDocument doc(1024);
            deserializeJson(doc, configFile);
            configFile.close();

            mqttEnable = doc["mqttEnable"] | true;
            mqttHost = doc["mqttHost"] | "broker.hivemq.com";
            mqttPort = doc["mqttPort"] | 1883;
            mqttUser = doc["mqttUser"] | "username";
            mqttPass = doc["mqttPass"] | "password";
            wifiMode = doc["wifiMode"] | "STA";
            ssid = doc["ssid"] | "I-Soft";
            pass = doc["pass"] | "i-soft@2023";
            conId = doc["conId"] | "b8e54d33-b34a-45ab-b76f-62c8a9abc6c4";
            mqttTopic = doc["mqttTopic"] | "test/topic";
            mqttTopicSub = doc["mqttTopicSub"] | "test/topic/sub";
        }
    }
    if(mqttEnable){
        Serial.println("SSID: " + ssid);
        Serial.println("Password: " + pass);
        Serial.println("WiFi Mode: " + wifiMode);
        Serial.println("MQTT enabled.");
        Serial.print("MQTT Host: ");
        Serial.println(mqttHost);
        Serial.print("MQTT Port: ");
        Serial.println(mqttPort);
        Serial.print("MQTT User: ");
        Serial.println(mqttUser);
        Serial.print("MQTT Password: ");
        Serial.println(mqttPass);
        Serial.println("ConId: " + conId);
        Serial.print("MQTT Topic: ");
        Serial.println(mqttTopic);
        Serial.print("MQTT Topic Sub: ");
        Serial.println(mqttTopicSub);
    
        setupWiFi();
        if (mqttEnable) {
            setupMQTT();
        }
    }

    // server.begin();
    // Serial.println("Web server started.");
}

void WifiMqttConfig::loop() {
    if (mqttEnable) {
        static long lastTimeConnect = 0;
        if (millis() - lastTimeConnect > 10000) {
            lastTimeConnect = millis();
            Serial.println("MQTT loop.");
            if(WiFi.status() != WL_CONNECTED) {
                Serial.println("WiFi not connected, trying to reconnect...");
                setupWiFi();
                }
                else{
                    mqttClient.loop();
                    if (!mqttClient.connected()) {
                        setupMQTT();
                    }
                }
            }
    }
    
    // server.handleClient();
}
#endif// MQTT_V1

// bool mqttIsConnected = false;

void WifiMqttConfig::saveJsonToWifiMqttFile(char &jsonString,bool debug, fs::FS &FileSystem) {
    File file = FileSystem.open(WIFIMQTT_FILE, "w");
    if (!file) {
        if (debug) Serial.println("Failed to open wifi_mqtt.json for writing.");
        return;
    }

    size_t bytesWritten = file.print(jsonString);
    file.close();

    if (bytesWritten == 0) {
        if (debug) Serial.println("Failed to write JSON to wifi_mqtt.json.");
    } else {
        if (debug) Serial.println("JSON saved to wifi_mqtt.json successfully.");
    }
}

// Hàm tải file wifi_mqtt.json và truyền chuỗi JSON vào WifiMqttInit
String WifiMqttConfig::loadWifiMqttConfig(bool debug, fs::FS &FileSystem) {
    String jsonString;
    if (!FileSystem.exists(WIFIMQTT_FILE)) {
        if (debug) Serial.println("wifi_mqtt.json not found.");
        // Nếu file không tồn tại, tạo file với cấu hình mặc định
        DynamicJsonDocument doc(1024);
        doc["mqttEnable"] = true;
        doc["mqttHost"] = "broker.hivemq.com";
        doc["mqttPort"] = 1883;
        doc["mqttUser"] = "username";
        doc["mqttPass"] = "password";
        doc["wifiMode"] = "STA";
        doc["ssid"] = "I-Soft";
        doc["pass"] = "i-soft@2023";
        doc["conId"] = "b8e54d33-b34a-45ab-b76f-62c8a9abc6c4";
        doc["mqttTopic"] = "test/topic";
        doc["mqttTopicSub"] = "test/topic/sub";
        doc["mqttKeepAlive"] = 60; // Default Keep Alive
        doc["mqttCleanSession"] = true; // Default Clean Session
        doc["mqttQos"] = 1; // Default QoS
        doc["mqttRetain"] = false; // Default Retain
        doc["mqttLwtTopic"] = "lwt/topic"; // Default Last Will Topic
        doc["mqttLwtMessage"] = "Offline"; // Default Last Will Message
        doc["mqttLwtQos"] = 1; // Default Last Will QoS
        doc["mqttLwtRetain"] = false; // Default Last Will Retain
        doc["mqttLwtEnabled"] = false; // Default Last Will Enabled

        // Lưu cấu hình mặc định vào file
        File configFile = FileSystem.open(WIFIMQTT_FILE, "w");
        if (configFile) {
            serializeJson(doc, configFile);
            configFile.close();
            Serial.println("Default configuration created.");
        } else {
            return "Failed to create default config";
        }

        // Trả về cấu hình mặc định
        serializeJson(doc, jsonString);
        return jsonString;
    }
    else{
        File file = FileSystem.open(WIFIMQTT_FILE, "r");
        if (!file) {
            if (debug) Serial.println("Failed to open wifi_mqtt.json for reading.");
            return "Failed to open wifi_mqtt.json for reading.";
        }
        else{
                while (file.available()) {
                    jsonString += char(file.read());
                }
                file.close();

                if (debug) {
                    Serial.println("Loaded JSON from wifi_mqtt.json:");
                    Serial.println(jsonString);
                }

        }
    }
    return jsonString;
}

#ifdef MQTT_Client

void WifiMqttConfig::MQTTPush(String Topic,String Payload) {
    if(WiFi.status() == WL_CONNECTED && mqttIsConnected)mqttClient.publish(Topic.c_str(), mqttQos, mqttRetain, Payload.c_str());
}
static int reconnectAttempts = 0; // Biến đếm số lần thử kết nối lại
void connectToWifi() {
    Serial.println("Connecting to Wi-Fi...");
    Serial.print("SSID: ");
    Serial.println(ssid);
    Serial.print("Password: ");
    Serial.println(pass);
    WiFi.begin(ssid.c_str(), pass.c_str()); 
    reconnectAttempts++;
    
  }
  
  void connectToMqtt() {
    Serial.println("Connecting to MQTT...");
    mqttClient.connect();
  }
  
  void WiFiEvent(WiFiEvent_t event) {
      Serial.printf("[WiFi-event] event: %d\n", event);
      switch(event) {
      case SYSTEM_EVENT_STA_GOT_IP:
        reconnectAttempts = 0;
          Serial.println("WiFi connected");
          Serial.println("IP address: ");
          Serial.println(WiFi.localIP());
          connectToMqtt();
          webInterface.setupWebConfig();
          Serial.print("WiFi Channel: ");
          Serial.println(WiFi.channel());
          xTimerStop(wifiReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
          xTimerStart(mqttReconnectTimer, 0);
          break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
          Serial.println("WiFi lost connection");
          
          xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
          xTimerStart(wifiReconnectTimer, 0);
          break;
      case SYSTEM_EVENT_ETH_START:
        LOGLN("ETH Started");
        // set eth hostname here
        //ETH.setHostname("iotdevice");
        break;
    case SYSTEM_EVENT_ETH_CONNECTED:
        LOGLN("ETH Connected");
        break;
    case SYSTEM_EVENT_ETH_GOT_IP:
        Serial.print("ETH MAC: ");
        //Serial.print(ETH.macAddress());
        Serial.print(", IPv4: ");
        //Serial.print(ETH.localIP());
        // if (ETH.fullDuplex())
        // {
        //     Serial.print(", FULL_DUPLEX");
        // }
        Serial.print(", ");
        // Serial.print(ETH.linkSpeed());
        LOGLN("Mbps");
        //eth_connected = true;

        break;
        case SYSTEM_EVENT_ETH_DISCONNECTED:
            LOGLN("ETH Disconnected");
            //eth_connected = false;
            break;
        case SYSTEM_EVENT_ETH_STOP:
            LOGLN("ETH Stopped");
            // eth_connected = false;
            break;

        case SYSTEM_EVENT_STA_CONNECTED:
            LOGLN("STA Connected");
            //SocketConnect = true;
            //xTimerStart(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi

            // ap_connected = false;SocketConnect = false;
            break;

        case SYSTEM_EVENT_AP_STADISCONNECTED:
            LOGLN("AP Disconnected");
            //ap_connected = false;
            //SocketConnect = false;
            break;
        case SYSTEM_EVENT_AP_STACONNECTED:
            LOGLN("AP Connected");
            //ap_connected = true;
            //SocketConnect = true;
            break;
        case SYSTEM_EVENT_AP_START:
            LOGLN("AP Start");
            break;
        case SYSTEM_EVENT_AP_STOP:
            LOGLN("AP Stop");
            break;

        } 
    }
  
#define RTC_Onl
#include "RTC_Online.h"
  void onMqttConnect(bool sessionPresent) {
    Serial.println("Connected to MQTT.");
    Serial.print("Session present: ");
    Serial.println(sessionPresent);
    uint16_t packetIdSub = mqttClient.subscribe(mqttTopicSub.c_str() , mqttQos);
    Serial.print("Subscribing at QoS " + String(mqttQos) + ", packetId: ");
    Serial.println(packetIdSub);
    // mqttClient.publish("test/lol", 0, true, "test 1");
    // Serial.println("Publishing at QoS 0");
        String TimeAt = String(Getyear) + "-" + String(Getmonth) + "-" + String(Getday) + " " + String(Gethour) + ":" + String(Getmin) + ":" + String(Getsec);
        String payload = "Node Start at: " + TimeAt + " with IP: " + String(WiFi.localIP().toString());
    uint16_t packetIdPub1 = mqttClient.publish(mqttTopicStat.c_str(), 1, true, payload.c_str());payload.clear();TimeAt.clear();
    Serial.print("Publishing at QoS " + String(mqttQos) + ", packetId: ");
    Serial.println(packetIdPub1);
    // Set Last Will and Testament (LWT) if enabled
    if (mqttLwtEnabled) {
        mqttClient.setWill(
            mqttLwtTopic.c_str(),
            mqttLwtQos,
            mqttLwtRetain,
            mqttLwtMessage.c_str()
        );
        Serial.println("MQTT Last Will and Testament (LWT) configured.");
    }

    // Configure MQTT keep-alive, clean session, QoS, and retain settings
    mqttClient.setKeepAlive(mqttKeepAlive);
    mqttClient.setCleanSession(mqttCleanSession);

    Serial.print("MQTT Keep Alive: ");
    Serial.println(mqttKeepAlive);
    Serial.print("MQTT Clean Session: ");
    Serial.println(mqttCleanSession);
    Serial.print("MQTT QoS: ");
    Serial.println(mqttQos);
    Serial.print("MQTT Retain: ");
    Serial.println(mqttRetain);
    mqttIsConnected = true;
    xTimerStop(mqttReconnectTimer, 0);
  }
  
  void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
    Serial.println("Disconnected from MQTT.");
    mqttIsConnected = false;
    if (WiFi.isConnected()) {
      xTimerStart(mqttReconnectTimer, 0);
    }
  }
  
  void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
    Serial.println("Subscribe acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
    Serial.print("  qos: ");
    Serial.println(qos);
  }
  
  void onMqttUnsubscribe(uint16_t packetId) {
    Serial.println("Unsubscribe acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
  }
  
  void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
    Serial.println("Publish received.");
    Serial.print("  topic: ");
    Serial.println(topic);
    Serial.print("  qos: ");
    Serial.println(properties.qos);
    Serial.print("  dup: ");
    Serial.println(properties.dup);
    Serial.print("  retain: ");
    Serial.println(properties.retain);
    Serial.print("  len: ");
    Serial.println(len);
    Serial.print("  index: ");
    Serial.println(index);
    Serial.print("  total: ");
    Serial.println(total);
  }
  
  void onMqttPublish(uint16_t packetId) {
    Serial.println("Publish acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
  }
  
  void WifiMqttConfig::setup() {
    if (LittleFS.exists(WIFIMQTT_FILE)) {
        File configFile = LittleFS.open(WIFIMQTT_FILE, "r");
        if (configFile) {
            DynamicJsonDocument doc(1024);
            deserializeJson(doc, configFile);
            configFile.close();

            mqttEnable = doc["mqttEnable"] | true;
            mqttHost = doc["mqttHost"] | "broker.hivemq.com";
            mqttPort = doc["mqttPort"] | 1883;
            mqttUser = doc["mqttUser"] | "username";
            mqttPass = doc["mqttPass"] | "password";
            wifiMode = doc["wifiMode"] | "AP";
            ssid = doc["ssid"] | "I-Soft";
            pass = doc["pass"] | "i-soft@2023";
            conId = doc["conId"] | "b8e54d33-b34a-45ab-b76f-62c8a9abc6c4";
            mqttTopic = doc["mqttTopic"] | "test/topic";
            mqttTopicSub = doc["mqttTopicSub"] | "test/topic/sub";
            mqttKeepAlive = doc["mqttKeepAlive"] | 60; // Default Keep Alive
            mqttCleanSession = doc["mqttCleanSession"] | true; // Default Clean Session
            mqttQos = doc["mqttQos"] | 1; // Default QoS
            mqttRetain = doc["mqttRetain"] | false; // Default Retain
            mqttLwtTopic = doc["mqttLwtTopic"] | "lwt/topic"; // Default Last Will Topic
            mqttLwtMessage = doc["mqttLwtMessage"] | "Offline"; // Default Last Will Message
            mqttLwtQos = doc["mqttLwtQos"] | 1; // Default Last Will QoS
            mqttLwtRetain = doc["mqttLwtRetain"] | false; // Default Last Will Retain
            mqttLwtEnabled = doc["mqttLwtEnabled"] | false; // Default Last Will Enabled
            timezone = doc["timezone"] | 7; // Default timezone
        }
    }else{
        Serial.println("wifi_mqtt.json not found.");
        // Nếu file không tồn tại, tạo file với cấu hình mặc định
        DynamicJsonDocument doc(1024);
        doc["mqttEnable"] = true;
        doc["mqttHost"] = "broker.hivemq.com";
        doc["mqttPort"] = 1883;
        doc["mqttUser"] = "username";
        doc["mqttPass"] = "password";
        doc["wifiMode"] = "STA";
        doc["ssid"] = "I-Soft";
        doc["pass"] = "i-soft@2023";
        doc["conId"] = "b8e54d33-b34a-45ab-b76f-62c8a9abc6c4";
        doc["mqttTopic"] = "test/topic";
        doc["mqttTopicSub"] = "test/topic/sub";
        doc["mqttKeepAlive"] = 60; // Default Keep Alive
        doc["mqttCleanSession"] = true; // Default Clean Session
        doc["mqttQos"] = 1; // Default QoS
        doc["mqttRetain"] = false; // Default Retain
        doc["mqttLwtTopic"] = "lwt/topic"; // Default Last Will Topic
        doc["mqttLwtMessage"] = "Offline"; // Default Last Will Message
        doc["mqttLwtQos"] = 1; // Default Last Will QoS
        doc["mqttLwtRetain"] = false; // Default Last Will Retain
        doc["mqttLwtEnabled"] = false; // Default Last Will Enabled
        doc["timezone"] = 7; // Default timezone
        

        // Lưu cấu hình mặc định vào file
        File configFile = LittleFS.open(WIFIMQTT_FILE, "w");
        if (configFile) {
            serializeJson(doc, configFile);
            configFile.close();
            Serial.println("Default configuration created.");
        } else {
            return;
        }
    }
    if(wifiMode == "STA" && mqttEnable){
        Serial.println("MQTT enabled.");
        Serial.print("MQTT Host: ");
        Serial.println(mqttHost);
        Serial.print("MQTT Port: ");
        Serial.println(mqttPort);
        Serial.print("MQTT User: ");
        Serial.println(mqttUser);
        Serial.print("MQTT Password: ");
        Serial.println(mqttPass);
        Serial.println("ConId: " + conId);
        Serial.print("MQTT Topic: ");
        Serial.println(mqttTopic);
        Serial.print("MQTT Topic Sub: ");
        Serial.println(mqttTopicSub);
        Serial.print("MQTT Keep Alive: ");
        Serial.println(mqttKeepAlive);
        Serial.print("MQTT Clean Session: ");
        Serial.println(mqttCleanSession);
        Serial.print("MQTT QoS: ");
        Serial.println(mqttQos);
        Serial.print("MQTT Retain: ");
        Serial.println(mqttRetain);
        Serial.print("MQTT Last Will Topic: ");
        Serial.println(mqttLwtTopic);
        Serial.print("MQTT Last Will Message: ");
        Serial.println(mqttLwtMessage);
        Serial.print("MQTT Last Will QoS: ");
        Serial.println(mqttLwtQos);
        Serial.print("MQTT Last Will Retain: ");
        Serial.println(mqttLwtRetain);
        Serial.print("MQTT Last Will Enabled: ");
        Serial.println(mqttLwtEnabled);
        Serial.print("Timezone: ");
        Serial.println(timezone);
        Serial.println("SSID: " + ssid);
        Serial.println("Password: " + pass);
        Serial.println("WiFi Mode: " + wifiMode);
        
        mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
    }
    if(wifiMode == "STA"){

        wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));
    }
  
    WiFi.onEvent(WiFiEvent);
    if(wifiMode == "STA" && mqttEnable){
        mqttClient.onConnect(onMqttConnect);
        mqttClient.onDisconnect(onMqttDisconnect);
        mqttClient.onSubscribe(onMqttSubscribe);
        mqttClient.onUnsubscribe(onMqttUnsubscribe);
        mqttClient.onMessage(onMqttMessage);
        mqttClient.onPublish(onMqttPublish);
        mqttClient.setServer(mqttHost.c_str(), mqttPort);
        mqttClient.setCredentials(mqttUser.c_str(), mqttPass.c_str());
        connectToWifi();
    }
  }
bool once2 = true;
  void WifiMqttConfig::loop(){
    if(once2){once2 = false;Serial.println("MQTT loop.");}
    // if (WiFi.status() != WL_CONNECTED && wifiMode == "STA" && mqttEnable) {
    //     static unsigned long lastAttemptTime = 0;
    //     if (millis() - lastAttemptTime > 2000) { // Delay 2000ms between connection attempts
    //         lastAttemptTime = millis();
    //         connectToWifi();
    //     }
    // }
    if (reconnectAttempts >= 10) {
        Serial.println("Failed to connect to Wi-Fi after 3 attempts. Switching to AP mode.");
        // Cập nhật cấu hình sang chế độ AP
        wifiMode = "AP";
        DynamicJsonDocument doc(1024);
        doc["mqttEnable"] = mqttEnable;
        doc["mqttHost"] = mqttHost;
        doc["mqttPort"] = mqttPort;
        doc["mqttUser"] = mqttUser;
        doc["mqttPass"] = mqttPass;
        doc["wifiMode"] = wifiMode; // Lưu chế độ WiFi (STA hoặc AP)
        doc["ssid"] = ssid;
        doc["pass"] = pass;
        doc["conId"] = conId;
        doc["mqttTopic"] = mqttTopic;
        doc["mqttTopicSub"] = mqttTopicSub;
    
        File configFile = LittleFS.open(WIFIMQTT_FILE, "w");
        if (!configFile) {
            Serial.println("Failed to open config file for writing.");
            return;
        }
        serializeJson(doc, configFile);
        configFile.close();
        delay(1000);
        ESP.restart();
    }
  }
  #endif//MQTT_Client