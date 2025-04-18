#include <Arduino.h>
#include "Modbus_RTU.h"
Modbus_Prog TskModbus;
#define EN_DEBUG
#if defined(EN_DEBUG)
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
/* ------------------------------------------------------------------------- */
            // data array for modbus network sharing
            uint16_t au16data[16];
            uint8_t u8state;

#define RXD2 16
#define TXD2 17

void TskRS485code( void * pvParameters ) {
  DB("Task RS485 running on core ");
  DB_LN(xPortGetCoreID());
  bool RS485Once = true;
  TskModbus.modbus_setup(1);
        static long LastRS485 = millis();
        static long TimeRS485 = 1000;
    for (;;) {
        TskModbus.modbus_loop(1);
        if(millis() - LastRS485 > TimeRS485){LastRS485 = millis();
         
        }
    }
  }