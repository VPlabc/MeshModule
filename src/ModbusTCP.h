#ifndef MODBUS_TCP_
#define MODBUS_TCP_
#include "Arduino_JSON.h"
#include "Modbus.h"
// Khai báo extern để sử dụng các biến toàn cục
extern boolean modbusTcp_coils[50];
extern uint16_t modbusTcp_holdingRegisters[200];
extern uint16_t modbusTcp_inputRegisters[200];

#define modbusTcp_Slave 0
#define modbusTcp_Master 1

class ModbusTcp_Prog
{
  public:
    bool isConnect;
    bool MB_connect = false;

    uint16_t GetHoldingReg(uint16_t addr);
    bool WriteHoldingReg(uint16_t addr, uint16_t value);
    bool WriteNameToHoldregister(uint16_t addr, char *name);
    bool WriteHoldingReg(uint16_t addr, uint16_t value, uint16_t len);
    String getNameFromHoldregister(uint16_t addr);
    void modbus_loop(int Timeout);
    void modbus_setup(String ModbusParameter, uint8_t master_slave, String IP, uint16_t Port, uint8_t unitId);

    void modbus_set_print_debug(bool value);

  private:

    //uint16_t cbRead(TRegister* reg, uint16_t val);
    //uint16_t cbWrite(TRegister* reg, uint16_t val);
    //uint16_t ModbusTcp_HoldregGet (TRegister* reg, uint16_t val0);
};
#endif