#include <ArduinoJson.h>
#include "main.h"
#include "LoRa.h"

#ifdef VERSION_1
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

#endif//VERSION_1

#ifdef VERSION_2
 /*
AT+RESET

AT+MODE=<Parameter>[,<RX time>,<Low speed time>]
<Parameter>range 0 to 2
0：Transceiver mode (default).
1：Sleep mode.
Example : Set to sleep mode.
AT+MODE=1
2 : Smart receiving power saving mode
The switch between receiving mode and low speed mode can
be used to achieve the effect of power saving, and the
appropriate transmission time must be adjusted by yourself to
match this mode.
<RX time>=30ms~60000ms, (default 1000)
<Low speed time>=30ms~60000ms, (default 1000)
When the correct LoRa® data format is received, it will return to
the transceiver mode.
When the received data is correct, +RCV format data will
be output.
Example : The Smart receiving power saving mode.
AT+MODE=2,3000,3000
Set to turn on receiving mode for 3 seconds and then low speed mode
for 3 seconds to cycle until the correct signal is received.

  AT+BAND=<Parameter>,<Frequency Memory>
<Parameter>is the RF Frequency, Unit is Hz
490000000: 490000000Hz(default: RYLY498)
915000000: 915000000Hz(default: RYLY998)
<Frequency Memory> M for memory
Example: Set the frequency as 868500000Hz.
AT+BAND=868500000
Example: Set the frequency as 868500000Hz
and be memorized in Flash.(Only support after
F/W version 1.2.0)
AT+BAND=868500000,M
get
AT+BAND?

  AT+PARAMETER=<Spreading Factor>,
  <Bandwidth>,<Coding Rate>,
  <Programmed Preamble>
  <Spreading Factor>5~11 (default 9)
  *SF7to SF9 at 125kHz, SF7 to SF10 at 250kHz, and SF7 to SF11 at 500kHz
  <Bandwidth>7~9, list as below：
  7: 125 KHz (default)
  8: 250 KHz
  9: 500 KHz
  <Coding Rate>1~4, (default 1)
  1: Coding Rate 4/5
  2 :Coding Rate 4/6
  3 :Coding Rate 4/7
  4 :Coding Rate 4/8
  <Programmed Preamble>(default 12)
  When NETWORKID=18, The value can be
  configured to 4~24.
  Other NETWORKID can only be configured to 12.
  Example: Set the parameters as below,
  <Spreading Factor> 7, <Bandwidth> 500KHz, <Coding
  Rate> 4, <Programmed Preamble> 15.
  AT+PARAMETER=7,9,4,15
get 
AT+PARAMETER?

  AT+CPIN=<Password>
<Password>An 8 character long password
From 00000001 to FFFFFFFF,
Only by using same password can the data be
recognized.
After resetting, the previously password will
disappear.
Example：Set the password to EEDCAA90
AT+CPIN=EEDCAA90
get
AT+CPIN?

AT+ADDRESS=<Address>
<Address>=0~65535 (default 0)
Example: Set the address of module as 120.
*The settings will be memorized in Flash.
AT+ADDRESS=120
get 
AT+ADDRESS?

AT+NETWORKID=<Network ID>
<NetworkID>=3~15,18(default18)
Example: Set the network ID as 6,
*The settings will be memorized in Flash.
AT+NETWORKID=6
get 
AT+NETWORKID?

AT+CRFOP=<power>
<power>0~22 dBm
22: 22dBm(default)
21: 21dBm
20: 20dBm
......
01: 1dBm
00: 0dBm
Example: Set the output power as 10dBm,
AT+CRFOP=10
* RF Output Power must be set to less than
AT+CRFOP=14 to comply CE certificatio
get
AT+CRFOP?

AT+UID?

AT+FACTORY
*/

HardwareSerial loraSerial(2); // Serial2 cho LoRa
#ifdef Module_RS485
#define LoRaTx  2
#define LoRaRx  15
#endif//Module_RS485
#ifdef Module_10O4I
#define LoRaTx  32
#define LoRaRx  33
#endif//Module_10O4I
bool LoRaInit = false;
// Khai báo hàm
void sendCommand(const String &cmd);
void sendData(String address, String data);
String hexToAscii(const String &hex);
void receiveData();

