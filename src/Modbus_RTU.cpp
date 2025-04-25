// #include <Arduino.h>
// #include "config.h"
#include "Modbus_RTU.h"
// #include "./HardwareSerial.h"
#define RTU_RS485
#define EN_DEBUG

Modbus_Prog ModbuS;
// #define PLC_OEE
#if defined(EN_DEBUG)
#define EN_DEBUG
#define debug Serial
#define DB(x) debug.print(x);
#define DBf(x) debug.printf(x);
#define DB_LN(x) debug.println(x);
#define DB_BG(x) debug.begin(x);
#define DB_FL() debug.flush();
#else
#define DB_BG(...)
#define DB(...)
#define DB_LN(...)
#endif

// #define ESP32_C3

#if defined(ESP32_C3)
#define Modbus_Serial Serial1
#elif defined(ESP32)
#define ModbusSerial Serial2
#else
#include <SoftwareSerial.h>
SoftwareSerial serial_ESP1(5, 4);
#define Modbus_Serial serial_ESP1
#endif //ESP32
#include "HardwareSerial.h"
ModbusRTU mb; 

TaskHandle_t Task1;

#define D0 0      //Holding register Address 0  Hunidity
#define D1 1      //Holding register Address 1  Temperature
#define D2 2      
#define D3 3       
boolean coils[50] = {false};
uint16_t holdingRegisters[200] = {0};
uint16_t inputRegisters[200] = {0};


//Task1 :
void Task1code( void * pvParameters ) {
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {
    int randomNumber1 = random(0, 800);
    //Serial.print("randomNumber1  "); Serial.println(randomNumber1);
    mb.Hreg (D2, randomNumber1);
    int randomNumber2 = random(-400, 600);
    //Serial.print("randomNumber2  "); Serial.println(randomNumber2);
    mb.Hreg(D3, randomNumber2 );
    delay(5000);
  }//End loop
}


// #define DEBUG
const long interval_ = 3000;
unsigned long previousMillis_ = 0;
bool Done = false;

byte MBRole = MBslave;

void debugs();
#define dataSize 10
#if defined(PLC_OEE) || defined(RFData)
// const int16_t offsetM0 = 2048;//Delta PLC
// const int16_t offsetD0 = 4096;//Delta PLC
// const int16_t Write_Coil = offsetM0+0;
// const int16_t Read_Coil = offsetM0+100;
// const int16_t RegWrite = offsetD0+2500;//Register Write
const int16_t offsetD0 = 4096; //
// const int16_t RegRead = offsetD0+2000;//Register Read
const int16_t RegRead = 140; //Register Read

#endif //PLC_OEE
#ifdef VOM
const int16_t offsetD0 = 4096; //
// const int16_t RegRead = offsetD0+2000;//Register Read
const int16_t RegRead = 0; //Register Read
#else
// const int16_t RegRead = 0;//Register Read
#endif //VOM

bool state0 = 0;
// bool state1 = 1;
// bool state2 = 0;
int count_mb = 0;
const long intervalupdate = 10000;
unsigned long previousMillisupdate = 0;
byte once = true;
const long interval_update = 3;
unsigned long previousMillis_update = 0;
#define baudrate 9600 //19200
// #define Enthernet
// #define WifiConnect
// #define Modbus_Serial
// #define Master
#define SerialPort
#define TEST

#define ledPin 2 // onboard led 

void Modbus_Prog::modbus_write_setParameter(int pos, int Value, int cmd) {
  // if(pos == HOLDING_REGS_SIZE+1 && Value == 0 && cmd == 0){ModbuS.regs_WRITE[CMD] = 2;}
  // else{ ModbuS.regs_WRITE[pos] = Value;ModbuS.regs_WRITE[cmd] = cmd;debugs();}

}

#define SLAVE_ID 1

/// @brief /// Register address
// bool ModbusRole = Master;

void debugs();
uint16_t HoldregGet (TRegister* reg, uint16_t val0);
uint16_t CoilGet(TRegister* reg, uint16_t val0);
// HardwareSerial1 MySerial1_(1);
// #define Serial2 MySerial1_

String connId[4] = {"connId1", "connId2", "connId3", "connId4"};
String Tag[40] = {"tag1", "tag2", "tag3", "tag4"};
JSONVar iMagSetting;

extern String connId[4];
extern String Tag[40];
extern int AddrOffset;

