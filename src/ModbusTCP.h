
#ifdef ESP8266
 #include <ESP8266WiFi.h>
#else
 #include <WiFi.h>
#endif
#include <ModbusIP_ESP8266.h>


const uint16_t Coil_Y40 = 32;               // Modbus Coil Offset
const uint16_t CoilOffset = 0;               // Modbus Coil Offset
// IPAddress remote(192, 168, 3, 251);  // Address of Modbus Slave device

static uint16_t nCountReadConnected = 0; 

//Used Pins
#ifdef ESP8266
  #define 26 D4
 #else
  #define UES_LED TX
 #endif

ModbusIP ModBus;  //ModbusIP object
class Modbus_TCP_Prog
{
public:
uint16_t holdingRegisters[120];//0-30 PLC Data /30-34 total Plan / 34-74 product name 
boolean coils[50];

};
Modbus_TCP_Prog mb_c;

// uint16_t gc(TRegister* r, uint16_t v) { // Callback function
//   if (r->value != v) {  // Check if Coil state is going to be changed
//     Serial.print("Set reg: ");
//     Serial.println(v);
//     if (COIL_BOOL(v)) {
//       digitalWrite(Y8, LOW);
//       RunStop = 0;
//     } else {
//       digitalWrite(Y8, HIGH);
//       RunStop = 1;

//     }
//   }
//   return v;
// }
uint16_t HoldRegCall(TRegister* r, uint16_t v) { // Callback function
  if (r->value != v) {  // Check if Coil state is going to be changed
    Serial.print("Read holdReg reg: ");
    Serial.println(v);

  }
  return v;
}
uint16_t CoilCall(TRegister* r, uint16_t v) { // Callback function
  if (r->value != v) {  // Check if Coil state is going to be changed
    Serial.print("Read coil reg: ");
    Serial.println(v);

  }
  return v;
}
void TCP_setup(bool role) {
  if(role == 0){
  ModBus.client();                    // Initialize local Modbus Client
  ModBus.addCoil(Coil_Y40);           // Add Coil
  // ModBus.onSetCoil(Coil_Y40, gc);     // Assign Callback on set the Coil
  }
  if(role == 1){
    ModBus.server();
    // ModBus.addCoil(CoilOffset, 0, 50);
    // ModBus.addHreg(AddrOffset, 0, 50);
    ModBus.onGetHreg(AddrOffset, HoldRegCall, 50);   
    ModBus.onGetHreg(CoilOffset, CoilCall, 50);   
  }
}
byte TCPconnected = 0; 
byte TCPdisconnected = 0; 
void TCP_loop(bool role,int IPAddr1,int IPAddr2,int IPAddr3,int IPAddr4) {
  if(role == 0){
    IPAddress remote(IPAddr1, IPAddr2, IPAddr3, IPAddr4);
    // Serial.println(remote);
    if (ModBus.isConnected(remote)) {   // Check if connection to Modbus Slave is established
      if(TCPconnected++ > 3){Serial.println("TCP connected");TCPconnected = 3;TCPdisconnected = 0;}

      ModBus.readCoil(remote, Coil_Y40, &RunStop); delay(1);


      // bool coil_value = ModBus.pullCoil(remote, Coil_Y40, 41);  // Initiate Read Coil from Modbus Slave

      // RunStop = coil_value;
      // ModBus.readHreg(remote_Server_IP, D_Read, &D0_value_Read);   // Initiate Read Coil from Modbus Slave
    //  ModBus.readHreg(remote, CounterTotal, (uint16_t *)&CounterData,4); //Read holdingRegisters
        //-----Speed
      ModBus.readHreg(remote, Speed, (uint16_t *)&SpeedData,2); //Read holdingRegisters
  //ModBus.
    ModBus.readHreg(remote, CounterTotal, (uint16_t *)&CounterData,4);delay(1);
    ModBus.readHreg(remote, AddrOffset, (uint16_t *)&ModbusDataRead,50);delay(1);
    ModBus.readCoil(remote, CoilOffset, (bool*)&ModbusCoilRead, 50); delay(1);
  //
  if (nCountReadConnected++ > 3)
  {
    nCountReadConnected = 0;
      CounterData[0] = 0;CounterData[1] = 0;CounterData[2] = 0;CounterData[3] = 0;
      SpeedData[0] = 0;SpeedData[1] = 0;
      for(int i = 0; i < 130 ; i++){ModbusDataRead[i] = 0;}
      for(int i = 0; i < 130 ; i++){ModbusCoilRead[i] = 0;}
      ModBus.disconnect(remote); 
    }
    } else {
      if(TCPdisconnected++ > 5){TCPdisconnected = 5;TCPconnected = 0;Serial.println("TCP disconnected, retry...");}
      ModBus.connect(remote);           // Try to connect if no connection
    }
  }if(role == 1){       
  }
  ModBus.task();                      // Common local Modbus task
  delay(10);                     // Polling interval
}
