#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <FS.h>

int8_t BUZZ = -1;
int8_t SETUP_BUTTON = -1;
int8_t LED_STT = -1;
int8_t I2C_SDA = -1;
int8_t I2C_SCL = -1;
int8_t Y8 = -1;
int8_t Y9 = -1;
int8_t DA0 = -1;
int8_t DA1 = -1;
int8_t InPut0 = -1;
int8_t InPut1 = -1;
int8_t InPut2 = -1;
int8_t InPut3 = -1;
int8_t InPut4 = -1;
int8_t Ser_1RX = -1;
int8_t Ser_1TX = -1;
int8_t M0_PIN = -1;
int8_t M1_PIN = -1;
int8_t CS_PIN = -1;
int8_t RST_PIN = -1;
int8_t INIT_PIN = -1;
int8_t Ser_2RX = -1;
int8_t Ser_2TX = -1;
int8_t MOSI_PIN = -1;
int8_t MISO_PIN = -1;
int8_t SCK_PIN = -1; 
int8_t SD_CS_PIN = -1;
int8_t SD_MOSI_PIN = -1;
int8_t SD_MISO_PIN = -1;
int8_t SD_SCK_PIN = -1;

//BUZZ | SETUP_BUTTON | LED_STT | I2C_SDA | I2C_SCL | Y8 | Y9 | DA0 | DA1 | InPut0 | InPut1 | InPut2 | InPut3 | InPut4 | Ser_1RX | Ser_1TX | M0_PIN | M1_PIN | ETH_POWER_PIN_ALTERNATIVE(CS) | ETH_MDC_PIN(RST) | ETH_MDIO_PIN(INT) | Ser_2RX | Ser_2TX | MOSI_PIN | MISO_PIN | SCK_PIN
/*board RS485-Wifi*/
int8_t Board0[] = {/*BUZZ*/ -1, /*SETUP_BUTTON*/ 26, /*LED_STT*/ 25, /*I2C_SDA*/ 21, /*I2C_SCL*/ 22, /*Y8*/ -1, /*Y9*/ -1, /*DA0*/ -1, /*DA1*/ -1, /*InPut0*/ -1, /*InPut1*/ -1, /*InPut2*/ -1, /*InPut3*/ -1, /*InPut4*/ -1, /*Ser_1RX*/ 15, /*Ser_1TX*/  2, /*M0_PIN*/ -1, /*M1_PIN*/ -1, /*CS*/ -1, /*RST*/ -1, /*INIT*/ -1, /*Ser_2RX*/ 16, /*Ser_2TX*/ 17, /*MOSI_PIN*/ 23, /*MISO_PIN*/ 19, /*SCK_PIN*/ 18, /*SD_CS_PIN*/  5, /*SD_MOSI_PIN*/ -1, /*SD_MISO_PIN*/ -1, /*SD_SCK_PIN*/ -1};
/*board 410WER-Wifi*/
int8_t Board1[] = {/*BUZZ*/ 12, /*SETUP_BUTTON*/ 13, /*LED_STT*/  4, /*I2C_SDA*/ 32, /*I2C_SCL*/ 33, /*Y8*/  4, /*Y9*/  5, /*DA0*/ 39, /*DA1*/ 36, /*InPut0*/  0, /*InPut1*/ 34, /*InPut2*/ 35, /*InPut3*/ 15, /*InPut4*/ -1, /*Ser_1RX*/ 33, /*Ser_1TX*/ 32, /*M0_PIN*/ -1, /*M1_PIN*/ -1, /*CS*/ 14, /*RST*/ 23, /*INIT*/ 18, /*Ser_2RX*/ 16, /*Ser_2TX*/ 17, /*MOSI_PIN*/ -1, /*MISO_PIN*/ -1, /*SCK_PIN*/ -1, /*SD_CS_PIN*/ -1, /*SD_MOSI_PIN*/ -1, /*SD_MISO_PIN*/ -1, /*SD_SCK_PIN*/ -1};
/*board LkLineGW*/
int8_t Board2[] = {/*BUZZ*/ -1, /*SETUP_BUTTON*/  0, /*LED_STT*/ 27, /*I2C_SDA*/ 21, /*I2C_SCL*/ 22, /*Y8*/ -1, /*Y9*/ -1, /*DA0*/ -1, /*DA1*/ -1, /*InPut0*/ -1, /*InPut1*/ -1, /*InPut2*/ -1, /*InPut3*/ -1, /*InPut4*/ -1, /*Ser_1RX*/ -1, /*Ser_1TX*/ -1, /*M0_PIN*/ -1, /*M1_PIN*/ -1, /*CS*/ 14, /*RST*/ 23, /*INIT*/ 18, /*Ser_2RX*/ 16, /*Ser_2TX*/ 17, /*MOSI_PIN*/ -1, /*MISO_PIN*/ -1, /*SCK_PIN*/ -1, /*SD_CS_PIN*/ -1, /*SD_MOSI_PIN*/ -1, /*SD_MISO_PIN*/ -1, /*SD_SCK_PIN*/ -1};
/*board LkLineNode*/
int8_t Board3[] = {/*BUZZ*/ -1, /*SETUP_BUTTON*/ 26, /*LED_STT*/ 25, /*I2C_SDA*/ 21, /*I2C_SCL*/ 22, /*Y8*/ -1, /*Y9*/ -1, /*DA0*/ -1, /*DA1*/ -1, /*InPut0*/ -1, /*InPut1*/ -1, /*InPut2*/ -1, /*InPut3*/ -1, /*InPut4*/ -1, /*Ser_1RX*/ 15, /*Ser_1TX*/  2, /*M0_PIN*/ -1, /*M1_PIN*/ -1, /*CS*/ -1, /*RST*/ 16, /*INIT*/ 17, /*Ser_2RX*/ 16, /*Ser_2TX*/ 17, /*MOSI_PIN*/ -1, /*MISO_PIN*/ -1, /*SCK_PIN*/ -1, /*SD_CS_PIN*/ -1, /*SD_MOSI_PIN*/ -1, /*SD_MISO_PIN*/ -1, /*SD_SCK_PIN*/ -1};
/*board 0404WER*/
int8_t Board4[] = {/*BUZZ*/ -1, /*SETUP_BUTTON*/  0, /*LED_STT*/ 13, /*I2C_SDA*/ 21, /*I2C_SCL*/ 22, /*Y8*/ -1, /*Y9*/ -1, /*DA0*/ 36, /*DA1*/ 39, /*InPut0*/  0, /*InPut1*/ 33, /*InPut2*/ 32, /*InPut3*/ 34, /*InPut4*/ 35, /*Ser_1RX*/ 35, /*Ser_1TX*/  2, /*M0_PIN*/ -1, /*M1_PIN*/ 17, /*CS*/ 16, /*RST*/ 23, /*INIT*/ 18, /*Ser_2RX*/ 16, /*Ser_2TX*/ 17, /*MOSI_PIN*/ -1, /*MISO_PIN*/ -1, /*SCK_PIN*/ -1, /*SD_CS_PIN*/ -1, /*SD_MOSI_PIN*/ -1, /*SD_MISO_PIN*/ -1, /*SD_SCK_PIN*/ -1};
/*board S3SDCWER*/
int8_t Board5[] = {/*BUZZ*/ -1, /*SETUP_BUTTON*/  0, /*LED_STT*/ 13, /*I2C_SDA*/ 41, /*I2C_SCL*/ 42, /*Y8*/ -1, /*Y9*/ -1, /*DA0*/ -1, /*DA1*/ -1, /*InPut0*/ -1, /*InPut1*/ -1, /*InPut2*/ -1, /*InPut3*/ -1, /*InPut4*/ -1, /*Ser_1RX*/ 18, /*Ser_1TX*/ 17, /*M0_PIN*/ 16, /*M1_PIN*/ -1, /*CS*/ 20, /*RST*/  8, /*INIT*/ 19, /*Ser_2RX*/ -1, /*Ser_2TX*/ -1, /*MOSI_PIN*/ 35, /*MISO_PIN*/ 37, /*SCK_PIN*/ 36, /*SD_CS_PIN*/ 38, /*SD_MOSI_PIN*/ -1, /*SD_MISO_PIN*/ -1, /*SD_SCK_PIN*/ -1};
/*board S3SDCWE*/
int8_t Board6[] = {/*BUZZ*/ -1, /*SETUP_BUTTON*/  0, /*LED_STT*/ 21, /*I2C_SDA*/ 41, /*I2C_SCL*/ 42, /*Y8*/ -1, /*Y9*/ -1, /*DA0*/ -1, /*DA1*/ -1, /*InPut0*/ -1, /*InPut1*/ -1, /*InPut2*/ -1, /*InPut3*/ -1, /*InPut4*/ -1, /*Ser_1RX*/ 18, /*Ser_1TX*/ 17, /*M0_PIN*/ 16, /*M1_PIN*/ -1, /*CS*/ 14, /*RST*/  9, /*INIT*/ 10, /*Ser_2RX*/ -1, /*Ser_2TX*/ -1, /*MOSI_PIN*/ 11, /*MISO_PIN*/ 12, /*SCK_PIN*/ 13, /*SD_CS_PIN*/  4, /*SD_MOSI_PIN*/  6, /*SD_MISO_PIN*/  5, /*SD_SCK_PIN*/  7};
/*board Custom*/
int8_t BoardCustom[] = {/*BUZZ*/ -1, /*SETUP_BUTTON*/  0, /*LED_STT*/ 14, /*I2C_SDA*/ 41, /*I2C_SCL*/ 42, /*Y8*/ -1, /*Y9*/ -1, /*DA0*/ -1, /*DA1*/ -1, /*InPut0*/ -1, /*InPut1*/ -1, /*InPut2*/ -1, /*InPut3*/ -1, /*InPut4*/ -1, /*Ser_1RX*/ 18, /*Ser_1TX*/ 17, /*M0_PIN*/ 16, /*M1_PIN*/ -1, /*CS*/ 20, /*RST*/  8, /*INIT*/ 19, /*Ser_2RX*/ -1, /*Ser_2TX*/ -1, /*MOSI_PIN*/ 35, /*MISO_PIN*/ 37, /*SCK_PIN*/ 36, /*SD_CS_PIN*/ 21, /*SD_MOSI_PIN*/ -1, /*SD_MISO_PIN*/ -1, /*SD_SCK_PIN*/ -1};

