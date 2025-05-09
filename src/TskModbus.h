#include <Arduino.h>
#include <FS.h>
#define MODBUS_FILE "/modbus.json"
//{"Modbus":"Config","role":"slave","Com":"TCP/IP","id":255,"slaveip":[192,168,3,251],"conId":"b8e54d33-b34a-45ab-b76f-62c8a9abc6c4","Tag":[1101,1102,1103,1153,1168,1169],"Value":[164,166,32,130,142,170],"Type":[2,2,0,1,2,2]}
/*  
  role : master/slave
  Com : TCP/IP/RS485
  id : Modbus Slave ID
  slaveip : Modbus Slave IP
  Tag: MQTT value tag
  Value : Register Address
  Type : Data Type coil =0, WORD=1, DWORD=2, FLOAT=3
*/
class ModbusConfig {
    
    public:
        void saveJsonToModbusFile(char &jsonString,bool debug, fs::FS &FileSystem);
        String loadModbusConfig(bool debug, fs::FS &FileSystem);
    
};

void ModbusConfig::saveJsonToModbusFile(char &jsonString,bool debug, fs::FS &FileSystem) {
    File file = FileSystem.open(MODBUS_FILE, "w");
    if (!file) {
        if (debug) Serial.println("Failed to open modbus.json for writing.");
        return;
    }

    size_t bytesWritten = file.print(jsonString);
    file.close();

    if (bytesWritten == 0) {
        if (debug) Serial.println("Failed to write JSON to modbus.json.");
    } else {
        if (debug) Serial.println("JSON saved to modbus.json successfully.");
    }
}

// Hàm tải file modbus.json và truyền chuỗi JSON vào ModbusInit
String ModbusConfig::loadModbusConfig(bool debug, fs::FS &FileSystem) {
    String jsonString;
    if (!FileSystem.exists(MODBUS_FILE)) {
        if (debug) Serial.println("modbus.json not found.");
        
        // Nếu file không tồn tại, tạo file với cấu hình mặc định
        DynamicJsonDocument doc(1024);
        doc["Modbus"] = "Config";
        doc["role"] = "slave";
        doc["Com"] = "TCP/IP";
        doc["id"] = 255;
        JsonArray slaveip = doc.createNestedArray("slaveip");
        slaveip.add(192);
        slaveip.add(168);
        slaveip.add(3);
        slaveip.add(251);
        doc["conId"] = "b8e54d33-b34a-45ab-b76f-62c8a9abc6c4";
        JsonArray tags = doc.createNestedArray("Tag");
        tags.add(1101);
        tags.add(1102);
        tags.add(1103);
        JsonArray values = doc.createNestedArray("Value");
        values.add(164);
        values.add(166);
        values.add(32);
        JsonArray types = doc.createNestedArray("Type");
        types.add(2);
        types.add(2);
        types.add(0);
        // Lưu cấu hình mặc định vào file
        File configFile = FileSystem.open(MODBUS_FILE, "w");
        if (configFile) {
            serializeJson(doc, configFile);
            configFile.close();
            Serial.println("Default configuration created.");
        } else {
            return "Failed to create default config";
        }
        // Trả về cấu hình mặc định
        serializeJson(doc, jsonString);
        return jsonString;
    }
    else{
        File file = FileSystem.open(MODBUS_FILE, "r");
        if (!file) {
            if (debug) Serial.println("Failed to open modbus.json for reading.");
            return "Failed to open modbus.json for reading.";
        }
        else{
                while (file.available()) {
                    jsonString += char(file.read());
                }
                file.close();

                if (debug) {
                    Serial.println("Loaded JSON from modbus.json:");
                    Serial.println(jsonString);
                }

        }
    }
    return jsonString;
}