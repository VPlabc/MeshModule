#ifndef LORA_H
#define LORA_H

#define M0 2
#define M1 15
#define NORMAL_MODE 0
#define CONFIG_MODE 3

class LoRaFunction {

public:
    void LoRaSetup();
    void LoRaLoop();
    void configureE32(String json);
};

#endif// LORA_H