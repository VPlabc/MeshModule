#include "webserver.h"
#include "config.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// AsyncWebServer server(80);
// DNSServer dnsServer;

void startWebServer() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        String html = "<html lang='vi'><h1>Cấu hình ESP32</h1><form method='post' action='/save'>"
                      "MAC: <input name='mac' value='" + String(config.mac) + "'><br>"
                      "Node ID: <input name='node_id' value='" + String(config.node_id) + "'><br>"
                      "regBit: <input name='regBit' value='" + String(config.regBit) + "'><br>"
                      "reg8: <input name='reg8' value='" + String(config.reg8) + "'><br>"
                      "reg16: <input name='reg16' value='" + String(config.reg16) + "'><br>"
                      "Role: <select name='role'>"
                      "<option value='Node'" + String(config.role == "Node" ? " selected" : "") + ">Node</option>"
                      "<option value='Bridge'" + String(config.role == "Bridge" ? " selected" : "") + ">Bridge</option>"
                      "<option value='Broker'" + String(config.role == "Broker" ? " selected" : "") + ">Broker</option>"
                      "</select><br>"
                      "SSID: <input name='ssid' value='" + String(config.ssid) + "'><br>"
                      "Password: <input name='password' value='" + String(config.password) + "'><br>"
                      "<input type='submit' value='Lưu'></form></html>";
        request->send(200, "text/html", html);
    });

    server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (request->hasParam("mac", true) &&
            request->hasParam("node_id", true) &&
            request->hasParam("regBit", true) &&
            request->hasParam("reg8", true) &&
            request->hasParam("reg16", true) &&
            request->hasParam("role", true) &&
            request->hasParam("ssid", true) &&
            request->hasParam("password", true)) {

            strlcpy(config.mac, request->getParam("mac", true)->value().c_str(), sizeof(config.mac));
            config.node_id = request->getParam("node_id", true)->value().toInt();
            config.regBit = request->getParam("regBit", true)->value().toInt();
            config.reg8 = request->getParam("reg8", true)->value().toInt();
            config.reg16 = request->getParam("reg16", true)->value().toInt();
            config.role = request->getParam("role", true)->value();
            strlcpy(config.ssid, request->getParam("ssid", true)->value().c_str(), sizeof(config.ssid));
            strlcpy(config.password, request->getParam("password", true)->value().c_str(), sizeof(config.password));

            saveConfig();
            request->send(200, "text/html", "<html lang='vi'>Lưu cấu hình thành công!</html>");
        } else {
            request->send(400, "text/html", "<html lang='vi'>Thiếu tham số!</html>");
        }
    });

    server.begin();
}
