#include <Arduino.h>
#include <vector>
#include <map>
#include <chrono>
struct nodeData {
    int id;
    int netId;
    uint16_t DataReg[200];
    bool DataCoil[200];
    float dataSuccessRate;
    std::chrono::steady_clock::time_point lastReceivedTime;
};

std::map<int, nodeData> nodeDataMap;


struct DataMapping {
    std::vector<int> regAddresses; // Ensure std::vector is included
    std::vector<int> coilAddresses;
};

DataMapping dataMapping;


// String DATA_MAPPING_FILE = "/DataMapping.json";

void updateDataMappingFromPacket(const dataPacket &packet) {
    if (dataMapping.regAddresses.empty() && dataMapping.coilAddresses.empty()) {
        if (MeshConfig.debug) Serial.println("DataMapping is empty. Skipping update.");
        return;
    }

    size_t dataIndex = 0; // Index to track position in packet.data
    for (size_t i = 0; i < dataMapping.regAddresses.size(); ++i) {
        if (dataIndex + 1 < sizeof(packet.data)) {
            uint16_t value = (packet.data[dataIndex] << 8) | packet.data[dataIndex + 1];
            dataIndex += 2;
            if (MeshConfig.debug) Serial.println("WORD at regAddress[" + String(dataMapping.regAddresses[i]) + "] = " + String(value));
        }
    }

    for (size_t i = 0; i < dataMapping.coilAddresses.size(); ++i) {
        if (dataIndex < sizeof(packet.data)) {
            bool coilValue = packet.data[dataIndex] > 0;
            dataIndex++;
            if (MeshConfig.debug) Serial.println("COIL at coilAddress[" + String(dataMapping.coilAddresses[i]) + "] = " + String(coilValue));
        }
    }
}

void createDefaultNodeDataFile() {

    if (!FileSystem.exists(DATA_MAPPING_FILE)) {
        if (MeshConfig.debug) Serial.println("DataMapping.json not found. Creating default file.");
        File file = FileSystem.open(DATA_MAPPING_FILE, "w");
        if (file) {
            DynamicJsonDocument doc(1024);
            JsonArray nodeDataArray = doc.createNestedArray("nodeData");

            // Add default entries
            JsonObject entry1 = nodeDataArray.createNestedObject();
            entry1["nodeId"] = 1;
            JsonObject data1 = entry1.createNestedObject("data");
            data1["k"] = "key1";
            data1["a"] = 0;
            data1["t"] = 1; // WORD

            JsonObject entry2 = nodeDataArray.createNestedObject();
            entry2["nodeId"] = 2;
            JsonObject data2 = entry2.createNestedObject("data");
            data2["k"] = "key1";
            data2["a"] = 0;
            data2["t"] = 1; // WORD

            if (serializeJson(doc, file) == 0) {
                if (MeshConfig.debug) Serial.println("Failed to write to DataMapping.json.");
            } else {
                if (MeshConfig.debug) Serial.println("Default DataMapping.json created.");
            }
            file.close();
        } else {
            if (MeshConfig.debug) Serial.println("Failed to open DataMapping.json for writing.");
        }
    } else {
        if (MeshConfig.debug) Serial.println("DataMapping.json already exists.");
    }
}

void saveDataMapping() {
    File file = FileSystem.open(DATA_MAPPING_FILE, "w");
    if (!file) {
        if (MeshConfig.debug) Serial.println("Failed to open DataMapping.json for writing.");
        return;
    }

    DynamicJsonDocument doc(1024);
    JsonArray nodeDataArray = doc.createNestedArray("nodeData");

    for (size_t i = 0; i < dataMapping.regAddresses.size(); ++i) {
        JsonObject entry = nodeDataArray.createNestedObject();
        entry["nodeId"] = i + 1;
        JsonObject data = entry.createNestedObject("data");
        data["k"] = "key" + String(i + 1);
        data["a"] = dataMapping.regAddresses[i];
        data["t"] = 1; // WORD
    }

    for (size_t i = 0; i < dataMapping.coilAddresses.size(); ++i) {
        JsonObject entry = nodeDataArray.createNestedObject();
        entry["nodeId"] = dataMapping.regAddresses.size() + i + 1;
        JsonObject data = entry.createNestedObject("data");
        data["k"] = "key" + String(dataMapping.regAddresses.size() + i + 1);
        data["a"] = dataMapping.coilAddresses[i];
        data["t"] = 0; // COILS
    }

    if (serializeJson(doc, file) == 0) {
        if (MeshConfig.debug) Serial.println("Failed to write to DataMapping.json.");
    } else {
        if (MeshConfig.debug) Serial.println("DataMapping saved.");
    }
    file.close();
}

