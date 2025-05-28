#include <ArduinoJson.h>
#include <map>
#include <vector>
#include <ctime>
#include <iostream>
#include <map>
#include <chrono>

// Define a structure to hold node data
struct NodeDatas {
    int ID;
    int netId;
    std::string macAddress;
    std::vector<uint8_t> data;
    uint8_t dataSize;
    float dataSuccessRate;
    std::time_t timestamp; // Store the time when the data was received
    std::chrono::steady_clock::time_point lastReceivedTimes;
};

// Map to store data for each node, keyed by node ID
extern std::map<int, NodeDatas> nodeDataMaps;
NodeDatas nodeDatas;

// Function to update or add node data
void updateNodeData(const dataPacket &packet, const std::string &macAddress) {
    nodeDatas.ID = packet.ID;
    nodeDatas.netId = packet.netId;
    nodeDatas.data.assign(packet.data, packet.data + packet.dataSize);
    nodeDatas.dataSize = packet.dataSize;
    nodeDatas.timestamp = std::time(nullptr); // Get the current time

    auto currentTime = std::chrono::steady_clock::now();
    nodeDatas.lastReceivedTimes = currentTime;

    // Store the MAC address (assuming you add a field for it in NodeDatas)
    nodeDatas.macAddress = macAddress;

    // Update the map with the new data
    nodeDataMaps[packet.ID] = nodeDatas;
}

void calculateSuccessRates() {
    auto currentTime = std::chrono::steady_clock::now();

    for (auto &entry : nodeDataMaps) {
        NodeDatas &node = entry.second;
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(currentTime - node.lastReceivedTimes).count();

        if (duration <= 7) {
            node.dataSuccessRate = std::min(100.0f, node.dataSuccessRate + 1.0f); // Increment success rate
        } else {
            node.dataSuccessRate = std::max(0.0f, node.dataSuccessRate - 1.0f); // Decrement success rate
        }
    }
}

String LoadDataMapping() {
    // Load data mapping from JSON file
    String jsonString;
    if (!LittleFS.exists(DATA_MAPPING_FILE)) {
        // Tạo file mặc định nếu chưa tồn tại
        File defaultFile = LittleFS.open(DATA_MAPPING_FILE, "w");
        if (defaultFile) {
            defaultFile.print("[{\"ID\":1,\"types\":[\"COILS\",\"BYTE\",\"WORD\",\"DWORD\",\"FLOAT\"],\"keys\":[\"key1\",\"key2\",\"key3\",\"key4\",\"key5\"]}]");
            defaultFile.close();
            Serial.println("Created default DataMapping.json");
        } else {
            Serial.println("Failed to create default DataMapping.json");
        }
    }
    File file = LittleFS.open(DATA_MAPPING_FILE, "r");
    if (file) {
        jsonString = file.readString();
        file.close();
    } else {
        Serial.println("Failed to open DataMapping.json");
    }
    return jsonString;
}