const char* BoardCustomJson = R"json({
    "BUZZ": -1,
    "SETUP_BUTTON": 0,
    "LED_STT": 14,
    "I2C_SDA": 41,
    "I2C_SCL": 42,
    "Y8": -1,
    "Y9": -1,
    "DA0": -1,
    "DA1": -1,
    "InPut0": -1,
    "InPut1": -1,
    "InPut2": -1,
    "InPut3": -1,
    "InPut4": -1,
    "Ser_1RX": 18,
    "Ser_1TX": 17,
    "M0_PIN": 16,
    "M1_PIN": -1,
    "CS_PIN": 20,
    "RST_PIN": 8,
    "INIT_PIN": 19,
    "Ser_2RX": -1,
    "Ser_2TX": -1,
    "MOSI_PIN": 35,
    "MISO_PIN": 37,
    "SCK_PIN": 36,
    "SD_CS_PIN": 21,
    "SD_MOSI_PIN": -1,
    "SD_MISO_PIN": -1,
    "SD_SCK_PIN": -1
})json";



void loadPinMaping() {
    if (!LittleFS.begin(true)) {
        Serial.println("Failed to mount LittleFS");
        return;
    }
    File file = LittleFS.open("/PinMap.json", "r");
    if (!file) {
        // File does not exist, create with default content
        file = LittleFS.open("/PinMap.json", "w");
        if (file) {
            file.print(BoardCustomJson);
            file.close();
        }
        file = LittleFS.open("/PinMap.json", "r");
        if (!file) {
            Serial.println("Failed to create PinMap.json");
            return;
        }
    }

    // Allocate a buffer for JSON parsing
    size_t size = file.size();
    std::unique_ptr<char[]> buf(new char[size + 1]);
    file.readBytes(buf.get(), size);
    buf[size] = '\0';
    file.close();

    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, buf.get());
    if (error) {
        Serial.println("Failed to parse PinMap.json");
        return;
    }

    BoardCustom[0] = doc["BUZZ"] | -1;
    BoardCustom[1] = doc["SETUP_BUTTON"] | -1;
    BoardCustom[2] = doc["LED_STT"] | -1;
    BoardCustom[3] = doc["I2C_SDA"] | -1;
    BoardCustom[4] = doc["I2C_SCL"] | -1;
    BoardCustom[5] = doc["Y8"] | -1;
    BoardCustom[6] = doc["Y9"] | -1;
    BoardCustom[7] = doc["DA0"] | -1;
    BoardCustom[8] = doc["DA1"] | -1;
    BoardCustom[9] = doc["InPut0"] | -1;
    BoardCustom[10] = doc["InPut1"] | -1;
    BoardCustom[11] = doc["InPut2"] | -1;
    BoardCustom[12] = doc["InPut3"] | -1;
    BoardCustom[13] = doc["InPut4"] | -1;
    BoardCustom[14] = doc["Ser_1RX"] | -1;
    BoardCustom[15] = doc["Ser_1TX"] | -1;
    BoardCustom[16] = doc["M0_PIN"] | -1;
    BoardCustom[17] = doc["M1_PIN"] | -1;
    BoardCustom[18] = doc["CS_PIN"] | -1;
    BoardCustom[19] = doc["RST_PIN"] | -1;
    BoardCustom[20] = doc["INIT_PIN"] | -1;
    BoardCustom[21] = doc["Ser_2RX"] | -1;
    BoardCustom[22] = doc["Ser_2TX"] | -1;
    BoardCustom[23] = doc["MOSI_PIN"] | -1;
    BoardCustom[24] = doc["MISO_PIN"] | -1;
    BoardCustom[25] = doc["SCK_PIN"] | -1;
    BoardCustom[26] = doc["SD_CS_PIN"] | -1;
    BoardCustom[27] = doc["SD_MOSI_PIN"] | -1;
    BoardCustom[28] = doc["SD_MISO_PIN"] | -1;
    BoardCustom[29] = doc["SD_SCK_PIN"] | -1;
    doc.clear();
}