void loadDataMapping() {
    if (!FileSystem.exists(DATA_MAPPING_FILE)) {
        if (MeshConfig.debug) Serial.println("DataMapping.json not found. Creating default mapping.");
        createDefaultNodeDataFile();
    }
    
    File file = FileSystem.open(DATA_MAPPING_FILE, "r");
    if (!file) {
        if (MeshConfig.debug) Serial.println("Failed to open DataMapping.json for reading.");
        return;
    }
    Serial.println("content of MappingFile:\n" + file.readString());
    file.seek(0); // Reset file pointer to the beginning

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, file);
    if (error) {
        if (MeshConfig.debug) Serial.println("Failed to parse DataMapping.json.");
        file.close();
        return;
    }

    dataMapping.regAddresses.clear();
    dataMapping.coilAddresses.clear();

    for (JsonObject entry : doc["nodeData"].as<JsonArray>()) {
        JsonObject data = entry["data"];
        int address = data["a"];
        int type = data["t"];
        if (type == 1) { // WORD
            dataMapping.regAddresses.push_back(address);
        } else if (type == 0) { // COILS
            dataMapping.coilAddresses.push_back(address);
        }
    }

    file.close();
    if (MeshConfig.debug) Serial.println("DataMapping loaded.");
}

void updateNodeDataWithMapping(const dataPacket &packet) {
    int nodeId = packet.ID;
    auto currentTime = std::chrono::steady_clock::now();

    if (nodeDataMap.find(nodeId) == nodeDataMap.end()) {
        nodeDataMap[nodeId] = {nodeId, packet.netId, {}, {}, 100.0f, currentTime};
    }

    nodeData &node = nodeDataMap[nodeId];
    node.netId = packet.netId;
    node.lastReceivedTime = currentTime;

    size_t dataIndex = 0;
    for (int reg : dataMapping.regAddresses) {
        if (dataIndex + 1 < sizeof(packet.data)) {
            node.DataReg[reg] = (packet.data[dataIndex] << 8) | packet.data[dataIndex + 1];
            dataIndex += 2;
        }
    }

    for (int coil : dataMapping.coilAddresses) {
        if (dataIndex < sizeof(packet.data)) {
            node.DataCoil[coil] = packet.data[dataIndex] > 0;
            dataIndex++;
        }
    }
}

void printNodeDataWithMapping() {
    Serial.println("===== Node Data with Mapping =====");
    for (const auto &entry : nodeDataMap) {
        const nodeData &node = entry.second;
        if (node.id == 0) {
            Serial.println("Node 0 does not exist.");
            continue;
        }

        Serial.print("Node ID: ");
        Serial.print(node.id);
        Serial.print(" | Net ID: ");
        Serial.print(node.netId);
        Serial.print(" | Success Rate: ");
        Serial.print(node.dataSuccessRate);
        Serial.print("%");

        // Calculate time ago
        auto currentTime = std::chrono::steady_clock::now();
        auto timeAgo = std::chrono::duration_cast<std::chrono::seconds>(currentTime - node.lastReceivedTime).count();
        Serial.print(" | Time Ago: ");
        Serial.print(timeAgo);
        Serial.println(" seconds");

        Serial.print("Mapped Data Values: ");
        for (size_t i = 0; i < dataMapping.regAddresses.size(); ++i) {
            int address = dataMapping.regAddresses[i];
            int type = 1; // Default to WORD

            if (i < dataMapping.coilAddresses.size()) {
                type = 0; // COIL
            }

            if (type == 0) { // COIL
                Serial.print(node.DataCoil[address] ? "1" : "0");
                if (i < dataMapping.coilAddresses.size() - 1) {
                    Serial.print(", ");
                }
            } else if (type == 1) { // WORD
                uint16_t value = node.DataReg[address];
                Serial.print(value);
                if (i < dataMapping.regAddresses.size() - 1) {
                    Serial.print(", ");
                }
            } else if (type == 2) { // DWORD
                if (address + 1 < sizeof(node.DataReg) / sizeof(node.DataReg[0])) {
                    uint32_t value = (node.DataReg[address] << 16) | node.DataReg[address + 1];
                    Serial.print(value);
                    if (i < dataMapping.regAddresses.size() - 1) {
                        Serial.print(", ");
                    }
                }
            } else if (type == 3) { // FLOAT
                if (address + 1 < sizeof(node.DataReg) / sizeof(node.DataReg[0])) {
                    uint32_t combinedValue = (static_cast<uint32_t>(node.DataReg[address]) << 16) | node.DataReg[address + 1];
                    float value = *reinterpret_cast<float *>(&combinedValue);
                    Serial.print(value);
                    if (i < dataMapping.regAddresses.size() - 1) {
                        Serial.print(", ");
                    }
                }
            }
        }
        Serial.println();
    }
    Serial.println("==================================");
}
