#include <Arduino.h>
#include <vector>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include "mbedtls/md.h" // For SHA-256 hashing
#include <time.h>
#include <mbedtls/md.h>
#include <mbedtls/md5.h>
#include <mbedtls/sha256.h>
#include <LittleFS.h>
#include "ssl_client.h"
#include "ASyncElegantOTA.h" 

byte DoorState = 0; // 0: Closed, 1: Open, 2: Stopped


// Quản lý phiên đăng nhập
String generateToken(int length) {
    String token = "";
    for (int i = 0; i < length; i++) {
        // Tạo ký tự ngẫu nhiên từ a-z, A-Z, 0-9
        char c = random(0, 62);
        if (c < 10) {
        token += char('0' + c); // 0-9
        } else if (c < 36) {
        token += char('A' + c - 10); // A-Z
        } else {
        token += char('a' + c - 36); // a-z
        }
    }
    return token;
}

class Sessions {
private:
  struct Session {
    String token;
    String user;
    unsigned long expiry;
  };
  
  std::vector<Session> sessions;
  
public:
  String create(String user) {
    // Tạo token ngẫu nhiên
    String token = generateToken(32);
    
    // Thời gian hết hạn (1 giờ)
    Session newSession{
      token,
      user,
      millis() + 3600000 
    };
    
    sessions.push_back(newSession);
    return token;
  }
  
  bool isValid(String token) {
    cleanup();
    for(auto& s : sessions) {
      if(s.token == token) return true;
    }
    return false;
  }
  
  String getUser(String cookie) {
    // ... trích xuất user từ cookie
  }
  
  void cleanup() {
    unsigned long now = millis();
    sessions.erase(
      std::remove_if(sessions.begin(), sessions.end(), 
        [now](Session s){ return s.expiry < now; }),
      sessions.end()
    );
  }
};

#include "History.h"
HistoryStorage historyStorage;
#define RECORD_POOL_SIZE 500
HistoryRecord recordPool[RECORD_POOL_SIZE];
int freeIndex = 0;

HistoryRecord* allocateRecord() {
    if(freeIndex >= RECORD_POOL_SIZE) return nullptr;
    return &recordPool[freeIndex++];
}

// Cấu hình WiFi và SSL
const char* SSID = "I-Soft";
const char* PASS = "i-soft@2023";

// SSL Certificate (self-signed)
const char* ssl_cert = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDXTCCAkWgAwIBAgIUJeWct5bU0x0Hk0...\n" \
"-----END CERTIFICATE-----\n";

const char* ssl_key = \
"-----BEGIN PRIVATE KEY-----\n" \
"MIIEvQIBADANBgkqhkiG9w0BAQEFAASC...\n" \
"-----END PRIVATE KEY-----\n";

AsyncWebServer DLserver(443);
Sessions sessions; // Quản lý phiên

// Tạo salt ngẫu nhiên
String generateSalt(int length = 16) {
  String salt = "";
  for (int i = 0; i < length; i++) {
    salt += char(random(65, 90));
  }
  return salt;
}

enum ActionType {
  LOGIN = 0,
  LOGOUT = 1,
  DOOR_OPEN = 2,
  DOOR_CLOSE = 3,
  // Thêm các hành động khác nếu cần
};
// Băm mật khẩu với SHA-256
String hashPassword(const String& password, const String& salt) {
  unsigned char hash[32];
  String data = salt + password;
  
  mbedtls_md_context_t ctx;
  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 0);
  
  mbedtls_md_starts(&ctx);
  mbedtls_md_update(&ctx, (const unsigned char*)data.c_str(), data.length());
  mbedtls_md_finish(&ctx, hash);
  mbedtls_md_free(&ctx);

  String result = "";
  for(int i=0; i<32; i++){
    char str[3];
    sprintf(str, "%02x", hash[i]);
    result += str;
  }
  return result;
}

// Middleware kiểm tra đăng nhập
bool checkAuth(AsyncWebServerRequest *request) {
  if(!request->hasHeader("Cookie")) return false;
  
  String cookie = request->header("Cookie");
  int tokenStart = cookie.indexOf("session_token=");
  if(tokenStart == -1) return false;
  
  int tokenEnd = cookie.indexOf(";", tokenStart);
  String token = (tokenEnd == -1) ? 
    cookie.substring(tokenStart + 14) : 
    cookie.substring(tokenStart + 14, tokenEnd);
  
  return sessions.isValid(token);
}

