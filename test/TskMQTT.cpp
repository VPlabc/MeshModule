#include "TskMQTT.h"

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
    net.setInsecure();

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
void WifiMqttConfig::setupWiFi() {
    if (wifiMode == "STA") {
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid.c_str(), pass.c_str());
    }
    else{
        mqttEnable = false;
        Serial.println("MQTT disabled.");
    }
}
void WifiMqttConfig::setup() {

    if (!LittleFS.begin()) {
        Serial.println("Failed to initialize LittleFS");
        return;
    }

    // Tải cấu hình từ file
    if (LittleFS.exists("/config.json")) {
        File configFile = LittleFS.open("/config.json", "r");
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
            ssid = doc["ssid"] | "yourSSID";
            pass = doc["pass"] | "yourPassword";
            conId = doc["conId"] | "b8e54d33-b34a-45ab-b76f-62c8a9abc6c4";
            mqttTopic = doc["mqttTopic"] | "test/topic";
            mqttTopicSub = doc["mqttTopicSub"] | "test/topic/sub";
        }
    }
    if(mqttEnable){
        Serial.println("MQTT enabled.");
        Serial.print("MQTT Host: ");
        Serial.println(mqttHost);
        Serial.print("MQTT Port: ");
        Serial.println(mqttPort);
        Serial.print("MQTT User: ");
        Serial.println(mqttUser);
        Serial.print("MQTT Password: ");
        Serial.println(mqttPass);}
        setupWiFi();
        if (mqttEnable) {
            setupMQTT();
        }

    server.begin();
    Serial.println("Web server started.");
}

void WifiMqttConfig::loop() {
    if(WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected, trying to reconnect...");
        setupWiFi();
    }else{
        if (mqttEnable) {
            mqttClient.loop();
            if (!mqttClient.connected()) {
                setupMQTT();
            }
        }
    }
    // server.handleClient();
}

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
        doc["ssid"] = "yourSSID";
        doc["pass"] = "yourPassword";
        doc["conId"] = "b8e54d33-b34a-45ab-b76f-62c8a9abc6c4";
        doc["mqttTopic"] = "test/topic";
        doc["mqttTopicSub"] = "test/topic/sub";

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