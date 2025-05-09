#ifndef MODBUS_
#define MODBUS_
// #include <config.h>

#include "Arduino_JSON.h"
// #include <ArduinoJson.h>

#include "DFRobot_RTU.h"
#include <ModbusRTU.h>
// #include <ModbusRtu.h>

#ifdef MASTER_MODBUS
#endif//MASTER_MODBUS
///////////////////////// Modbus Role //////////////////////////
enum {MBmaster, MBslave};
///////////////////////////////////////////////////////////////
#define Y0 0
#define Y1 1
#define Y2 2
#define Y3 3
#define Y4 4
#define Y5 5
#define Y6 6
#define Y7 7
#define Y10 8
#define Y11 9
#define Y12 10
#define Y13 11
#define Y14 12
#define Y15 13
#define Y16 14
#define Y17 15
#define Y20 16
#define Y21 17
#define Y22 18
#define Y23 19
#define Y24 20
#define Y25 21
#define Y26 22
#define Y27 23
#define Y30 24
#define Y31 25
#define Y32 26
#define Y33 27
#define Y34 28
#define Y35 29
#define Y36 30
#define Y37 31
#define Y40 32
#define Y41 33
#define Y42 34
#define Y43 35
#define Y44 36
#define Y45 37
#define Y46 38
#define Y47 39




// Khai báo extern để sử dụng các biến toàn cục
extern boolean coils[50];
extern uint16_t holdingRegisters[200];
extern uint16_t inputRegisters[200];
class Modbus_Prog
{
public:
bool isConnect;
bool MB_connect = false;
// ModbusRTU mb;
const uint16_t RegRead = 0; //Register Read
//////////////// registers of your slave ///////////////////
//////////////////// Port information ///////////////////
// #define baudrate 9600
#define timeouts 300
#define polling 10 // the scan rate
#define retry_count 500 

// used to toggle the receive/transmit pin on the driver
#define TxEnablePin -1 



// boolean coils[50];
// uint8_t discreteInputs[30];
//////////////////////// các thanh ghi Data cho 4 máy
// uint16_t holdingRegisters[120];//0-30 PLC Data /30-34 total Plan / 34-74 product name 
// uint16_t inputRegisters[120];// data tu web gui ve public 0-30 PLC Data /30-34 total Plan / 34-74 product name 
uint16_t ReadRegTemporary[10];//thanh ghi Read tạm thời
// uint16_t WriteRegTemporary[10];//thanh ghi Write tạm thời

uint16_t AddrOffset = 0;//counterIn total 

uint16_t ModbusDataRead[130];//counterIn total 
bool ModbusCoilRead[130];//counterIn total 
// The data from the PLC will be stored
// in the regs array

// unsigned int regs_Data[HOLDING_REGS_SIZE*2];
uint16_t* getInputRegs();
uint16_t* getOutputRegs();
void update();
void initupdate();
void modbusSet(uint16_t addr, uint16_t value);
void modbusWriteBuffer(uint16_t addrOffset, uint16_t *value);
void connectModbus(bool update);
bool getStart();
//////////////////////////////
void MonitorData();
uint32_t DWORD(uint16_t u1, uint16_t u2);
uint16_t GetHoldingReg(uint16_t addr);
void SetCoilReg(uint16_t addr,boolean value);
void SetHoldingReg(uint16_t addr,uint16_t value);

bool GetCoilReg(uint16_t addr);
void modbus_setup(String ModbusParameter, int8_t RXpin, int8_t TXpin) ;
void modbus_loop(int Timeout);
//////////////////////////////
void debugs();
void Write_PLC();
// void ModbusGetData(uint8_t *mbInputRegisters, size_t length);
void setModbusupdateState(bool state);
bool getModbusupdateState();
uint16_t getModbusupdateData();
uint16_t getModbusupdateAddr();
// void modbus_write_update(int16_t HOLDING_REGS_Data[]);
// void modbus_read_update(int16_t HOLDING_REGS_Data[]) ;            
void modbus_write_setParameter(int pos,int Value, int cmd);
void Clear_Lookline_Value();
private:



};

#endif//MODBUS_