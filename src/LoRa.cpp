#include <ArduinoJson.h>
#include "main.h"
#include "LoRa.h"
#define VERSION_2

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
// #ifndef LORA_
// #define LORA_
// #include "config.h"
// // #include "WIC.h"
// #ifdef USE_LORA
// #include "LookLine.h"
// #ifdef USE_LORA
#define E32_TTL_1W

    #include "LoRa_E32.h"
// #endif//USE_LORA


  uint8_t M0_ = 0;
  uint8_t M1_ = 0;
  uint8_t TX_ = 0;
  uint8_t RX_ = 0;
  String Str_Lora_CH = "";
  String Air_Rate = "";
  String Baud_Rate = "";
  String Lora_PWR = "";
void LoRaFunction::SetPinLoRa(uint8_t M0Pin, uint8_t M1Pin, uint8_t TX, uint8_t RX)
{
	M0_ = M0Pin;
	M1_ = M1Pin;
	TX_ = TX;
	RX_ = RX;
	LOGLN("LORA Pin M0:" + String(M0_) + "| M1:" + String(M1_) + "| TX:" + String(TX_) + "| RX:" + String(RX_));
}
void WriteLoRaConfig(byte CH);
void WriteLoRaConfig(byte CH,byte AirRate );
void LoRaFunction::SetChanel(byte CH){
	WriteLoRaConfig(CH);
}
// LoRa_E32 e32ttl100(16, 17, &Serial2, UART_BPS_RATE_9600, SERIAL_8N1); // e32 TX e32 RX
LoRa_E32 e32ttl100(&Serial2, -1, M0_, M1_, UART_BPS_RATE_9600);

void printParameters(struct Configuration configuration);

void LoRaFunction::ReadLoRaConfig()
{
    
  digitalWrite(M0_, HIGH);
  digitalWrite(M1_, HIGH);
	// Startup all pins and UART
	e32ttl100.begin();

	ResponseStructContainer c;
	c = e32ttl100.getConfiguration();
	// It's important get configuration pointer before all other operation
	Configuration configuration = *(Configuration*) c.data;
	LOG(c.status.getResponseDescription() + "| code: " + c.status.code + "| ");

	printParameters(configuration);

	c.close();
    digitalWrite(M0_, LOW);
    digitalWrite(M1_, LOW);
}
void LoRaFunction::WriteLoRaConfig(byte CH,byte AirRate )
{
    digitalWrite(M0_, HIGH);
    digitalWrite(M1_, HIGH);
	// Startup all pins and UART
	e32ttl100.begin();

	ResponseStructContainer c;
	c = e32ttl100.getConfiguration();
    Configuration configuration = *(Configuration*) c.data;
      ///*
	configuration.ADDL = 0x0;
	configuration.ADDH = 0x0;
	configuration.CHAN = CH;

	configuration.SPED.uartParity = MODE_00_8N1;
	configuration.SPED.uartBaudRate = UART_BPS_9600;
	configuration.SPED.airDataRate = AirRate;
	configuration.OPTION.fixedTransmission = FT_TRANSPARENT_TRANSMISSION;
	configuration.OPTION.ioDriveMode = IO_D_MODE_PUSH_PULLS_PULL_UPS;
	configuration.OPTION.wirelessWakeupTime = WAKE_UP_250;
	configuration.OPTION.fec = FEC_0_OFF;
	configuration.OPTION.transmissionPower = POWER_30;


	// Set configuration changed and set to not hold the configuration
	ResponseStatus rs = e32ttl100.setConfiguration(configuration, WRITE_CFG_PWR_DWN_LOSE);
	//LOGLN(rs.getResponseDescription());
	//LOGLN(rs.code);
	printParameters(configuration);
  //*/
    digitalWrite(M0_, LOW);
    digitalWrite(M1_, LOW);
}
void LoRaFunction::WriteLoRaConfig(byte CH)
{
    digitalWrite(M0_, HIGH);
    digitalWrite(M1_, HIGH);
	// Startup all pins and UART
	e32ttl100.begin();

	ResponseStructContainer c;
	c = e32ttl100.getConfiguration();
    Configuration configuration = *(Configuration*) c.data;
      ///*
	configuration.ADDL = 0x0;
	configuration.ADDH = 0x0;
	configuration.CHAN = CH;

	configuration.ADDL = 0x0;
	configuration.ADDH = 0x1;
	configuration.CHAN = 0x19;

	configuration.OPTION.fec = FEC_0_OFF;
	configuration.OPTION.fixedTransmission = FT_TRANSPARENT_TRANSMISSION;
	configuration.OPTION.ioDriveMode = IO_D_MODE_PUSH_PULLS_PULL_UPS;
	configuration.OPTION.transmissionPower = POWER_30;
	configuration.OPTION.wirelessWakeupTime = WAKE_UP_1250;

	configuration.SPED.airDataRate = AIR_DATA_RATE_001_12;
	configuration.SPED.uartBaudRate = UART_BPS_9600;
	configuration.SPED.uartParity = MODE_00_8N1;

	


	// Set configuration changed and set to not hold the configuration
	ResponseStatus rs = e32ttl100.setConfiguration(configuration, WRITE_CFG_PWR_DWN_LOSE);
	//LOGLN(rs.getResponseDescription());
	//LOGLN(rs.code);
	printParameters(configuration);
  //*/
    digitalWrite(M0_, LOW);
    digitalWrite(M1_, LOW);
}
void LoRaFunction::printParameters(struct Configuration configuration) {
	// if(GatewayTerminal){
  	// LOGLN("----------------------------------------");

	// LOG(F("HEAD : "));  LOG(configuration.HEAD, BIN);LOG(" ");LOG(configuration.HEAD, DEC);LOG(" ");LOGLN(configuration.HEAD, HEX);
	// LOGLN(F(" "));
	// LOG(F("AddH : "));  LOGLN(configuration.ADDH, BIN);
	// LOG(F("AddL : "));  LOGLN(configuration.ADDL, BIN);
	// LOG(F("Chan : "));  LOG(configuration.CHAN, DEC); LOG(" -> "); LOGLN(configuration.getChannelDescription());
	// LOGLN(F(" "));
	// LOG(F("SpeedParityBit     : "));  LOG(configuration.SPED.uartParity, BIN);LOG(" -> "); LOGLN(configuration.SPED.getUARTParityDescription());
	// LOG(F("SpeedUARTDatte  : "));  LOG(configuration.SPED.uartBaudRate, BIN);LOG(" -> "); LOGLN(configuration.SPED.getUARTBaudRate());
	// LOG(F("SpeedAirDataRate   : "));  LOG(configuration.SPED.airDataRate, BIN);LOG(" -> "); LOGLN(configuration.SPED.getAirDataRate());

	// LOG(F("OptionTrans        : "));  LOG(configuration.OPTION.fixedTransmission, BIN);LOG(" -> "); LOGLN(configuration.OPTION.getFixedTransmissionDescription());
	// LOG(F("OptionPullup       : "));  LOG(configuration.OPTION.ioDriveMode, BIN);LOG(" -> "); LOGLN(configuration.OPTION.getIODroveModeDescription());
	// LOG(F("OptionWakeup       : "));  LOG(configuration.OPTION.wirelessWakeupTime, BIN);LOG(" -> "); LOGLN(configuration.OPTION.getWirelessWakeUPTimeDescription());
	// LOG(F("OptionFEC          : "));  LOG(configuration.OPTION.fec, BIN);LOG(" -> "); LOGLN(configuration.OPTION.getFECDescription());
	// LOG(F("OptionPower        : "));  LOG(configuration.OPTION.transmissionPower, BIN);LOG(" -> "); LOGLN(configuration.OPTION.getTransmissionPowerDescription());

	// LOGLN("----------------------------------------");
//   }
  Str_Lora_CH = configuration.getChannelDescription();
  Air_Rate = configuration.SPED.getAirDataRate();
  Baud_Rate = configuration.SPED.getUARTBaudRate();
  Lora_PWR = configuration.OPTION.getTransmissionPowerDescription();

  LOG("LoRa chanel:" + Str_Lora_CH);
  LOG("| LoRa Air rate:" + Air_Rate);
  LOG("| LoRa baudrate:" + Baud_Rate);
  LOGLN("| LoRa Power:" + Lora_PWR);
}