///////////////////// Register /////////////////////
// [
//   {
//     "id": "user1",
//     "password_hash": "a665a459...", // SHA-256 hash
//     "salt": "random_salt"
//   }
// ]
bool userExists(const String& user) {
    File file = LittleFS.open("/users.json", "r");
    if(!file) {
        Serial.println("Failed to open users file");
        return false;
    }
    const char* content = file.readString().c_str();
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, content);
    if(error) {
        Serial.println("Failed to parse users file");
        file.close();
        return false;
    }
    for(JsonObject userObj : doc.as<JsonArray>()) {
        if(userObj["id"] == user) {
            file.close();
            return true; // User exists
        }
    }
    file.close();
  return false; // Thay thế bằng logic thực tế
}
// Thêm người dùng mới
void addUser(const String& user, const String& passwordHash, const String& salt) {
    File file = LittleFS.open("/users.json", "a+");
    if(!file) {
        Serial.println("Failed to open users file for writing");
        return;
    }
    
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, file);
    if(error) {
        Serial.println("Failed to parse users file, creating new one");
        doc.clear();
    }
    
    JsonObject newUser = doc.createNestedObject();
    newUser["id"] = user;
    newUser["password_hash"] = passwordHash;
    newUser["salt"] = salt;
    
    file.seek(0); // Đặt con trỏ về đầu file
    serializeJson(doc, file);
    file.close();
}
///////////////////// Login /////////////////////
// Kiểm tra thông tin đăng nhập
int idx = 0;
bool validateUser(const String& user, const String& password) {
    File file = LittleFS.open("/users.json", "r");
    if(!file) {
        Serial.println("Failed to open users file");
        return false;
    }
    
    const char* content = file.readString().c_str();
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, content);
    if(error) {
        Serial.println("Failed to parse users file");
        file.close();
        return false;
    }
    
    for(JsonObject userObj : doc.as<JsonArray>()) {
        idx++;
        if(userObj["id"] == user) {
            String salt = userObj["salt"];
            String hashedPass = hashPassword(password, salt);
            if(hashedPass == userObj["password_hash"]) {
                file.close();
                return true; // Đăng nhập thành công
            }
        }
    }
    
    file.close();
    return false; // Thông tin đăng nhập không hợp lệ
}

void logAction(uint16_t user, uint8_t action) {
    HistoryRecord* record = allocateRecord();
    if(!record) return; // Không còn chỗ trống
    // Lưu vào file hoặc cơ sở dữ liệu
    historyStorage.addRecord(user, action);
    // Hoặc sử dụng recordPool để lưu trữ tạm thời
    // Ví dụ: 
    Serial.printf("User: %s, Action: %s, Time: %lu\n", String(user), String(action), record->timestamp);
}

