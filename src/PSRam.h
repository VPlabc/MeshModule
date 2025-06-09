#include <Arduino.h>
// #include <psram.h>
#include <string.h>
#include <ArduinoJson.h>

void writeStringToPSRAM(const String &data) {
    if (psramFound() && psramInit()) {
        Serial.println("PSRAM initialized successfully.");
        acc_data_all = (unsigned char *) ps_malloc(data.length() + 1); // +1 for null terminator
        if (acc_data_all) {
            strcpy((char *)acc_data_all, data.c_str());
            Serial.println("Data written to PSRAM: " + String((char *)acc_data_all));
        } else {
            Serial.println("Failed to allocate memory in PSRAM.");
        }
    } else {
        Serial.println("PSRAM not available or initialization failed.");
    }
}
String ReadStringFromPSRAM() {
    if (acc_data_all) {
        String data = String((char *)acc_data_all);
        Serial.println("Data read from PSRAM: " + data);
        return data;
    } else {
        Serial.println("No data in PSRAM.");
        return "";
    }
}