void LoRaFunction::configureLoRaModule(int address, int networkID, int power, long frequency, int spreadingFactor, int bandwidth, int codingRate, int preamble) {
  sendCommand("AT+RESET"); // Reset module
  delay(1000);

  // Set frequency
  sendCommand("AT+BAND=" + String(frequency) + ",M");

  // Set network ID
  sendCommand("AT+NETWORKID=" + String(networkID));

  // Set address
  sendCommand("AT+ADDRESS=" + String(address));

  // Set power
  sendCommand("AT+CRFOP=" + String(power));

  // Set LoRa parameters
  String paramCmd = "AT+PARAMETER=" + String(spreadingFactor) + "," + String(bandwidth) + "," + String(codingRate) + "," + String(preamble);
  sendCommand(paramCmd);

  Serial.println("LoRa module configured successfully.");
}

String LoRaFunction::getLoRaModuleConfig() {
  DynamicJsonDocument doc(512);

  // Get frequency
  loraSerial.println("AT+BAND?");
  delay(100);
  if (loraSerial.available()) {
    String response = loraSerial.readStringUntil('\n');
    if (response.startsWith("+BAND=")) {
      doc["frequency"] = response.substring(6);
    }
  }

  // Get network ID
  loraSerial.println("AT+NETWORKID?");
  delay(100);
  if (loraSerial.available()) {
    String response = loraSerial.readStringUntil('\n');
    if (response.startsWith("+NETWORKID=")) {
      doc["networkID"] = response.substring(11).toInt();
    }
  }

  // Get address
  loraSerial.println("AT+ADDRESS?");
  delay(100);
  if (loraSerial.available()) {
    String response = loraSerial.readStringUntil('\n');
    if (response.startsWith("+ADDRESS=")) {
      doc["address"] = response.substring(9).toInt();
    }
  }

  // Get power
  loraSerial.println("AT+CRFOP?");
  delay(100);
  if (loraSerial.available()) {
    String response = loraSerial.readStringUntil('\n');
    if (response.startsWith("+CRFOP=")) {
      doc["power"] = response.substring(7).toInt();
    }
  }

  // Get LoRa parameters
  loraSerial.println("AT+PARAMETER?");
  delay(100);
  if (loraSerial.available()) {
    String response = loraSerial.readStringUntil('\n');
    if (response.startsWith("+PARAMETER=")) {
      int firstComma = response.indexOf(',');
      int secondComma = response.indexOf(',', firstComma + 1);
      int thirdComma = response.indexOf(',', secondComma + 1);

      doc["spreadingFactor"] = response.substring(11, firstComma).toInt();
      doc["bandwidth"] = response.substring(firstComma + 1, secondComma).toInt();
      doc["codingRate"] = response.substring(secondComma + 1, thirdComma).toInt();
      doc["preamble"] = response.substring(thirdComma + 1).toInt();
    }
  }

  // Get UID
  loraSerial.println("AT+UID?");
  delay(100);
  if (loraSerial.available()) {
    String response = loraSerial.readStringUntil('\n');
    if (response.startsWith("+UID=")) {
      doc["uid"] = response.substring(5);
    }
  }

  // Serialize JSON and print
  String jsonString;
  serializeJson(doc, jsonString);
  return jsonString;
}

void LoRaFunction::LoRaSetup(int LoRaID) {
  Serial.println("RX: " + String(Ser_1RX) + " | TX: " + String(Ser_1TX));
  loraSerial.begin(9600, SERIAL_8N1, Ser_1RX , Ser_1TX); // RX, TX
  delay(1000);
 
  // Cấu hình module
  sendCommand("AT"); // Kiểm tra kết nối
  if(LoRaInit){
    sendCommand("AT+BAND=868500000,M"); // Đặt tần số 868MHz
    sendCommand("AT+NETWORKID=18");    // Đặt Network ID
    sendCommand("AT+ADDRESS="+String(LoRaID));    // Địa chỉ của thiết bị này
  }
}

