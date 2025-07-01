#ifdef USE_TCP
#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

WiFiServer* tcpServer = nullptr;
WiFiClient tcpClient;   // dùng khi ESP là client
bool isTCPClientConnected = false;


void handleTCPServerResponse();
void serialHandler();
void stopAndDeleteTCPServer();
void handleTCPclient();

void TcpSetup()
{
  tcpServer = new WiFiServer(TCP_PORT);
}

void TcpLoop() {
  if (tcpServer) {
    handleTCPclient(); // Xử lý client kết nối
    handleTCPServerResponse(); // Xử lý phản hồi từ server nếu là client
  }
}



StaticJsonDocument<200> parseClientJson(const String& msg) {
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, msg);
  StaticJsonDocument<200> result;
  if (!error) {
    result["role"] = doc["role"] | "";
    result["status"] = doc["status"] | "";
    result["data"] = doc["data"] | "";
    result["ip"] = doc["ip"] | "";
    result["port"] = doc["port"] | "";
  } else {
    result["role"] = "";
    result["status"] = "";
    result["data"] = "";
    result["ip"] = "";
    result["port"] = "";
  }
  return result;
}

bool role = 0;
bool status = 1;
bool currentRole;

void TCPstatus(bool Status){status = Status;}

void handleTCPclient() {
  if (!tcpServer) return; // Nếu server chưa được khởi tạo
  WiFiClient client = tcpServer->available();
  if (client) {
      Serial.println("New client connected");
      // Xử lý client kết nối
      while (client.connected()) {
        if (client.available()) {
          char msg[256] = {0};
          size_t len = client.readBytesUntil('\n', msg, sizeof(msg) - 1);
          msg[len] = '\0';

          if (role == 0) {
            if (status == 0)
            {
            Serial.println("Client requested disconnect");
            client.println("Disconnected by server");
            break; // Thoát khỏi vòng lặp để ngắt kết nối client
            }
            else if (len > 0)
            {
            Serial.println("Client data: " + String(msg));
            client.println("Server received: " + String(msg));
            }
            else {
            client.println("No data received");
            }
          }
          else {
            client.println("Invalid role");
          }
        }
      }
      if(!client) return; 
      
      Serial.println("Client disconnected");
      client.stop();     
  }
}

void stopAndDeleteTCPServer() {
  if (tcpServer) {
    tcpServer->stop();
    delete tcpServer;
    tcpServer = nullptr;
    Serial.println("TCP Server stopped and deleted");
  }
}


void TCP_Config(char* ip, bool role, bool status, int port = TCP_PORT) {
    if (role == 1) {//TCP server
      currentRole = 1;
      Serial.println("Server IP: " + String(ip));
      Serial.println("Server Port: " + port);
      stopAndDeleteTCPServer(); // Dừng và xóa server cũ nếu có
      if(status == 1){
        tcpServer = new WiFiServer(port != 0 ? port : 80);
        tcpServer->begin();
        Serial.println("i am TCP Server with port " + String(port != 0 ? port : 80));
        Serial.println("Server started");
      }
      if(status == 0) {
        stopAndDeleteTCPServer(); // Dừng và xóa server cũ nếu có
      }
    }
    
    else if (role == 0) {//TCP client
    if (status == 1) {
      currentRole = 0;
      Serial.println("Server IP: " + String(ip));
      Serial.println("Server Port: " + port);
      // Thiết lập client TCP
      WiFiClient client;
      if (tcpClient.connect(ip, port)) {
        isTCPClientConnected = true;
        Serial.println("Connected to server at " + String(ip) + ":" + String(port));
        client.println("i am TCP client");
      } else {
        Serial.println("Connection failed");
      }
    } 
    else if (status == 0) {
      if (tcpClient.connected()) {
        tcpClient.stop();
        isTCPClientConnected = false;
        Serial.println("Disconnecting from server");
      }
    }      
  }
}


void handleTCPServerResponse() {

  if (isTCPClientConnected && tcpClient.connected()) {
    while (tcpClient.available()) {
      char serverMsg[256] = {0};
      size_t len = tcpClient.readBytesUntil('\n', serverMsg, sizeof(serverMsg) - 1);
      serverMsg[len] = '\0';
      Serial.println("Received from server: " + String(serverMsg));
      if (role == 1) {
        Serial.println("Server role confirmed");
        if (status == 0) {
          Serial.println("Server requested disconnect");
          tcpClient.stop();
          isTCPClientConnected = false;
          Serial.println("Disconnected from server");
          return; // Thoát khỏi hàm nếu server yêu cầu ngắt kết nối
        } else if (sizeof(serverMsg) > 0) {
          Serial.println("Server data: " + String(serverMsg));
        } else {
          Serial.println("No data received from server");
        }
      } else {
        Serial.println("Invalid role from server: " + String(role));
      }
    }
  }
}
#endif // USE_TCP