void savePinMaping() {
    if (!LittleFS.begin(true)) {
        Serial.println("Failed to mount LittleFS");
        return;
    }
    StaticJsonDocument<512> doc;
    doc["BUZZ"] = BoardCustom[0];
    doc["SETUP_BUTTON"] = BoardCustom[1];
    doc["LED_STT"] = BoardCustom[2];
    doc["I2C_SDA"] = BoardCustom[3];
    doc["I2C_SCL"] = BoardCustom[4];
    doc["Y8"] = BoardCustom[5];
    doc["Y9"] = BoardCustom[6];
    doc["DA0"] = BoardCustom[7];
    doc["DA1"] = BoardCustom[8];
    doc["InPut0"] = BoardCustom[9];
    doc["InPut1"] = BoardCustom[10];
    doc["InPut2"] = BoardCustom[11];
    doc["InPut3"] = BoardCustom[12];
    doc["InPut4"] = BoardCustom[13];
    doc["Ser_1RX"] = BoardCustom[14];
    doc["Ser_1TX"] = BoardCustom[15];
    doc["M0_PIN"] = BoardCustom[16];
    doc["M1_PIN"] = BoardCustom[17];
    doc["CS_PIN"] = BoardCustom[18];
    doc["RST_PIN"] = BoardCustom[19];
    doc["INIT_PIN"] = BoardCustom[20];
    doc["Ser_2RX"] = BoardCustom[21];
    doc["Ser_2TX"] = BoardCustom[22];
    doc["MOSI_PIN"] = BoardCustom[23];
    doc["MISO_PIN"] = BoardCustom[24];
    doc["SCK_PIN"] = BoardCustom[25];
    doc["SD_CS_PIN"] = BoardCustom[26];
    doc["SD_MOSI_PIN"] = BoardCustom[27];
    doc["SD_MISO_PIN"] = BoardCustom[28];
    doc["SD_SCK_PIN"] = BoardCustom[29];


    File file = LittleFS.open("/PinMap.json", "w");
    if (!file) {
        Serial.println("Failed to open PinMap.json for writing");
        return;
    }
    serializeJson(doc, file);
    file.close();
    doc.clear();
}

