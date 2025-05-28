# MeshModule

MeshModule là một dự án sử dụng ESP32 để xây dựng mạng lưới (mesh network) với các tính năng như giao tiếp ESP-NOW, Modbus RTU, MQTT, và hỗ trợ giao diện web để cấu hình. Dự án hỗ trợ nhiều loại board khác nhau và có khả năng mở rộng để tích hợp các module LoRa.

---

## **Tính năng chính**
- **ESP-NOW**: Giao tiếp không dây giữa các thiết bị ESP32.
- **Modbus RTU**: Giao tiếp qua RS485 để đọc/ghi dữ liệu từ các thiết bị Modbus.
- **MQTT**: Kết nối với MQTT Broker để gửi/nhận dữ liệu.
- **LoRa**: Hỗ trợ giao tiếp LoRa (có thể bật/tắt qua cấu hình).
- **WiFi AP Mode**: Chế độ Access Point để cấu hình thiết bị qua giao diện web.
- **Giao diện web**: Cấu hình các thông số như WiFi, MQTT, LoRa, và Modbus.
- **Hỗ trợ nhiều loại board**:
  - `0`: Board ModRTUMesh
  - `1`: Board 410WER
  - `2`: Board LklineGw
  - `3`: Board LklineNode

---

## **Cấu trúc dự án**
```
MeshModule/ 
    ├── data/ # Giao diện web (HTML, CSS, JS) 
    ├── src/ # Mã nguồn chính 
    │ ├── main.cpp # Tệp chính của dự án 
    │ ├── TskMQTT.h # Header file cho MQTT 
    │ ├── WebInterface.h # Header file cho giao diện web 
    │ ├── Modbus_RTU.h # Header file cho Modbus RTU 
    │ ├── LoRa.h # Header file cho LoRa 
    │ └── RTC_Online.h # Header file cho RTC Online
    ├── platformio.ini # Cấu hình PlatformIO 
    └── README.md # Tài liệu dự án

---

## **Cách sử dụng**

### **1. Cài đặt môi trường**
- Cài đặt [PlatformIO](https://platformio.org/) trên Visual Studio Code.
- Clone dự án:
  ```bash
  git clone https://github.com/vplabc/MeshModule.git
  cd MeshModule
  ```

Các hàm chính
1. WiFi
  setupWiFiAP(): Khởi động chế độ Access Point.
  checkConfigButton(): Kiểm tra nút nhấn để vào chế độ cấu hình.

2. ESP-NOW
  receiveCallback(): Callback khi nhận dữ liệu qua ESP-NOW.
  sentCallback(): Callback khi gửi dữ liệu qua ESP-NOW.

3. LoRa
  mainLoRa.LoRaSetup(): Cấu hình LoRa.
  mainLoRa.LoRaLoop(): Vòng lặp xử lý LoRa.

4. Modbus
  ModbusInit(): Khởi tạo Modbus RTU.
  ModbusLoop(): Vòng lặp xử lý Modbus.

Value: địa chỉ thanh ghi chip
value + addressOffset: địa chỉ thanh ghi thiế bị


5. MQTT
  MQTTwifiConfig.setup(): Cấu hình MQTT.
  MQTTwifiConfig.loop(): Vòng lặp xử lý MQTT.
  
Debug
  Bật/tắt debug bằng cách thay đổi giá trị:
  ```cpp
  MeshConfig.debug = true; // Bật debug
  MeshConfig.debug = false; // Tắt debug
  ```
### **2. Cấu hình dự án**
- Mở tệp `platformio.ini` và cấu hình các thông số như WiFi, MQTT, LoRa, và Modbus.
- Tải mã nguồn lên ESP32 bằng cách nhấn nút "Upload" trong PlatformIO.

Tên: [Vĩnh Phát]
Email: [PhatHoang]
GitHub: [github.com/vplabc]