// Lấy lịch sử đăng nhập
String getHistory(const String& user) {
    File file = LittleFS.open("/history.json", "r");
    if(!file) {
        Serial.println("Failed to open history file");
        return "[]"; // Trả về mảng rỗng nếu không tìm thấy
    }
    
    String history = "[";
    const char* content = file.readString().c_str();
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, content);
    if(error) {
        Serial.println("Failed to parse history file");
        file.close();
        return "[]";
    }
    
    for(JsonObject entry : doc.as<JsonArray>()) {
        if(entry["user"] == user) {
            history += "{\"time\":\"" + String(entry["time"].as<const char*>())
                    + "\",\"action\":\"" + String(entry["action"].as<const char*>()) + "\"},";
        }
    }
    
    if(history.length() > 1) {
        history.remove(history.length() - 1); // Xóa dấu phẩy cuối
    }
    
    history += "]";
    file.close();
    return history;
}
// Định tuyến chính
void setupServer() {
   
  // Trang đăng ký
DLserver.on("/register", HTTP_GET, [](AsyncWebServerRequest *request){
    // Chỉ cho phép admin hoặc staff truy cập trang đăng ký
    if (!checkAuth(request)) {
        request->redirect("/login");
        return;
    }
    String user = sessions.getUser(request->header("Cookie"));
    // Kiểm tra quyền admin hoặc staff (giả sử user là "admin" hoặc "staff")
    if (user != "admin" && user != "staff") {
        request->send(403, "text/plain", "Forbidden: Only admin or staff can register new users.");
        return;
    }
    request->send(LittleFS, "/register.html");
});

  DLserver.on("/register", HTTP_POST, [](AsyncWebServerRequest *request){
    // Xử lý đăng ký
    String user = request->arg("username");
    String pass = request->arg("password");
    
    // Kiểm tra user tồn tại
    if(userExists(user)) {
      request->send(400, "text/plain", "User exists");
      return;
    }
    
    // Tạo salt và băm mật khẩu
    String salt = generateSalt();
    String hashedPass = hashPassword(pass, salt);
    
    // Lưu user mới
    addUser(user, hashedPass, salt);
    
    request->send(201, "text/plain", "Registered!");
  });

  // Trang đăng nhập
  DLserver.on("/login", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/login.html");
  });

  DLserver.on("/login", HTTP_POST, [](AsyncWebServerRequest *request){
    String user = request->arg("username");
    String pass = request->arg("password");
    
    if(validateUser(user, pass)) {
      // Tạo phiên mới
      String token = sessions.create(user);
        Serial.printf("User %s [%d] logged in with token %s\n", user.c_str(), idx , token.c_str());
      // Ghi log
      logAction(idx, LOGIN);
      
      // Gửi cookie
      AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "");
      response->addHeader("Location", "/dashboard");
      response->addHeader("Set-Cookie", "session_token=" + token + "; HttpOnly; Secure; Max-Age=3600");
      request->send(response);
    } else {
      request->send(401, "text/plain", "Invalid credentials");
    }
  });

  // Trang lịch sử
  DLserver.on("/history", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!checkAuth(request)) {
      request->redirect("/login");
      return;
    }
    
    String user = sessions.getUser(request->header("Cookie"));
    String historyStr = getHistory(user);
    int page = 0;
    if (request->hasParam("page")) {
      page = request->getParam("page")->value().toInt();
    }
    int perPage = 50;

    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, historyStr);
    JsonArray arr = doc.as<JsonArray>();
    int total = arr.size();
    int start = page * perPage;
    int end = (start + perPage < total) ? (start + perPage) : total;

    String html = "<h1>Login History</h1><ul>";
    for (int i = start; i < end; ++i) {
      JsonObject entry = arr[i];
      html += "<li>" + String((const char*)entry["time"]) + " - " + String((const char*)entry["action"]) + "</li>";
    }
    html += "</ul>";
    
    request->send(200, "text/html", html);
  });

  // Middleware bảo vệ
  DLserver.on("/dashboard", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!checkAuth(request)) {
      request->redirect("/login");
      return;
    }
    request->send(LittleFS, "/dashboard.html");
  });

// API: Cập nhật trạng thái cửa (open/close)
DLserver.on("/api/door", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!checkAuth(request)) {
        request->send(401, "application/json", "{\"error\":\"Unauthorized\"}");
        return;
    }
    if (!request->hasParam("action", true)) {
        request->send(400, "application/json", "{\"error\":\"Missing action parameter\"}");
        return;
    }
    String action = request->getParam("action", true)->value();
    String user = sessions.getUser(request->header("Cookie"));
    uint8_t actionType = 0xFF;
    if (action == "open") {
        actionType = DOOR_OPEN;
        // TODO: Thực hiện mở cửa vật lý ở đây
    } else if (action == "close") {
        actionType = DOOR_CLOSE;
        // TODO: Thực hiện đóng cửa vật lý ở đây
    } else {
        request->send(400, "application/json", "{\"error\":\"Invalid action\"}");
        return;
    }
    logAction(user.toInt(), actionType);
    request->send(200, "application/json", "{\"status\":\"success\"}");
});