void set_Pinout(uint8_t BoardModel){
    int8_t* selectedBoard;
    switch (BoardModel) {
        case 0:
        selectedBoard = BoardCustom;
        break;
        case 1:
        selectedBoard = Board0;
        break;
        case 2:
        selectedBoard = Board1;
        break;
        case 3:
        selectedBoard = Board2;
        break;
        case 4:
        selectedBoard = Board3;
        break;
        case 5:
        selectedBoard = Board4;
        break;
        case 6:
        selectedBoard = Board5;
        break;
        case 7:
        selectedBoard = Board6;
        break;
        default:
        selectedBoard = BoardCustom; // Handle invalid BoardModel
        break;
    }
    BUZZ = selectedBoard[0];
    SETUP_BUTTON = selectedBoard[1];
    LED_STT = selectedBoard[2];
    I2C_SDA = selectedBoard[3];
    I2C_SCL = selectedBoard[4];
    Y8 = selectedBoard[5];
    Y9 = selectedBoard[6];
    DA0 = selectedBoard[7];
    DA1 = selectedBoard[8];
    InPut0 = selectedBoard[9];
    InPut1 = selectedBoard[10];
    InPut2 = selectedBoard[11];
    InPut3 = selectedBoard[12];
    InPut4 = selectedBoard[13];
    Ser_1RX = selectedBoard[14];
    Ser_1TX = selectedBoard[15];
    M0_PIN = selectedBoard[16];
    M1_PIN = selectedBoard[17];
    CS_PIN = selectedBoard[18];
    RST_PIN = selectedBoard[19];
    INIT_PIN = selectedBoard[20];
    Ser_2RX = selectedBoard[21];
    Ser_2TX = selectedBoard[22];
    MOSI_PIN = selectedBoard[23];
    MISO_PIN = selectedBoard[24];
    SCK_PIN = selectedBoard[25];
    SD_CS_PIN = selectedBoard[26];
    SD_MOSI_PIN = selectedBoard[27];
    SD_MISO_PIN = selectedBoard[28];
    SD_SCK_PIN = selectedBoard[29];
    
}