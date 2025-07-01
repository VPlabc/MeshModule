#define AudioFunct
#include <ArduinoJson.h>

#ifdef AudioFunct
#include "./AudioFunc.h"
AudioCmd audio_cmd; // Create an instance of AudioCmd
#endif//AudioFunct

void VehicleSetup() {
    Serial.println("========[ Vehicle Setup Starting... ]========");

    #ifdef AudioFunct
    audio_cmd.audio_setup();
    #endif//AudioFunct
    // Initialize other vehicle components here
}

void VehicleLoop() {

    #ifdef AudioFunct
    audio_cmd.audio_loop(); // Handle audio playback
    #endif//AudioFunct
    
}