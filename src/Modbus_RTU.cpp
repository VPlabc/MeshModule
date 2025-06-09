// #include <Arduino.h>
// #include "config.h"
#include "Modbus_RTU.h"
DFRobot_RTU Modbus_Master;

// #include "WebInterface.h"
// WebinterFace modbusWebInterface;
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

#define ESP32_C3

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
#define D2 170      
#define D3 172       
boolean coils[50] = {false};
boolean inputCoils[50] = {false};
uint8_t Coils[50] = {0};
uint16_t holdingRegisters[200] = {0};
uint16_t inputRegisters[2000] = {0};
uint16_t BufferRegisters[200] = {0};





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

void debugs();
uint16_t HoldregGet (TRegister* reg, uint16_t val0);
uint16_t CoilGet(TRegister* reg, uint16_t val0);

// HardwareSerial1 MySerial1_(1);
// #define Serial2 MySerial1_

// String connId[4] = {"connId1", "connId2", "connId3", "connId4"};
// String Tag[40] = {"tag1", "tag2", "tag3", "tag4"};
JSONVar MobusSettings;
JSONVar DataBlocks;

// extern String connId[4];
// extern String Tag[40];
extern int AddrOffset;

//Task1 :
void Task1code( void * pvParameters ) {
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {
    // if (MBRole == MBmaster) {
      
    //   for (int i = 0; i < (int)MobusSettings["Value"].length(); i++) {
    //     if((int)MobusSettings["Type"][i] == 0){
    //       Modbus_Master.writeCoilsRegister((int)MobusSettings["id"], (uint16_t)MobusSettings["Value"][i],inputCoils[(uint16_t)MobusSettings["Value"][i]]); //Read holdingRegisters
    //     }
    //     if((int)MobusSettings["Type"][i] == 1){
    //       Modbus_Master.writeHoldingRegister((int)MobusSettings["id"], (uint16_t)MobusSettings["Value"][i],inputRegisters[(uint16_t)MobusSettings["Value"][i]]); //write inputRegisters`
    //     }
    //     if((int)MobusSettings["Type"][i] == 2){
    //       Modbus_Master.writeHoldingRegister((int)MobusSettings["id"], (uint16_t)MobusSettings["Value"][i],inputRegisters[(uint16_t)MobusSettings["Value"][i]]); //write inputRegisters
    //       Modbus_Master.writeHoldingRegister((int)MobusSettings["id"], (uint16_t)MobusSettings["Value"][i]+1,inputRegisters[((uint16_t)MobusSettings["Value"][i])+1]); //write inputRegisters
    //     }
    //     if((int)MobusSettings["Type"][i] == 3){
    //       Modbus_Master.writeHoldingRegister((int)MobusSettings["id"], (uint16_t)MobusSettings["Value"][i], inputRegisters[(uint16_t)MobusSettings["Value"][i]]); //write inputRegisters
    //       Modbus_Master.writeHoldingRegister((int)MobusSettings["id"], (uint16_t)MobusSettings["Value"][i]+1, inputRegisters[((uint16_t)MobusSettings["Value"][i])+1]); //write inputRegisters
    //     }
    //   }
    // }
    if (MBRole == MBslave) {
    int randomNumber1 = random(0, 800);
    //Serial.print("randomNumber1  "); Serial.println(randomNumber1);
    mb.Hreg (D2, randomNumber1);
    int randomNumber2 = random(0, 65000);
    //Serial.print("randomNumber2  "); Serial.println(randomNumber2);
    mb.Hreg(D3, randomNumber2 );
    }
    delay(5000);
  }//End loop
}

JSONVar Modbus_Prog::getDataBlocks() {
  return DataBlocks;
}

void Modbus_Prog::modbus_write_setParameter(int pos, int Value, int cmd) {
  // if(pos == HOLDING_REGS_SIZE+1 && Value == 0 && cmd == 0){ModbuS.regs_WRITE[CMD] = 2;}
  // else{ ModbuS.regs_WRITE[pos] = Value;ModbuS.regs_WRITE[cmd] = cmd;debugs();}

}