// API: Lấy trạng thái cửa cho FE
DLserver.on("/api/door/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!checkAuth(request)) {
        request->send(401, "application/json", "{\"error\":\"Unauthorized\"}");
        return;
    }

    String doorStatus = String(DoorState == 1 ? "open" : "closed");
    request->send(200, "application/json", "{\"status\":\"" + doorStatus + "\"}");
});

  DLserver.on("/tool", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream("text/html");
        response->print("<!DOCTYPE html><html><head><title>System File Tool</title></head><style>");
        response->print("body { font-family: Arial, sans-serif; margin: 20px; }");
        response->print("h1 { color: #333; }");
        response->print("button { margin-top: 10px; padding: 10px; background-color: #4CAF50; color: white; border: none; cursor: pointer; }");
        response->print("button:hover { background-color: #45a049; }");
        response->print("table { width: 100%; border-collapse: collapse; margin-top: 20px; }");
        response->print("th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }");
        response->print("th { background-color: #f2f2f2; }");
        response->print("tr:hover { background-color: #f5f5f5; }");
        response->print("td button { background-color: #f44336; color: white; border: none; cursor: pointer; padding: 5px 10px; }");
        response->print("td button:hover { background-color: #d32f2f; }");
        response->print("</style></head><body><CENTER>");
        response->print("<h2>File Manager</h2>");
        response->print("<table><thead><tr><th>File Name</th><th>Action</th></tr></thead>");
        response->print("<tbody id=\"fileList\"></tbody></table>");
        response->print("<div><h3>Upload File</h3>");
        response->print("<input type='file' id='fileInput'><button onclick='uploadFile()'>Upload</button></div></CENTER>");
        response->print("<script>");
        response->print("function fetchFileList() {");
        response->print("fetch('/list-files').then(response => response.json()).then(files => {");
        response->print("const fileList = document.getElementById('fileList'); fileList.innerHTML = '';");
        response->print("files.forEach(file => {");
        response->print("const row = document.createElement('tr');");
        response->print("const fileNameCell = document.createElement('td'); fileNameCell.textContent = file;");
        response->print("const actionCell = document.createElement('td');");
        response->print("const deleteButton = document.createElement('button'); deleteButton.textContent = 'Delete';");
        response->print("deleteButton.onclick = () => deleteFile(file); actionCell.appendChild(deleteButton);");
        response->print("row.appendChild(fileNameCell); row.appendChild(actionCell); fileList.appendChild(row); }); }); }");
        response->print("function uploadFile() {");
        response->print("const fileInput = document.getElementById('fileInput'); const file = fileInput.files[0];");
        response->print("if (!file) { alert('Please select a file to upload.'); return; }");
        response->print("const formData = new FormData(); formData.append('file', file);");
        response->print("fetch('/upload', { method: 'POST', body: formData }).then(response => {");
        response->print("if (response.ok) { alert('File uploaded successfully.'); fetchFileList(); } else { alert('Failed to upload file.'); } }); }");
        response->print("function deleteFile(fileName) {");
        response->print("fetch(`/delete?file=${encodeURIComponent(fileName)}`, { method: 'DELETE' }).then(response => {");
        response->print("if (response.ok) { alert('File deleted successfully.'); fetchFileList(); } else { alert('Failed to delete file.'); } }); }");
        response->print("fetchFileList();");
        response->print("</script></body></html>");
        request->send(response);
    });
    // API: Upload file
    DLserver.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (!checkAuth(request)) {
            request->send(401, "text/plain", "Unauthorized");
            return;
        }
        request->send(200, "text/plain", "File uploaded");
    }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
        static File uploadFile;
        if (index == 0) {
            String path = "/" + filename;
            uploadFile = LittleFS.open(path, "w");
        }
        if (uploadFile) {
            uploadFile.write(data, len);
        }
        if (final && uploadFile) {
            uploadFile.close();
        }
    });

    // API: Delete file
    DLserver.on("/delete", HTTP_DELETE, [](AsyncWebServerRequest *request) {
        if (!checkAuth(request)) {
            request->send(401, "text/plain", "Unauthorized");
            return;
        }
        if (!request->hasParam("file")) {
            request->send(400, "text/plain", "Missing file parameter");
            return;
        }
        String filename = request->getParam("file")->value();
        if (!filename.startsWith("/")) filename = "/" + filename;
        if (LittleFS.exists(filename)) {
            LittleFS.remove(filename);
            request->send(200, "text/plain", "File deleted");
        } else {
            request->send(404, "text/plain", "File not found");
        }
    });

    // API: List files (for file manager)
    DLserver.on("/list-files", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (!checkAuth(request)) {
            request->send(401, "application/json", "[]");
            return;
        }
        String json = "[";
        File root = LittleFS.open("/");
        File file = root.openNextFile();
        while (file) {
            json += "\"" + String(file.name()) + "\"";
            file = root.openNextFile();
            if (file) json += ",";
        }
        json += "]";
        request->send(200, "application/json", json);
    });
}

void DoorSetup() {
  Serial.begin(115200);
  
  // Khởi tạo LittleFS
  if(!LittleFS.begin(true)){
    Serial.println("LittleFS Mount Failed");
    return;
  }

  // Kết nối WiFi
  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  AsyncElegantOTA.begin(&DLserver, "admin", "admin@123"); // Khởi tạo OTA với username và password
  // Cấu hình thời gian
  configTime(7, 0, "pool.ntp.org");
  Serial.println(" ⏰  Time configured");
  setupServer();

  // Khởi động server
  DLserver.begin();
}

void DoorLoop() {
  sessions.cleanup(); // Dọn dẹp session hết hạn
}