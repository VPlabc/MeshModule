#include <Arduino.h>
#include "esp_wifi.h"
#define DEBUG_OUTPUT_SERIAL

#ifdef DEBUG_OUTPUT_SERIAL
#define DEBUG_PIPE SERIAL_PIPE
#define LOG(string) {Serial.print(string);}
#define LOGLN(string) {Serial.println(string);}

#else
#define LOG(string) {}
#define LOGLN(string) {}
#endif 
  uint8_t current_protocol;
  wifi_interface_t current_wifi_interface;
  /////////////////////////////////////////////// WIFI RF Function /////////////////////////////////////////////////
  byte rssi_display = 0;
  typedef struct {
    unsigned frame_ctrl: 16;
    unsigned duration_id: 16;
    uint8_t addr1[6]; /* receiver address */
    uint8_t addr2[6]; /* sender address */
    uint8_t addr3[6]; /* filtering address */
    unsigned sequence_ctrl: 16;
    uint8_t addr4[6]; /* optional */
  } wifi_ieee80211_mac_hdr_t;
  
  typedef struct {
    wifi_ieee80211_mac_hdr_t hdr;
    uint8_t payload[0]; /* network data ended with 4 bytes csum (CRC32) */
  } wifi_ieee80211_packet_t;
  //La callback que hace la magia
  void promiscuous_rx_cb(void *buf, wifi_promiscuous_pkt_type_t type) {
    // All espnow traffic uses action frames which are a subtype of the mgmnt frames so filter out everything else.
    if (type != WIFI_PKT_MGMT)
      return;
    const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buf;
    const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
    const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;
  
    int rssi = ppkt->rx_ctrl.rssi;
    rssi_display = rssi;
  }
  void init_wifi_promiscuous_mode() {
    // Set the WiFi to promiscuous mode
    // esp_wifi_set_promiscuous(true);
    // esp_wifi_set_promiscuous_rx_cb(&promiscuous_rx_cb);
    // esp_wifi_set_channel(MeshConfig.wifiChannel, WIFI_SECOND_CHAN_NONE); // Set the WiFi channel
    // wifi_interface_t current_wifi_interface = WIFI_IF_STA; // Sử dụng giao diện Wi-Fi Station
    esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR); // Set the WiFi protocol  WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N | WIFI_PROTOCOL_LR
    // esp_wifi_set_ps(WIFI_PS_NONE); // Disable power save mode
    // esp_wifi_set_max_tx_power(78); // 78 tương ứng với 20 dBm
    
  }
  void init_wifi(byte wifiChannel) {
    esp_err_t addStatus = esp_wifi_start();
    if (addStatus == ESP_OK) {LOGLN("esp_wifi_start success"); }
    else{LOGLN("esp_wifi_start failed");}
    // Set the WiFi to station mode
    // esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(&promiscuous_rx_cb);
    esp_wifi_set_channel(wifiChannel, WIFI_SECOND_CHAN_NONE); // Set the WiFi channel
    esp_wifi_set_protocol(current_wifi_interface, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N ); // Set the WiFi protocol
    esp_wifi_set_ps(WIFI_PS_NONE); // Disable power save mode
  }
  
  int check_protocol()
  {
    // CONFIG::read_byte (EP_EEPROM_DEBUG, &MeshDebug);
      char error_buf1[100];
    // if(MeshDebug){
      LOGLN();
       esp_err_t error_code = esp_wifi_get_protocol(current_wifi_interface, &current_protocol);
       esp_err_to_name_r(error_code,error_buf1,100);
       LOG("esp_wifi_get_protocol error code: ");
       LOGLN(error_buf1);
      LOGLN("Code: " + String(current_protocol));
      if ((current_protocol&WIFI_PROTOCOL_11B) == WIFI_PROTOCOL_11B)
        LOGLN("Protocol is WIFI_PROTOCOL_11B");
      if ((current_protocol&WIFI_PROTOCOL_11G) == WIFI_PROTOCOL_11G)
        LOGLN("Protocol is WIFI_PROTOCOL_11G");
      if ((current_protocol&WIFI_PROTOCOL_11N) == WIFI_PROTOCOL_11N)
        LOGLN("Protocol is WIFI_PROTOCOL_11N");
      if ((current_protocol&WIFI_PROTOCOL_LR) == WIFI_PROTOCOL_LR)
        LOGLN("Protocol is WIFI_PROTOCOL_LR");
      LOGLN("___________________________________");
      LOGLN();
      LOGLN();
    // }
      return current_protocol;
  }