/*{
  "block": [
    {
      "tagFrom": 0,
      "valueFrom": 6096,
      "typeFrom": 1,
      "amount": 120
    }
  ]
}
  */
void Modbus_Prog::modbus_setup(String ModbusParameter, String DataBckParam,  int8_t RXpin, int8_t TXpin) {
  #ifdef RTU_RS485
      MobusSettings = JSON.parse(ModbusParameter);
      DataBlocks = JSON.parse(DataBckParam);
      Serial.println("DataBckParam: " + DataBckParam);
      if (JSON.typeof(MobusSettings) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }else{
        DB_LN("role: " + String((const char*)MobusSettings["role"]));
        DB_LN("Com: " + String((const char*)MobusSettings["Com"]));
        DB_LN("id: " + String((int)MobusSettings["id"]));
        DB_LN("slave ip: " + String((int)MobusSettings["slaveip"][0]) + "." + String((int)MobusSettings["slaveip"][1]) + "." + String((int)MobusSettings["slaveip"][2]) + "." + String((int)MobusSettings["slaveip"][3]));
        DB_LN("AddrOffset: " + String((int)MobusSettings["AddrOffset"]));
        AddrOffset = (int)MobusSettings["offset"];
        // if((const char*)MobusSettings["conId"] != NULL){
        //     DB_LN((const char*)MobusSettings["conId"]);
        //     connId[0] = (const char*)MobusSettings["conId"];
        // }
        // for(byte i = 0 ; i < MobusSettings["Tag"].length(); i++){
        // DB_LN("Tag" + String(i) + ": " + (int)MobusSettings["Tag"][i]);
        // Tag[i] = String((int)MobusSettings["Tag"][i]);
        // }
        // for(byte i = 0 ; i < MobusSettings["Value"].length(); i++){
        // DB_LN("Value" + String(i) + ": " + (int)MobusSettings["Value"][i]);
        // }
        // for(byte i = 0 ; i < MobusSettings["Type"].length(); i++){
        // DB_LN("Type" + String(i) + ": " + (int)MobusSettings["Type"][i]);
        // }
        for(byte i = 0 ; i < MobusSettings["Type"].length(); i++){
        DB_LN("Reg Offset: " + String((int)MobusSettings["Offset"][i]));
        }
        
        for(byte i = 0 ; i < (int)DataBlocks["block"].length(); i++){
        DB_LN("DataBlock Offset: " + String((int)DataBlocks["block"][i]["offset"]));
        }
      }
    
      if(String((const char*)MobusSettings["role"]) == "master"){MBRole = MBmaster;} else {MBRole = MBslave;}
            Serial.println("Modbus :" + String((const char*)MobusSettings["role"]));
      if(String((const char*)MobusSettings["Com"]) == "RS485"){
      DB_LN("_________________________________________ MODBUS RTU ________________________________________");
      
      //   initupdate();
        if (MBRole == MBmaster) {
          DB_LN("Modbus Master Init");
          Modbus_Serial.begin(9600, SERIAL_8N1, RXpin ,TXpin); // RX, TX
          Modbus_Master.setTimeoutTimeMs(100);
          Modbus_Master.begin(&Modbus_Serial);
        }
        if (MBRole == MBslave) {
          DB_LN("Modbus Slave Init");
          Modbus_Serial.begin(9600, SERIAL_8N1);
          
          #if defined(ESP32) || defined(ESP8266)
            mb.begin(&Modbus_Serial);
            mb.server((int)MobusSettings["id"]);
          #else
            // mb.begin(&Modbus_Serial);
            mb.begin(&Modbus_Serial, RXD2, TXD2); //or use RX/TX direction control pin (if required)
            //mb.setBaudrate(9600);
            mb.setBaudrate(BUAD_RATE);     //กำหนด buadrate จาก ตัวแปล BUAD_RATE
          #endif
          //------------------------------------------------------------------------
          //Task1 :
          // xTaskCreatePinnedToCore(
          //   Task1code,   /* Task function. */
          //   "Task1",     /* name of task. */
          //   10000,       /* Stack size of task */
          //   NULL,        /* parameter of the task */
          //   1,           /* priority of the task*/
          //   &Task1,      /* Task handle to keep track of created task */
          //   0);          /* pin task to core x */
          // delay(500);
          //------------------------------------------------------------------------

          for (int i = 0; i < (int)MobusSettings["Value"].length(); i++) {
            if((int)MobusSettings["Type"][i] == 0){
              mb.addCoil((int)MobusSettings["Value"][i] + (int)MobusSettings["Offset"][i]);
              mb.onSetCoil((int)MobusSettings["Value"][i] + (int)MobusSettings["Offset"][i], CoilGet);
            }
            if((int)MobusSettings["Type"][i] == 1){
              mb.addHreg((int)MobusSettings["Value"][i] + (int)MobusSettings["Offset"][i]);
              mb.onGetHreg((int)MobusSettings["Value"][i] + (int)MobusSettings["Offset"][i], HoldregGet);
            }
            if((int)MobusSettings["Type"][i] == 2){
              mb.addHreg((int)MobusSettings["Value"][i] + (int)MobusSettings["Offset"][i]);
              mb.addHreg((int)MobusSettings["Value"][i] + (int)MobusSettings["Offset"][i]+1);
              mb.onGetHreg((int)MobusSettings["Value"][i] + (int)MobusSettings["Offset"][i], HoldregGet);
              mb.onGetHreg((int)MobusSettings["Value"][i] + (int)MobusSettings["Offset"][i]+1, HoldregGet);
            }
            if((int)MobusSettings["Type"][i] == 3){
              mb.addHreg((int)MobusSettings["Value"][i] + (int)MobusSettings["Offset"][i]);
              mb.addHreg((int)MobusSettings["Value"][i] + (int)MobusSettings["Offset"][i]+1);
              mb.onGetHreg((int)MobusSettings["Value"][i] + (int)MobusSettings["Offset"][i], HoldregGet);
              mb.onGetHreg((int)MobusSettings["Value"][i] + (int)MobusSettings["Offset"][i]+1, HoldregGet);
            }
          }
          if (JSON.typeof(DataBlocks) != "undefined" && DataBlocks.hasOwnProperty("block")) {
            for (int b = 0; b < (int)DataBlocks["block"].length(); b++) {
              int AddressOffset = (int)DataBlocks["block"][b]["offset"];
              int tagFrom = (int)DataBlocks["block"][b]["tagFrom"];
              int valueFrom = (int)DataBlocks["block"][b]["valueFrom"];
              int typeFrom = (int)DataBlocks["block"][b]["typeFrom"];
              int amount = (int)DataBlocks["block"][b]["amount"];
              for (int j = 0; j < amount; j++) {
                int tag = tagFrom + j;
                int value = valueFrom + j;
                if (typeFrom == 0) {
                  mb.addCoil(value + AddressOffset);
                  mb.onSetCoil(value + AddressOffset, CoilGet);
                } else if (typeFrom == 1) {
                  mb.addHreg(value + AddressOffset);
                  mb.onGetHreg(value + AddressOffset, HoldregGet);
                } else if (typeFrom == 2 || typeFrom == 3) {
                  mb.addHreg(value + AddressOffset);
                  mb.addHreg(value + AddressOffset + 1);
                  mb.onGetHreg(value + AddressOffset, HoldregGet);
                  mb.onGetHreg(value + AddressOffset + 1, HoldregGet);
                }
              }
            }
          }
        }
        DB_LN("________________________________________________________________________________________");
      }
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
uint16_t Modbus_Prog::GetInputHoldingReg(uint16_t addr) {
  return inputRegisters[addr];
}

boolean Modbus_Prog::GetCoilReg(uint16_t addr) {
  return coils[addr];
}
void Modbus_Prog::SetCoilReg(int ID, uint16_t addr,boolean value) {
  Modbus_Master.writeCoilsRegister(ID, addr, value);
}
void Modbus_Prog::SetHoldingReg(int ID, uint16_t addr,uint16_t value) {
  Modbus_Master.writeHoldingRegister(ID , addr, value);
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

String Modbus_Prog::GetJson(){
    String json = "{\"masterData\":[";
    // --- Thêm log giá trị đọc từ DataBlocks ---
    if (JSON.typeof(DataBlocks) != "undefined" && DataBlocks.hasOwnProperty("block")) {
      for (int b = 0; b < (int)DataBlocks["block"].length(); b++) {
        int amount = (int)DataBlocks["block"][b]["amount"];
        for (int j = 0; j < amount; j++) {
          if (j > 0 || b > 0) json += ",";
          int addr = (int)DataBlocks["block"][b]["valueFrom"] + j;
          int type = (int)DataBlocks["block"][b]["typeFrom"];
          json += "{\"";
          json += String(addr + (int)DataBlocks["block"][b]["offset"]);
          json += "\":";
          if (type == 0) {
            json += coils[addr] ? 1 : 0;
            json += ",\"type\":\"COIL\",\"rawValue\":[" + String(inputRegisters[addr]) + "]}";
          } else if (type == 1) {
            json += String(inputRegisters[addr]);
            json += ",\"type\":\"WORD\",\"rawValue\":[" + String(inputRegisters[addr]) + "]}";
          } else if (type == 2) {
            uint32_t dwordValue = DWORD(
              inputRegisters[addr],
              inputRegisters[addr + 1]
            );
            json += String(dwordValue);
            json += ",\"type\":\"DWORD\",\"rawValue\":[" + String(inputRegisters[addr]) + "," + String(inputRegisters[addr + 1]) + "]}";
          } else if (type == 3) {
            float floatValue = (float)DWORD(
              inputRegisters[addr],
              inputRegisters[addr + 1]
            );
            json += String(floatValue);
            json += ",\"type\":\"FLOAT\",\"rawValue\":[" + String(inputRegisters[addr]) + "," + String(inputRegisters[addr + 1]) + "]}";
          }
        }
      }
      if(MobusSettings.hasOwnProperty("Value") && MobusSettings.hasOwnProperty("Type")){
        for (int i = 0; i < MobusSettings["Value"].length(); i++) {
          if(MobusSettings["Value"].length() > 0){
            json += ",";
            int addr = (int)MobusSettings["Value"][i];
            int type = (int)MobusSettings["Type"][i];
            json += "{\"" + String(addr + (int)MobusSettings["Offset"][i]) + "\":";
            if (type == 0) {
              json += coils[addr] ? 1 : 0;
              json += ",\"type\":\"COIL\",\"rawValue\":[" + String(holdingRegisters[addr]) + "]}";
            } else if (type == 1) {
              json += String(holdingRegisters[addr]);
              json += ",\"type\":\"WORD\",\"rawValue\":[" + String(holdingRegisters[addr]) + "]}";
            } else if (type == 2) {
              uint32_t dwordValue = DWORD(
                holdingRegisters[addr],
                holdingRegisters[addr + 1]
              );
              json += String(dwordValue);
              json += ",\"type\":\"DWORD\",\"rawValue\":[" + String(holdingRegisters[addr]) + "," + String(holdingRegisters[addr + 1]) + "]}";
            } else if (type == 3) {
              float floatValue = (float)DWORD(
                holdingRegisters[addr],
                holdingRegisters[addr + 1]
              );
              json += String(floatValue);
              json += ",\"type\":\"FLOAT\",\"rawValue\":[" + String(holdingRegisters[addr]) + "," + String(holdingRegisters[addr + 1]) + "]}";
            }
          }
        }
      }
    }
    json += "]}";
    return json;
  }

void Modbus_Prog::MonitorData(){
    DB_LN("--------------------------------------------------------------------------------");
  DB_LN("Registers Value:");
    for (int i = 0; i < MobusSettings["Value"].length(); i++) {
      if((int)MobusSettings["Type"][i] == 0){
        DB(String((int)MobusSettings["Value"][i]) + " [" + String(coils[(int)MobusSettings["Value"][i]] ) + "] | ");
      }
      if((int)MobusSettings["Type"][i] == 1){
        DB(String((int)MobusSettings["Value"][i]) + " [" + String(holdingRegisters[(int)MobusSettings["Value"][i]]) + "] | ");
      }
      if((int)MobusSettings["Type"][i] == 2){
        uint32_t dwordValue = DWORD(holdingRegisters[(int)MobusSettings["Value"][i]], holdingRegisters[(int)MobusSettings["Value"][i]+1]);
        DB(String((int)MobusSettings["Value"][i]) + " [" + String(dwordValue) + "] | ");
      }
      if((int)MobusSettings["Type"][i] == 3){
        DB(String((int)MobusSettings["Value"][i]) + " [" + String((float)DWORD(holdingRegisters[(int)MobusSettings["Value"][i]], holdingRegisters[(int)MobusSettings["Value"][i]+1])) + "] | ");
      }
    }
    DB_LN("");
  
    // --- Thêm log giá trị đọc từ DataBlocks ---
    if (JSON.typeof(DataBlocks) != "undefined" && DataBlocks.hasOwnProperty("block")) {
        DB_LN("DataBlock Values:");
        for (int b = 0; b < (int)DataBlocks["block"].length(); b++) {
            int offset = (MobusSettings.hasOwnProperty("offset")) ? (int)MobusSettings["offset"] : 0;
            int tagFrom = (int)DataBlocks["block"][b]["tagFrom"];
            int valueFrom = (int)DataBlocks["block"][b]["valueFrom"];
            int typeFrom = (int)DataBlocks["block"][b]["typeFrom"];
            int amount = (int)DataBlocks["block"][b]["amount"];
            String logStr;
            for (int j = 0; j < amount; j++) {
                int tag = tagFrom + j;
                int value = valueFrom + j;
                logStr += String(value + offset) + " [";
                if (typeFrom == 0) {
                    logStr += String(Coils[value]);
                } else if (typeFrom == 1) {
                    logStr += String(inputRegisters[value]);
                } else if (typeFrom == 2) {
                    uint32_t dwordValue = DWORD(inputRegisters[value], inputRegisters[value + 1]);
                    logStr += String(dwordValue);
                } else if (typeFrom == 3) {
                    float floatValue = (float)DWORD(inputRegisters[value], inputRegisters[value + 1]);
                    logStr += String(floatValue);
                };
                logStr += "] | ";
            }
                DB_LN(logStr);logStr = "";
        }
    }
    DB_LN("--------------------------------------------------------------------------------");
}
void Modbus_Prog::modbus_loop(int Timeout) {
  if(String((const char*)MobusSettings["Com"]) == "RS485"){
    if (millis() - previousMillis_ >= Timeout) {
      previousMillis_ = millis();
      if (Done == false) {
        Done = true;
        DB_LN("Modbus Loop");
      }
      if (MBRole == MBmaster) {
        for (int i = 0; i < (int)MobusSettings["Value"].length(); i++) {
          
          // Serial.println("Modbus register read: " + String((int)MobusSettings["id"]) + " | " + String((int)MobusSettings["Value"][i]) + " | " + String((int)MobusSettings["Type"][i]) + " | " + String((int)MobusSettings["Offset"][i]));
          if((int)MobusSettings["Type"][i] == 0){
            coils[(int)MobusSettings["Value"][i]] = Modbus_Master.readCoilsRegister((int)MobusSettings["id"], (int)MobusSettings["Value"][i] + (int)MobusSettings["Offset"][i]); //Read holdingRegisters
          }
          if((int)MobusSettings["Type"][i] == 1){
            holdingRegisters[(int)MobusSettings["Value"][i]] = Modbus_Master.readHoldingRegister((int)MobusSettings["id"], (int)MobusSettings["Value"][i] + (int)MobusSettings["Offset"][i]); //Read holdingRegisters
          }
          if((int)MobusSettings["Type"][i] == 2){
            holdingRegisters[(int)MobusSettings["Value"][i]] = Modbus_Master.readHoldingRegister((int)MobusSettings["id"], (int)MobusSettings["Value"][i] + (int)MobusSettings["Offset"][i]); //Read holdingRegisters
            holdingRegisters[((int)MobusSettings["Value"][i])+1] = Modbus_Master.readHoldingRegister((int)MobusSettings["id"], (int)MobusSettings["Value"][i] + (int)MobusSettings["Offset"][i] + 1); //Read holdingRegisters
          }
          if((int)MobusSettings["Type"][i] == 3){
            holdingRegisters[(int)MobusSettings["Value"][i]] = Modbus_Master.readHoldingRegister((int)MobusSettings["id"], (int)MobusSettings["Value"][i] + (int)MobusSettings["Offset"][i]); //Read holdingRegisters
            holdingRegisters[((int)MobusSettings["Value"][i])+1] = Modbus_Master.readHoldingRegister((int)MobusSettings["id"], (int)MobusSettings["Value"][i] + (int)MobusSettings["Offset"][i] + 1); //Read holdingRegisters
          }
        }
        // Đọc Modbus master theo cấu hình DataBlocks
        if (JSON.typeof(DataBlocks) != "undefined" && DataBlocks.hasOwnProperty("block")) {
          for (int b = 0; b < (int)DataBlocks["block"].length(); b++) {
            int offset = (int)DataBlocks["block"][b]["offset"];
            int tagFrom = (int)DataBlocks["block"][b]["tagFrom"];
            int valueFrom = (int)DataBlocks["block"][b]["valueFrom"];
            int typeFrom = (int)DataBlocks["block"][b]["typeFrom"];
            int amount = (int)DataBlocks["block"][b]["amount"];
            // Serial.print("Reading DataBlock: ");
            // Serial.print("valueFrom: " + String(valueFrom) + ", typeFrom: " + String(typeFrom) + ", amount: " + String(amount) + ", offset: " + String(offset));
            // Serial.println();
              if (typeFrom == 0) {
                Modbus_Master.readCoilsRegister((int)MobusSettings["id"], valueFrom + offset,valueFrom + offset + amount, Coils, amount); // Read coils
              } else if (typeFrom == 1) {
                Modbus_Master.readHoldingRegister((int)MobusSettings["id"], valueFrom + offset, BufferRegisters, amount);
              } else if (typeFrom == 2) {
                Modbus_Master.readHoldingRegister((int)MobusSettings["id"], valueFrom + offset, BufferRegisters, amount*2);
              } else if (typeFrom == 3) {
                Modbus_Master.readHoldingRegister((int)MobusSettings["id"], valueFrom + offset, BufferRegisters, amount*2);
              }
                // Ghi dữ liệu của mỗi block vào inputRegisters dựa theo block và amount
                if (typeFrom == 0) {
                // Coils: copy từ Coils[] sang inputRegisters[]
                  for (int j = 0; j < amount; j++) {
                    int addr = valueFrom + j;
                    inputRegisters[addr] = Coils[j];
                  }
                } else if (typeFrom == 1) {
                // WORD: copy từ BufferRegisters[] sang inputRegisters[]
                  for (int j = 0; j < amount; j++) {
                    int addr = valueFrom + j;
                    inputRegisters[addr] = BufferRegisters[j];
                  }
                } else if (typeFrom == 2 || typeFrom == 3) {
                // DWORD/FLOAT: copy từng cặp từ BufferRegisters[] sang inputRegisters[]
                  for (int j = 0; j < amount * 2; j++) {
                    int addr = valueFrom + j;
                    inputRegisters[addr] = BufferRegisters[j];
                  }
                }
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
  }
}//loop