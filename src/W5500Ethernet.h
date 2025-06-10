#include <Arduino.h>
#define ESP32SC

#define DEBUG_ETHERNET_WEBSERVER_PORT       Serial
// Debug Level from 0 to 4
#define _ETHERNET_WEBSERVER_LOGLEVEL_       3
#ifdef ESP32SC
#include <WebServer_ESP32_SC_W5500.h>
#else
#include <WebServer_ESP32_W5500.h>
#endif// ESP32

bool EthernetConnected = false;
extern bool EthernetConnected;
// Enter a MAC address and IP address for your controller below.
#define NUMBER_OF_MAC      20

byte mac[][NUMBER_OF_MAC] =
{
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x01 },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xBE, 0x02 },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x03 },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xBE, 0x04 },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x05 },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xBE, 0x06 },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x07 },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xBE, 0x08 },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x09 },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xBE, 0x0A },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x0B },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xBE, 0x0C },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x0D },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xBE, 0x0E },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x0F },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xBE, 0x10 },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x11 },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xBE, 0x12 },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x13 },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xBE, 0x14 },
};

// Select the IP address according to your local network
IPAddress myIP(192, 168, 5, 232);
IPAddress myGW(192, 168, 5, 1);
IPAddress mySN(255, 255, 255, 0);
IPAddress dns1(8, 8, 8, 8); 
IPAddress dns2(8, 8, 4, 4); 
// Google DNS Server IP
IPAddress myDNS(8, 8, 8, 8);

bool getEtherConnectState(){
  #ifdef ESP32SC
  return ESP32_W5500_isConnected();
  #else
  return W5500ETH.isConnected();
  #endif//ESP32SC
}
void W5500setup(int CS_GP,int INT_GP, int SCK_GP, int MISO_GP, int MOSI_GP)
{
  if(MOSI_GP < 0 || MISO_GP < 0 || SCK_GP < 0 || CS_GP < 0 || INT_GP < 0){
    ET_LOGERROR(F(" ⚠️   Invalid GPIO pin configuration for W5500!"));
    return;
  }
  Serial.print(F("\nStart MQTTClient_Auth on "));
  Serial.print(ARDUINO_BOARD);
  Serial.print(F(" with "));
  // Serial.println(SHIELD_TYPE);
  // Serial.println(WEBSERVER_ESP32_W5500_VERSION);

  ET_LOGWARN(F("Default SPI pinout:"));
  ET_LOGWARN1(F("SPI_HOST:"), ETH_SPI_HOST);
  ET_LOGWARN1(F("MOSI:"), MOSI_GP);
  ET_LOGWARN1(F("MISO:"), MISO_GP);
  ET_LOGWARN1(F("SCK:"),  SCK_GP);
  ET_LOGWARN1(F("CS:"),   CS_GP);
  ET_LOGWARN1(F("INT:"),  INT_GP);
  ET_LOGWARN1(F("SPI Clock (MHz):"), SPI_CLOCK_MHZ);
  ET_LOGWARN(F("========================="));

  ///////////////////////////////////

  // To be called before ETH.begin()
  ESP32_W5500_onEvent();

  // start the ethernet connection and the server:
  // Use DHCP dynamic IP and random mac
  //bool begin(int MISO_GPIO, int MOSI_GPIO, int SCLK_GPIO, int CS_GPIO, int INT_GPIO, int SPI_CLOCK_MHZ,
  //           int SPI_HOST, uint8_t *W6100_Mac = W6100_Default_Mac);
  #ifdef ESP32SC
  if(SCETH.begin( MISO_GP, MOSI_GP, SCK_GP, CS_GP, INT_GP, SPI_CLOCK_MHZ, ETH_SPI_HOST, mac[millis() % NUMBER_OF_MAC] )) 
  #else
  if(W5500ETH.begin( MISO_GP, MOSI_GP, SCK_GP, CS_GP, INT_GP, SPI_CLOCK_MHZ, ETH_SPI_HOST, mac[millis() % NUMBER_OF_MAC] )) 
  #endif//ESP32SC
  {
    ET_LOG(F("✅   W5500 Ethernet started successfully"));
  } else {
    ET_LOGERROR(F("❌   Failed to start W5500 Ethernet!"));
    return;
  }
  //ETH.begin( MISO_GPIO, MOSI_GPIO, SCK_GPIO, CS_GPIO, INT_GPIO, SPI_CLOCK_MHZ, ETH_SPI_HOST, mac[millis() % NUMBER_OF_MAC] );
    // Load IP configuration from Ethernet.json
    if (!LittleFS.exists("/Ethernet.json")) {
        DynamicJsonDocument doc(256);
        doc["ip"] = myIP.toString();
        doc["gateway"] = myGW.toString();
        doc["subnet"] = mySN.toString();
        doc["dns"] = myDNS.toString();
        File configFile = LittleFS.open("/Ethernet.json", "w");
        if (configFile) {
            serializeJson(doc, configFile);
            configFile.close();
        }
    }

    File configFile = LittleFS.open("/Ethernet.json", "r");
    if (configFile) {
        size_t size = configFile.size();
        if (size > 0) {
            std::unique_ptr<char[]> buf(new char[size + 1]);
            configFile.readBytes(buf.get(), size);
            buf[size] = '\0';
            DynamicJsonDocument doc(512);
            DeserializationError error = deserializeJson(doc, buf.get());
            if (!error) {
                if (doc.containsKey("ip")) {
                    myIP.fromString(doc["ip"].as<const char*>());
                    Serial.println("myIP: " + myIP.toString());
                }
                if (doc.containsKey("gateway")) {
                    myGW.fromString(doc["gateway"].as<const char*>());
                    Serial.println("myGW: " + myGW.toString());
                }
                if (doc.containsKey("subnet")) {
                    mySN.fromString(doc["subnet"].as<const char*>());
                    Serial.println("mySN: " + mySN.toString());
                }
                if (doc.containsKey("dns")) {
                    myDNS.fromString(doc["dns"].as<const char*>());
                    Serial.println("myDNS: " + myDNS.toString());
                }
                if(doc.containsKey("dhcp")){
                    bool dhcpEnable = doc["dhcp"];
                    Serial.println("dhcp: " + String(dhcpEnable? "enable":"disable"));
                    if(dhcpEnable){
                      #ifdef ESP32SC
                        SCETH.config(myIP, myGW, mySN, myDNS);
                      #else
                        W5500ETH.config(myIP, myGW, mySN, myDNS);
                      #endif//ESP32SC
                    }else{
                            // Static IP, leave without this line to get IP via DHCP
                        bool config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1, IPAddress dns2 );
                    }
                }
            }
        }
        configFile.close();
    }
    


  // ESP32_W5500_waitForConnect();
  mainwebInterface.setupWebConfig();
  ///////////////////////////////////
}

void W5500loop()
{
    
}