String createJsonForWebSocket() {
    DynamicJsonDocument doc(4096); // Adjust size as needed
    JsonArray nodesArray = doc.to<JsonArray>();

    for (const auto &entry : nodeDataMaps) {
        const NodeDatas &node = entry.second;
        JsonObject nodeObject = nodesArray.createNestedObject();

        nodeObject["nodeId"] = node.ID;
        nodeObject["mac"] = node.macAddress; // Replace with actual MAC if available
        nodeObject["netId"] = node.netId;
        nodeObject["successRate"] = String(node.dataSuccessRate, 1) + "%";

        std::time_t currentTime = std::time(nullptr);
        double timeDiff = std::difftime(currentTime, node.timestamp);
        nodeObject["timeAgo"] = String(static_cast<int>(timeDiff)) + " seconds ago";

        JsonArray dataArray = nodeObject.createNestedArray("data");
        for (uint8_t byte : node.data) {
            dataArray.add(byte);
        }
        // Parse the JSON string from LoadDataMapping
        String jsonMapping = LoadDataMapping();
        // Serial.println(jsonMapping);
        // Parse the JSON string from LoadDataMapping
        DynamicJsonDocument mappingDoc(4096); // Adjust size as needed
        DeserializationError error = deserializeJson(mappingDoc, jsonMapping);

        JsonArray mappingArray = mappingDoc.as<JsonArray>();
        if (!error) {
            for (JsonObject mappingObject : mappingArray) {
                if (mappingObject["ID"] == node.ID) {

                    JsonArray keysArray = mappingObject["keys"].as<JsonArray>();
                    JsonArray keysNestedArray = nodeObject.createNestedArray("keys");
                    for (JsonVariant key : keysArray) {
                    keysNestedArray.add(key);
                    }
                    
                    JsonArray typesArray = mappingObject["types"].as<JsonArray>();
                    JsonArray typesNestedArray = nodeObject.createNestedArray("type");
                    for (JsonVariant type : typesArray) {
                    typesNestedArray.add(type);
                    }
                    break;
                }
            }
        } else {
            Serial.println("Failed to parse JSON mapping");
            nodeObject.createNestedArray("types"); // Empty keys array
            nodeObject.createNestedArray("keys"); // Empty keys array
        }
    }

    String jsonString;
    serializeJson(nodesArray, jsonString);
    return jsonString;
    // Clear the document to free memory
    doc.clear();
}
String createJsonForMqttt() {
    DynamicJsonDocument doc(4096); // Adjust size as needed
    JsonArray nodesArray = doc.to<JsonArray>();
    // Parse the JSON string from LoadDataMapping
    String jsonMapping = LoadDataMapping();
    // Parse the JSON string from LoadDataMapping
    DynamicJsonDocument mappingDoc(4096); // Adjust size as needed
    DeserializationError error = deserializeJson(mappingDoc, jsonMapping);
    JsonArray mappingArray = mappingDoc.as<JsonArray>();
    
    if (!error) {
        for (const auto &entry : nodeDataMaps) {
            const NodeDatas &node = entry.second;
            JsonObject nodeObject = nodesArray.createNestedObject();
            nodeObject["nodeId"] = node.ID;

            JsonArray dataArray = nodeObject.createNestedArray("data");
            for (uint8_t byte : node.data) {

                dataArray.add(byte);
            }
            for (JsonObject mappingObject : mappingArray) {
                if (mappingObject["ID"] == node.ID) {

                    JsonArray keysArray = mappingObject["keys"].as<JsonArray>();
                    JsonArray keysNestedArray = nodeObject.createNestedArray("keys");
                    JsonArray typesArray = mappingObject["types"].as<JsonArray>();
                    JsonArray typesNestedArray = nodeObject.createNestedArray("type");
                    for (JsonVariant key : keysArray) {
                    keysNestedArray.add(key);
                    }
                    for (JsonVariant type : typesArray) {
                    typesNestedArray.add(type);
                    }
                    break;
                }
            }
        }
    } else {
        Serial.println("Failed to parse JSON mapping");
    }

    String jsonString;
    serializeJson(nodesArray, jsonString);
    return jsonString;
    // Clear the document to free memory
    doc.clear();
}
// Function to print data for all nodes
void printNodeData() {
    for (const auto &entry : nodeDataMaps) {
        const NodeDatas &node = entry.second;
        Serial.print("Node ID: ");
        Serial.print(node.ID);
        Serial.print(" | Net ID: ");
        Serial.print(node.netId);
        Serial.print(" | Data Size: ");
        Serial.print(static_cast<int>(node.dataSize));
        Serial.print(" | Data: ");
        for (uint8_t byte : node.data) {
            Serial.print(byte);
            Serial.print(" ");
        }
        Serial.println();
        // Calculate success rate based on data received within 7 seconds
        int successCount = 0;
        int totalCount = 0;
        for (const auto &entry : nodeDataMaps) {
            const NodeDatas &node = entry.second;
            std::time_t currentTime = std::time(nullptr);
            double timeDiff = std::difftime(currentTime, node.timestamp);
            if (timeDiff <= 7) {
                successCount++;
            }
            totalCount++;
        }
        // double successRate = (totalCount > 0) ? (static_cast<double>(successCount) / totalCount) * 100 : 0.0;
        // Serial.print("Success Rate: ");
        // Serial.print(successRate);
        // Serial.print("%");
        calculateSuccessRates();
        Serial.print("Success Rate: ");
        Serial.print(node.dataSuccessRate);
        Serial.print("%");
        Serial.print(" | Time Ago: ");
        Serial.print(std::difftime(std::time(nullptr), node.timestamp));
        Serial.println(" seconds ago");
        // Print free heap memory in KB
        Serial.print("Free Heap: ");
        Serial.print(ESP.getFreeHeap() / 1024);
        Serial.println(" KB");
        Serial.println("--------------------------");
    }
}
