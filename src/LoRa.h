#ifndef LORA_H
#define LORA_H
#define Module_RS485
// #define VERSION_1
#define VERSION_2

#define M0 2
#define M1 15
#define NORMAL_MODE 0
#define CONFIG_MODE 3
extern bool LoRaInit;
class LoRaFunction {

public:
#ifdef VERSION_1
    LoRaFunction(){};
    void LoRaSetup(int LoRaID);
    void LoRaLoop(int SlaveID);
    void sendCommand(const String &cmd);
    void sendData(String address, String data);
    String hexToAscii(const String &hex);
    void receiveData();
    void setMode(uint8_t mode);
    void configureE32(String json);
    void parseUartSpeed(String str, uint8_t &baud, uint8_t &parity);
    uint8_t parseAirDataRate(String rate);
    uint8_t parsePower(int power);
    bool verifyResponse(uint8_t *cmd);
    void updateBaudRate(String uartSpeed);
#endif//VERSION_1
#ifdef VERSION_2
    void LoRaSetup(int LoRaID);
    void LoRaLoop(int SlaveID);
    void receiveData();
    void configureLoRaModule(int address, int networkID, int power, long frequency, int spreadingFactor, int bandwidth, int codingRate, int preamble);
    String getLoRaModuleConfig();
    
#endif//VERSION_2
};

#endif// LORA_H