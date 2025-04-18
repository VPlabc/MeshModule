#include "HardwareSerial.h"

#include <ModbusRTU.h>


#define D0 0       //Holding register Address 0 
#define D1 1       //Holding register Address 1 


#define M0 0       //coil status Address 0
#define M1 1       //coil status Address 1 



#define SLAVE_ID 1    //Slave Address  1

ModbusRTU mb;

int D_0;
int D_1;

int M_0;
int M_1;

#include "0_HoldingReg.h"
#include "0_CoilReg.h"


#define RXD2 16
#define TXD2 17




void TaskModbus(void *pvParameter)
{
    Serial.print("TaskModbus Run in core ");
    Serial.println(xPortGetCoreID());
    for (;;)
    {
        mb.task();
        // vTaskDelay(10); // Nhường quyền điều khiển trong 1 tick
        delay(200);
    }
}

void setup() {

  Serial.begin(115200);

  Serial2.begin(9600, SERIAL_8N1);

#if defined(ESP32) || defined(ESP8266)
  mb.begin(&Serial2);
#else
  // mb.begin(&Serial);
  mb.begin(&Serial2, RXD2, TXD2); //or use RX/TX direction control pin (if required)
  mb.setBaudrate(9600);
#endif
  mb.slave(SLAVE_ID);



  // Holding register
  mb.addHreg(D0);
  mb.onSetHreg(D0, HoldregSet);
  mb.onGetHreg(D0, D0HoldregGet);

  mb.addHreg(D1);
  mb.onGetHreg(D1, D1HoldregGet);
  // mb.Hreg(D0, 100);//กำหนดให้ D0 มีค่า =100



  // Coil reister
  mb.addCoil(M0);
  mb.addCoil(M1);
 

  mb.onSetCoil(M0, M0CoilGet);
  mb.onSetCoil(M1, M1CoilGet);
  xTaskCreatePinnedToCore(TaskModbus, "TaskModbusRTU", 5000, NULL, 2, NULL, 0);
delay(500);
}




void loop() {

  }
