

#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Hàm xử lý chuỗi JSON
void handleJsonCommand(const std::string& jsonStr) {
    try {
        json j = json::parse(jsonStr);

        std::string cmnd = j.value("cmnd", "");
        std::string name = j.value("Name", "");
        std::string type = j.value("type", "");
        int size = j.value("size", 0);

        if (cmnd == "upload") {
            std::cout << "Upload file: " << name << ", size: " << size << ", type: " << type << std::endl;
            // Xử lý upload
        } else if (cmnd == "delete") {
            std::cout << "Delete " << type << ": " << name << std::endl;
            // Xử lý delete
        } else if (cmnd == "download") {
            std::cout << "Download " << type << ": " << name << std::endl;
            // Xử lý download
        } else if (cmnd == "list") {
            std::cout << "List folder: " << name << std::endl;
            // Xử lý list
        } else {
            std::cout << "Unknown command: " << cmnd << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
    }
}

// Ví dụ sử dụng
int main() {
    handleJsonCommand(R"({"cmnd":"upload","Name":"/folder2","size":56,"type":"file"})");
    handleJsonCommand(R"({"cmnd":"delete","Name":"/","type":"file"})");
    handleJsonCommand(R"({"cmnd":"download","Name":"/","type":"file"})");
    handleJsonCommand(R"({"cmnd":"list","Name":"/"})");
    return 0;
}