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
    char macAddress[18]; // MAC address as "XX:XX:XX:XX:XX:XX" + null terminator
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
void updateNodeData(const dataPacket &packet, const char* macAddress) {
    nodeDatas.ID = packet.ID;
    nodeDatas.netId = packet.netId;
    nodeDatas.data.assign(packet.data, packet.data + packet.dataSize);
    nodeDatas.dataSize = packet.dataSize;
    nodeDatas.timestamp = std::time(nullptr); // Get the current time

    auto currentTime = std::chrono::steady_clock::now();
    nodeDatas.lastReceivedTimes = currentTime;

    // Copy MAC address to fixed-size char array
    strncpy(nodeDatas.macAddress, macAddress, sizeof(nodeDatas.macAddress) - 1);
    nodeDatas.macAddress[sizeof(nodeDatas.macAddress) - 1] = '\0';

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

void LoadDataMapping(char* buffer, size_t bufferSize) {
    // Load data mapping from JSON file
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
        size_t len = file.readBytes(buffer, bufferSize - 1);
        buffer[len] = '\0'; // Null-terminate
        file.close();
    } else {
        Serial.println("Failed to open DataMapping.json");
        if (bufferSize > 0) buffer[0] = '\0';
    }
}

void createJsonForWebSocket(char* outBuffer, size_t outBufferSize) {
    DynamicJsonDocument doc(4096); // Adjust size as needed
    JsonArray nodesArray = doc.to<JsonArray>();

    char mappingBuffer[4096];
    LoadDataMapping(mappingBuffer, sizeof(mappingBuffer));

    DynamicJsonDocument mappingDoc(4096);
    DeserializationError error = deserializeJson(mappingDoc, mappingBuffer);

    JsonArray mappingArray = mappingDoc.as<JsonArray>();

    for (const auto &entry : nodeDataMaps) {
        const NodeDatas &node = entry.second;
        JsonObject nodeObject = nodesArray.createNestedObject();

        nodeObject["nodeId"] = node.ID;
        nodeObject["mac"] = node.macAddress;
        nodeObject["netId"] = node.netId;

        char successRateStr[16];
        snprintf(successRateStr, sizeof(successRateStr), "%.1f%%", node.dataSuccessRate);
        nodeObject["successRate"] = successRateStr;

        std::time_t currentTime = std::time(nullptr);
        double timeDiff = std::difftime(currentTime, node.timestamp);

        char timeAgoStr[32];
        snprintf(timeAgoStr, sizeof(timeAgoStr), "%d seconds ago", static_cast<int>(timeDiff));
        nodeObject["timeAgo"] = timeAgoStr;

        JsonArray dataArray = nodeObject.createNestedArray("data");
        for (uint8_t byte : node.data) {
            dataArray.add(byte);
        }

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
            nodeObject.createNestedArray("types");
            nodeObject.createNestedArray("keys");
        }
    }

    serializeJson(nodesArray, outBuffer, outBufferSize);
    doc.clear();
}

void createJsonForMqttt(char* outBuffer, size_t outBufferSize) {
    DynamicJsonDocument doc(4096); // Adjust size as needed
    JsonArray nodesArray = doc.to<JsonArray>();

    char mappingBuffer[4096];
    LoadDataMapping(mappingBuffer, sizeof(mappingBuffer));

    DynamicJsonDocument mappingDoc(4096);
    DeserializationError error = deserializeJson(mappingDoc, mappingBuffer);
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

    serializeJson(nodesArray, outBuffer, outBufferSize);
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
