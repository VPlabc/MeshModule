#ifndef LMDMain_h
#define LMDMain_h

#include <Arduino.h>
#include <ArduinoJson.h>

// ==== Pin mapping (giữ nguyên như bạn đang dùng) ====
#define R1_PIN 3
#define G1_PIN 10
#define B1_PIN 11
#define R2_PIN 12
#define G2_PIN 13
#define B2_PIN 14

#define CLK_PIN 42
#define LAT_PIN 41
#define OE_PIN 47

#define A_PIN 36
#define B_PIN 35
#define C_PIN 45
#define D_PIN 46
#define E_PIN -1

#define FORMAT_LITTLEFS_IF_FAILED true

// const char* ssid = "I-Soft";
// const char* password = "i-soft@2023";
// // const char* ssid = "DAT PHUONG";
// // const char* password = "19201974";

// ==== Panel config ====
#define PANEL_RES_X 64//104//64      // Số pixel ngang của panel
#define PANEL_RES_Y 32//52//32      // Số pixel dọc của panel

// Tối đa 10 nhóm, mỗi nhóm tối đa 10 dòng (bạn có thể tăng nếu cần)
#define MAX_GROUPS 10
#define MAX_LINES_PER_GROUP 10

#define MAX_GROUPS 10
#define MAX_LINES_PER_GROUP 10
#define MAX_TEXT_LENGTH 10


// Định nghĩa struct cho từng dòng chữ
struct TextLine {
    int x;
    int y;
    int id;
    uint16_t color; 
};

class LMDmain {
public:
    // void drawShapeFromType(DynamicJsonDocument dataIn);
    // void showAllTextLines(TextLine textCoorID[MAX_GROUPS][MAX_LINES_PER_GROUP], char textContents[MAX_GROUPS][MAX_LINES_PER_GROUP][MAX_TEXT_LENGTH]);
    void StringProcess(String jsonStr);

};


#endif // LMDMain_h