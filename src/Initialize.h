#include <Arduino.h>
bool SPI_Initialized = false;
void initializeSPI() {
  if(SD_SCK_PIN == -1 && SD_MOSI_PIN == -1 && SD_MISO_PIN == -1){
        SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN); // SCK, MISO, MOSI
        if(MeshConfig.debug) Serial.println("✅   SPI initialized with SCK_PIN, MISO_PIN, MOSI_PIN");
        if(MeshConfig.debug) Serial.println("⚠️   SD_SCK_PIN, SD_MOSI_PIN, SD_MISO_PIN not set");
        if(MeshConfig.debug) Serial.println(" SCK_PIN: " + String(SCK_PIN) + " | MISO_PIN: " + String(MISO_PIN) + " | MOSI_PIN: " + String(MOSI_PIN));
        SPI_Initialized = true;
  }else{
        SPI.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN); // SCK, MISO, MOSI
        if(MeshConfig.debug) Serial.println(" SD_SCK_PIN: " + String(SD_SCK_PIN) + " | SD_MISO_PIN: " + String(SD_MISO_PIN) + " | SD_MOSI_PIN: " + String(SD_MOSI_PIN));
        if(MeshConfig.debug) Serial.println("✅   SPI initialized with SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN");
  }
}


bool initializeSDCard() {
#ifdef USE_SD
  if (SD_CS_PIN == -1 && SPI_Initialized == false){
    Serial.println("⚠️   SD_CS_PIN not set");
    Serial.println((" CS Pin: " + String(SD_CS_PIN)));
    return false; // SD_CS_PIN not set, cannot initialize SD card
  } else if (SD_CS_PIN > 0 ){
    Serial.println((" CS Pin: " + String(SD_CS_PIN)));
    if (!SD.begin(SD_CS_PIN, SPI)) { // 4MHz SPI frequency
        Serial.println("\n❌ SD card failed to initialize! ❌");
        return false; // SD card initialization failed
    } else {
        Serial.println("\n✅   SD Card initialized.  ✅");
        return true; // SD card initialized successfully
    }
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


void initHardware() {

    // #ifdef Module_10O4I
    if(BUZZ == -1){
        Serial.println("⚠️   BUZZ pin not set");
    }else if(BUZZ > 0){
        pinMode(BUZZ, OUTPUT);
        digitalWrite(BUZZ, LOW);
        digitalWrite(BUZZ, HIGH);delay(50);digitalWrite(BUZZ, LOW);
    }
    if(Y8 == -1){
        Serial.println("⚠️   Y8 pin not set");
    }else if(Y8 > 0){
        pinMode(Y8, OUTPUT);
        digitalWrite(Y8, LOW);
    }
    if(Y9 == -1){
        Serial.println("⚠️   Y9 pin not set");
    }else if(Y9 > 0){   
        pinMode(Y9, OUTPUT);
        digitalWrite(Y9, LOW);
    }

    // if(LED_STT == -1){
    //     Serial.println("⚠️   LED_STT pin not set");
    //     LED_STT = 0; // Set to 0 to disable LED functionality
    // }else if(LED_STT > 0){
    //     pinMode(LED_STT, OUTPUT);
    //     digitalWrite(LED_STT, HIGH);delay(100);
    //     digitalWrite(LED_STT, LOW);delay(100);
    //     digitalWrite(LED_STT, HIGH);delay(100);
    //     digitalWrite(LED_STT, LOW);delay(100);
    // }

    if(I2C_SDA == -1 || I2C_SCL == -1){
        Serial.println("⚠️   I2C pins not set");
    }else if(I2C_SDA > 0 && I2C_SCL > 0){
        Wire.begin(I2C_SDA, I2C_SCL); // Initialize I2C with specified pins
    }
    
}