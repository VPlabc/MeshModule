#ifndef LORA_H
#define LORA_H

#define M0 -1//5//2
#define M1 -1//27//15
#define NORMAL_MODE 0
#define CONFIG_MODE 3

class LoRaFunction {

public:
    void LoRaSetup(int LoRaID);
    void LoRaLoop(int SlaveID);
    void receiveData();
    // void configureE32(String json);
    void SetPinLoRa(uint8_t M0Pin, uint8_t M1Pin, uint8_t TX = 17, uint8_t RX = 16);
    void SetChanel(byte CH);
    void WriteLoRaConfig(byte CH);
    void WriteLoRaConfig(byte CH, byte AirRate);
    void ReadLoRaConfig();
    void printParameters(struct Configuration configuration);


};

#endif// LORA_H