void LoRaFunction::LoRaLoop(int SlaveID) {
  // Gửi dữ liệu mỗi 5 giây
  static unsigned long lastSend = 0;
  if (millis() - lastSend >= 1000) {
    sendData(String(SlaveID), "Hello LoRa!"); // Gửi "Hello LoRa!" đến địa chỉ 456
    lastSend = millis();
  }

  // Đọc dữ liệu đến
  receiveData();
}

// Hàm gửi lệnh AT và đọc phản hồi
void sendCommand(const String &cmd) {
  loraSerial.println(cmd);
  Serial.print("Sent: ");
  Serial.println(cmd);
  unsigned long startTime = millis();
  while (millis() - startTime < 500) {
    if (loraSerial.available()) {
    String response = loraSerial.readStringUntil('\n');
    Serial.print("Response: ");
    Serial.println(response);
    if (response.startsWith("+OK")) {
      LoRaInit = true;
      break;
    } else if (response.startsWith("+ERR")) {
      break;
    }
    }
  }
  if (millis() - startTime >= 5000) {
    Serial.println("Timeout: No response from LoRa module.");
    LoRaInit = false;
  }
}

// Hàm gửi dữ liệu dạng hex
void sendData(String address, String data) {
  String hexData;
  for (int i = 0; i < data.length(); i++) {
    char hex[3];
    sprintf(hex, "%02X", data[i]);
    hexData += hex;
  }
  
  String cmd = "AT+SEND=" + address + "," + String(data.length()) + "," + data;//hexData;
  loraSerial.println(cmd);
  Serial.print("Sent: ");
  Serial.println(cmd);
  unsigned long startTime = millis();
  while (millis() - startTime < 100) {
    if (loraSerial.available()) {
    String response = loraSerial.readStringUntil('\n');
    Serial.print("Response: ");
    Serial.println(response);
    if (response.startsWith("+OK")) {
      LoRaInit = true;
      break;
    } else if (response.startsWith("+ERR")) {
      break;
    }
    }
  }
}
bool LRonce = false;
// Hàm đọc và xử lý dữ liệu nhận được
void LoRaFunction::receiveData() {
  if(!LRonce) {
    LRonce = true;
    // delay(1000);
    
  Serial.println("RX: " + String(Ser_1RX) + " | TX: " + String(Ser_1TX));
    loraSerial.begin(9600, SERIAL_8N1, Ser_1RX , Ser_1TX); // RX, TX
    // delay(1000);
    sendCommand("AT"); // Kiểm tra kết nối
  }
  while (loraSerial.available()) {
    String response = loraSerial.readStringUntil('\n');
    Serial.print("Response: ");
    Serial.println(response);
  // }
    response.trim();
    digitalWrite(BUZZ, HIGH);delay(100);digitalWrite(BUZZ, LOW);
    if (response.startsWith("+RCV")) {
      // Phân tích dữ liệu
      int firstComma = response.indexOf(',', 5);
      int secondComma = response.indexOf(',', firstComma + 1);
      int thirdComma = response.indexOf(',', secondComma + 1);
      int fourthComma = response.indexOf(',', thirdComma + 1);

      if (firstComma == -1 || secondComma == -1 || thirdComma == -1) return;

      String addrStr = response.substring(5, firstComma);
      String lenStr = response.substring(firstComma + 1, secondComma);
      String hexData = response.substring(secondComma + 1, thirdComma);
      String rssi = response.substring(thirdComma + 1, fourthComma);
      String snr = response.substring(fourthComma + 1);

      // Chuyển hex sang ASCII
      // String asciiData = hexToAscii(hexData);

      Serial.println("-----------------------------------");
      Serial.print("Nhận từ địa chỉ: ");
      Serial.println(addrStr);
      Serial.print("Dữ liệu: ");
      Serial.println(hexData);
      Serial.print("RSSI: ");
      Serial.println(rssi);
      Serial.print("SNR: ");
      Serial.println(snr);
    }
  }
}

// Hàm chuyển đổi hex sang ASCII
String hexToAscii(const String &hex) {
  String ascii;
  for (int i = 0; i < hex.length(); i += 2) {
    String byteStr = hex.substring(i, i + 2);
    char c = (char) strtol(byteStr.c_str(), NULL, 16);
    ascii += c;
  }
  return ascii;
}
// #endif //USE_LORA
// #endif//LORA_
#endif//VERSION_2