HardwareSerial loraSerial(2); // Serial2 cho LoRa
#ifdef Module_RS485
#define LoRaTx  2
#define LoRaRx  15
#endif//Module_RS485
#ifdef Module_10O4I
#define LoRaTx  32
#define LoRaRx  33
#endif//Module_10O4I
// Khai báo hàm
void sendCommand(const String &cmd);
void sendData(String address, String data);
String hexToAscii(const String &hex);
void receiveData();

void LoRaFunction::LoRaSetup(int LoRaID) {
  loraSerial.begin(9600, SERIAL_8N1, LoRaRx , LoRaTx); // RX, TX
  delay(1000);

  // Cấu hình module
  sendCommand("AT"); // Kiểm tra kết nối
  sendCommand("AT+BAND=868500000,M"); // Đặt tần số 868MHz
  sendCommand("AT+NETWORKID=18");    // Đặt Network ID
  sendCommand("AT+ADDRESS="+String(LoRaID));    // Địa chỉ của thiết bị này
  // sendCommand("AT+KEY=AAAAAAAAAAAAAAAA"); // Khóa mã hóa 16 ký tự
}

void LoRaFunction::LoRaLoop(int SlaveID) {
  // Gửi dữ liệu mỗi 5 giây
  static unsigned long lastSend = 0;
  if (millis() - lastSend >= 3000) {
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
  delay(100);
  while (loraSerial.available()) {
    String response = loraSerial.readStringUntil('\n');
    Serial.print("Response: ");
    Serial.println(response);
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
}
bool LRonce = false;
// Hàm đọc và xử lý dữ liệu nhận được
void LoRaFunction::receiveData() {
  if(!LRonce) {
    LRonce = true;
    // delay(1000);
  loraSerial.begin(9600, SERIAL_8N1, LoRaRx , LoRaTx); // RX, TX
  // delay(1000);
  sendCommand("AT"); // Kiểm tra kết nối
  }
  while (loraSerial.available()) {
    String response = loraSerial.readStringUntil('\n');
    Serial.print("Response: ");
    Serial.println(response);
  // }
    LOGLN(response);
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