void Modbus_Prog::modbus_setup(String ModbusParameter, int8_t RXpin, int8_t TXpin) {
  #ifdef RTU_RS485
      iMagSetting = JSON.parse(ModbusParameter);
      if (JSON.typeof(iMagSetting) == "undefined") {
        Serial.println("Parsing input failed!");
    }else{
        DB_LN("role: " + String((const char*)iMagSetting["role"]));
        DB_LN("Com: " + String((const char*)iMagSetting["Com"]));
        DB_LN("id: " + String((int)iMagSetting["id"]));
        DB_LN("slave ip: " + String((int)iMagSetting["slaveip"][0]) + "." + String((int)iMagSetting["slaveip"][1]) + "." + String((int)iMagSetting["slaveip"][2]) + "." + String((int)iMagSetting["slaveip"][3]));
        DB_LN("AddrOffset: " + String((int)iMagSetting["AddrOffset"]));
        AddrOffset = (int)iMagSetting["offset"];
        if((const char*)iMagSetting["conId"] != NULL){
            DB_LN((const char*)iMagSetting["conId"]);
            connId[0] = (const char*)iMagSetting["conId"];
        }
        for(byte i = 0 ; i < iMagSetting["Tag"].length(); i++){
        DB_LN("Tag" + String(i) + ": " + (int)iMagSetting["Tag"][i]);
        Tag[i] = String((int)iMagSetting["Tag"][i]);
        }
        for(byte i = 0 ; i < iMagSetting["Value"].length(); i++){
        DB_LN("Value" + String(i) + ": " + (int)iMagSetting["Value"][i]);
        }
        for(byte i = 0 ; i < iMagSetting["Type"].length(); i++){
        DB_LN("Type" + String(i) + ": " + (int)iMagSetting["Type"][i]);
        }
    
    }
    
      if(String((const char*)iMagSetting["role"]) == "master"){MBRole = MBmaster;} else {MBRole = MBslave;}
          Serial.println("Modbus :" + String((const char*)iMagSetting["role"]));

      DB_LN("_________________________________________ MODBUS RTU ________________________________________");
      
      //   initupdate();
        if (MBRole == MBmaster) {
          DB_LN("Modbus Master Init");
          Serial2.begin(9600, SERIAL_8N1, RXpin ,TXpin); // RX, TX
          Modbus_Master.setTimeoutTimeMs(100);
          Modbus_Master.begin(&Serial2);
        }
        if (MBRole == MBslave) {
          DB_LN("Modbus Slave Init");
          Serial2.begin(9600, SERIAL_8N1);
          
          #if defined(ESP32) || defined(ESP8266)
            mb.begin(&Serial2);
            mb.server((int)iMagSetting["id"]);
          #else
            // mb.begin(&Serial);
            mb.begin(&Serial2, RXD2, TXD2); //or use RX/TX direction control pin (if required)
            //mb.setBaudrate(9600);
            mb.setBaudrate(BUAD_RATE);     //กำหนด buadrate จาก ตัวแปล BUAD_RATE
          #endif
          //------------------------------------------------------------------------
          //Task1 :
          xTaskCreatePinnedToCore(
            Task1code,   /* Task function. */
            "Task1",     /* name of task. */
            10000,       /* Stack size of task */
            NULL,        /* parameter of the task */
            1,           /* priority of the task*/
            &Task1,      /* Task handle to keep track of created task */
            0);          /* pin task to core x */
          delay(500);
          //------------------------------------------------------------------------

          for (int i = 0; i < (int)iMagSetting["Value"].length(); i++) {
            if((int)iMagSetting["Type"][i] == 0){
              mb.addCoil((int)iMagSetting["Value"][i]);
              mb.onSetCoil((int)iMagSetting["Value"][i], CoilGet);
            }
            if((int)iMagSetting["Type"][i] == 1){
              mb.addHreg((int)iMagSetting["Value"][i]);
              mb.onGetHreg((int)iMagSetting["Value"][i], HoldregGet);
            }
            if((int)iMagSetting["Type"][i] == 2){
              mb.addHreg((int)iMagSetting["Value"][i]);
              mb.addHreg((int)iMagSetting["Value"][i]+1);
              mb.onGetHreg((int)iMagSetting["Value"][i], HoldregGet);
              mb.onGetHreg((int)iMagSetting["Value"][i]+1, HoldregGet);
            }
            if((int)iMagSetting["Type"][i] == 3){
              mb.addHreg((int)iMagSetting["Value"][i]);
              mb.addHreg((int)iMagSetting["Value"][i]+1);
              mb.onGetHreg((int)iMagSetting["Value"][i], HoldregGet);
              mb.onGetHreg((int)iMagSetting["Value"][i]+1, HoldregGet);
            }
          }


        }
        DB_LN("________________________________________________________________________________________");
#endif//RTU_RS485
}

