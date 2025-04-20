#include <ArduinoJson.h>
#include "main.h"
#include "LoRa.h"

void setMode(uint8_t mode);
void configureE32(String json);
void parseUartSpeed(String str, uint8_t &baud, uint8_t &parity);
uint8_t parseAirDataRate(String rate);
uint8_t parsePower(int power);
bool verifyResponse(uint8_t *cmd);
void updateBaudRate(String uartSpeed);

void LoRaFunction::LoRaSetup() {
  pinMode(M0, OUTPUT);
  pinMode(M1, OUTPUT);
  setMode(NORMAL_MODE);

  Serial.begin(115200);
  Serial2.begin(9600); // Mặc định baud rate 9600 ở chế độ cấu hình
}

void LoRaFunction::LoRaLoop() {
  if (Serial.available()) {
    String json = Serial.readStringUntil('\n');
    configureE32(json);
  }
}

void setMode(uint8_t mode) {
  digitalWrite(M0, (mode & 0b10) >> 1);
  digitalWrite(M1, mode & 0b01);
  delay(50);
}

void LoRaFunction::configureE32(String json) {
    SupendTask();
  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, json);
  if (error) {
    Serial.print("Lỗi JSON: ");
    Serial.println(error.c_str());
    return;
  }

  // Phân tích tham số từ JSON
  uint16_t address = doc["address"];
  uint8_t addrh = (address >> 8) & 0xFF;
  uint8_t addrl = address & 0xFF;

  String uartSpeed = doc["uart_speed"];
  uint8_t baudBits, parityBits;
  parseUartSpeed(uartSpeed, baudBits, parityBits);

  String airRate = doc["air_data_rate"];
  uint8_t airDataBits = parseAirDataRate(airRate);
  uint8_t spedByte = (baudBits << 5) | (parityBits << 3) | airDataBits;

  uint8_t chan = doc["channel"];
  if (chan > 30) chan = 23;

  // Tạo byte cấu hình
  uint8_t optionByte = 0;
  if (doc["transmission_mode"] == "fixed") optionByte |= 0x80;
  if (!doc["pull_up"]) optionByte |= 0x40;
  if (doc["fec"]) optionByte |= 0x10;
  optionByte |= parsePower(doc["power"]);

  // Tạo lệnh cấu hình
  uint8_t configCommand[8] = {
    0xC0, 0xC1, 0xC2, // Header
    addrh, addrl,      // Địa chỉ
    spedByte,           // Tốc độ
    chan,               // Kênh
    optionByte          // Tùy chọn
  };

  // Gửi lệnh cấu hình
  setMode(CONFIG_MODE);
  delay(100);
  Serial2.write(configCommand, 8);
  
  // Kiểm tra phản hồi
  bool success = verifyResponse(configCommand);
  setMode(NORMAL_MODE);

  // Cập nhật baud rate
  if (success) {
    updateBaudRate(uartSpeed);
    Serial.println("Cấu hình thành công!");
  } else {
    Serial.println("Cấu hình thất bại!");
  }
ResumeTask();
}

// Hàm hỗ trợ
void parseUartSpeed(String str, uint8_t &baud, uint8_t &parity) {
  int idx = str.indexOf('_');
  long baudVal = str.substring(0, idx).toInt();
  String parityStr = str.substring(idx+1);

  baud = (baudVal == 115200) ? 0b111 :
         (baudVal == 57600)  ? 0b110 :
         (baudVal == 38400)  ? 0b101 :
         (baudVal == 19200)  ? 0b100 : 0b011;

  parity = parityStr.startsWith("8O") ? 0b01 :
          parityStr.startsWith("8E") ? 0b10 : 0b00;
}

uint8_t parseAirDataRate(String rate) {
  return (rate == "0.3k")  ? 0b000 :
         (rate == "1.2k")  ? 0b001 :
         (rate == "4.8k")  ? 0b011 :
         (rate == "9.6k")  ? 0b100 : 0b010;
}
uint8_t parsePower(int power) {
    return (power == 30) ? 0b0000 : // 1W (30dBm)
                 (power == 27) ? 0b0001 :
                 (power == 24) ? 0b0010 :
                 (power == 21) ? 0b0011 : 0b0000;
}


bool verifyResponse(uint8_t *cmd) {
  delay(100);
  uint8_t response[8];
  int i = 0;
  while (Serial2.available() && i < 8) {
    response[i++] = Serial2.read();
  }
  return (i == 8) && (memcmp(cmd, response, 8) == 0);
}

void updateBaudRate(String uartSpeed) {
  int idx = uartSpeed.indexOf('_');
  long newBaud = uartSpeed.substring(0, idx).toInt();
  Serial2.end();
  Serial2.begin(newBaud);
}