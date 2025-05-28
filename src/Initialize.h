#include <Arduino.h>

void initializeSPI() {
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN); // SCK, MISO, MOSI
}


void initializeSDCard() {
#ifdef USE_SD
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("\n SD card failed to initialize! ❌");
    Serial.println((" CS Pin: " + String(SD_CS_PIN)));
  } else {
    Serial.println("\n SD Card initialized.  ✅");
  }
#endif//USE_SD
}

void initializeEthernet() {
#ifdef USE_LAN8720
//ETH_POWER_PIN_ALTERNATIVE(CS_PIN) | ETH_MDC_PIN(RST_PIN) | ETH_MDIO_PIN(INT_PIN) 
    if(RST_PIN == -1 && INT_PIN == -1 && CS_PIN == -1){
        Serial.println(" Pin ETH_MDC_PIN " + String(RST_PIN) + " \n Pin ETH_MDIO_PIN " + String(INT_PIN) + " \n Pin ETH_POWER_PIN_ALTERNATIVE " + String(CS_PIN));
        pinMode(ETH_POWER_PIN_ALTERNATIVE, OUTPUT);
        digitalWrite(CS_PIN, HIGH);
        if(ETH.begin(ETH_ADDR, ETH_POWER_PIN, RST_PIN, INT_PIN, ETH_TYPE, ETH_CLK_MODE)){
            // ETH.config(str2IP(SettingData[6]), str2IP(SettingData[7]), str2IP(SettingData[8]), str2IP(SettingData[9]), str2IP(SettingData[10]));
            EthernetAvilable = true;
        }else{
            LOGLN("❌ Failed to init Ethernet");
            EthernetAvilable = false;
        }
    }
    else
    {
        LOGLN("❌ Ethernet not available");
        EthernetAvilable = false;
    }
#endif//USE_LAN8720
#ifdef USE_W5500
  Serial.println("Initializing Ethernet...");
    W5500setup(CS_PIN, INIT_PIN, SCK_PIN, MISO_PIN, MOSI_PIN);
#endif//USE_W5500
}

void initializeModbus() {
    #ifdef USE_Modbus
        ModbusInit(mainModbusConfig.loadModbusConfig(MeshConfig.debug, FileSystem), loadModbusDataBlockConfig(MeshConfig.debug, FileSystem));
    #endif//USE_Modbus
}

void initializeWifi() {
if(mqttEnable && wifiMode == "STA"){
        esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N );
        while (WiFi.status() != WL_CONNECTED){
            static long timeCount = 0;
            static long timeCountConnect = 0;
            if (millis() - timeCount >= 1000) {
                timeCount = millis();
                digitalWrite(LED_STT, !digitalRead(LED_STT)); // Toggle LED state
                if (MeshConfig.debug) Serial.println("🕒 Waitt connecting to WiFi...");
                timeCountConnect++;
                if(timeCountConnect >= 10){
                    if (MeshConfig.debug) Serial.println("❌ Connecting to WiFi failed.");
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
            if (MeshConfig.debug) Serial.println("🔁 WiFi not connected. Retrying...");
            WiFi.begin(); // Retry WiFi connection
            delay(5000);  // Wait for connection
            if (WiFi.status() != WL_CONNECTED) {
                if (MeshConfig.debug) Serial.println("❌ Failed to connect to WiFi. Check your configuration.");
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
}

void initializeFileSytem(){
    if (!FileSystem.begin()) {
        if (MeshConfig.debug) Serial.println("❌ Failed to initialize LittleFS. Attempting to format...");
        if (FileSystem.format()) {
            if (MeshConfig.debug) Serial.println("✅ File system formatted successfully. Retrying initialization...");
            if (!FileSystem.begin()) {
                if (MeshConfig.debug) Serial.println("❌ Failed to initialize LittleFS after formatting.");
                return;
            }
        } else {
            if (MeshConfig.debug) Serial.println("❌ Failed to format LittleFS.");
            return;
        }
    }
}