bool MB_Update_Once = true;
bool MB_Update_Once1 = true;
bool Reg_Update_Once = true;
bool Reg_Update_Once1 = true;

uint16_t addrPLC = 0;
uint16_t valuePLC = 0;

byte bien = 0;

uint32_t Modbus_Prog::DWORD(uint16_t u1, uint16_t u2)
{  
  uint32_t num = ((uint32_t)u1 & 0xFFFF) << 16 | ((uint32_t)u2 & 0xFFFF);
    return num;
}
uint16_t Modbus_Prog::GetHoldingReg(uint16_t addr) {
  return holdingRegisters[addr];
}

boolean Modbus_Prog::GetCoilReg(uint16_t addr) {
  return coils[addr];
}
uint16_t HoldregGet (TRegister* reg, uint16_t val0) {
  holdingRegisters[reg->address.address] = val0;
  // Serial.println("HregGet "+ String(reg->address.address) + " val:" + String(holdingRegisters[reg->address.address]));
  return val0;
}

uint16_t CoilGet(TRegister* reg, uint16_t val0) {
  // Serial.println("CoilGet "+ String(reg->address.address) + " val:" + String(val0));
  if(val0 > 0){
    coils[reg->address.address] = true;
  }else{
    coils[reg->address.address] = false;
  }
  return val0;
}
void Modbus_Prog::MonitorData(){
  DB_LN("Registers Value:");
    for (int i = 0; i < iMagSetting["Value"].length(); i++) {
      if((int)iMagSetting["Type"][i] == 0){
        DB(String((int)iMagSetting["Value"][i]) + " [" + String(coils[(int)iMagSetting["Value"][i]]) + "] | ");
      }
      if((int)iMagSetting["Type"][i] == 1){
        DB(String((int)iMagSetting["Value"][i]) + " [" + String(holdingRegisters[(int)iMagSetting["Value"][i]]) + "] | ");
      }
      if((int)iMagSetting["Type"][i] == 2){
        uint32_t dwordValue = DWORD(holdingRegisters[(int)iMagSetting["Value"][i]], holdingRegisters[(int)iMagSetting["Value"][i]+1]);
        DB(String((int)iMagSetting["Value"][i]) + " [" + String(dwordValue) + "] | ");
      }
      if((int)iMagSetting["Type"][i] == 3){
        DB(String((int)iMagSetting["Value"][i]) + " [" + String((float)DWORD(holdingRegisters[(int)iMagSetting["Value"][i]], holdingRegisters[(int)iMagSetting["Value"][i]+1])) + "] | ");
      }
    }
    DB_LN("");
}
void Modbus_Prog::modbus_loop(int Timeout) {
  
  if (millis() - previousMillis_ >= Timeout) {
    previousMillis_ = millis();
    if (Done == false) {
      Done = true;
      DB_LN("Modbus Loop");
    }
    if (MBRole == MBmaster) {
      for (int i = 0; i < (int)iMagSetting["Value"].length(); i++) {
        
        if((int)iMagSetting["Type"][i] == 0){
          coils[i] = Modbus_Master.readCoilsRegister((int)iMagSetting["id"], (int)iMagSetting["Value"][i]); //Read holdingRegisters
        }
        if((int)iMagSetting["Type"][i] == 1){
          holdingRegisters[i*2] = Modbus_Master.readHoldingRegister((int)iMagSetting["id"], (int)iMagSetting["Value"][i]); //Read holdingRegisters
        }
        if((int)iMagSetting["Type"][i] == 2){
          holdingRegisters[i*2] = Modbus_Master.readHoldingRegister((int)iMagSetting["id"], (int)iMagSetting["Value"][i]); //Read holdingRegisters
          holdingRegisters[(i*2)+1] = Modbus_Master.readHoldingRegister((int)iMagSetting["id"], (int)iMagSetting["Value"][i]+1); //Read holdingRegisters
        }
        if((int)iMagSetting["Type"][i] == 3){
          holdingRegisters[i*2] = Modbus_Master.readHoldingRegister((int)iMagSetting["id"], (int)iMagSetting["Value"][i]); //Read holdingRegisters
          holdingRegisters[(i*2)+1] = Modbus_Master.readHoldingRegister((int)iMagSetting["id"], (int)iMagSetting["Value"][i]+1); //Read holdingRegisters
        }
      }
      // MonitorData();
    }


    // MonitorData();
    
  }
  // #ifdef MASTER_MODBUS
 
    if (MBRole == MBslave) {
      mb.task();
      yield();delay(10);
    }
    yield();
}//loop