#define HEVICLE
#include "LEDRGB.h"
#include <ArduinoJson.h>
#include "./AudioFunc.h"
AudioCmd audio_cmd; // Create an instance of AudioCmd

void VehicleSetup() {
    Serial.println("========[ Vehicle Setup Starting... ]========");
    Led_setup();
    audio_cmd.audio_setup();
    // Initialize other vehicle components here
}

void VehicleLoop() {
    audio_cmd.audio_loop(); // Handle audio playback
    // Add other vehicle loop functionalities here

}