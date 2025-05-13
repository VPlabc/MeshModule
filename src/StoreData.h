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

        JsonArray typeArray = nodeObject.createNestedArray("type");
        JsonArray keysArray = nodeObject.createNestedArray("keys");

        // Example types and keys, adjust as needed
        for (size_t i = 0; i < node.data.size(); ++i) {
            typeArray.add("DWORD"); // Replace with actual type logic
            keysArray.add("Key" + String(i + 1));
        }
    }

    String jsonString;
    serializeJson(nodesArray, jsonString);
    return jsonString;
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
            Serial.print(byte, HEX);
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
        Serial.println("--------------------------");
    }
}
