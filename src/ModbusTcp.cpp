#include "ModbusTcp.h"
#include <ModbusIP_ESP8266.h>

/***********************************************************
 * ********* MACRO ******************
 *************************************************************/
#define DISABLE  0
#define ENABLE  1

#define HOLDINGS_LEN 40
#define COILS_LEN 10
#define PRINT_DEBUG ENABLE

/***********************************************************
 * ********* PRIVATE VARIABLES ******************
 *************************************************************/
//Enter your plc-memory here
#define plc_modbusTcp_address_length  9
                                                              //0 speed   1 Counter   2    3 Status   4 LossCode  5 Loss State  6 staff name   7 staff Code  8 product
uint16_t plc_modbusTcp_address[plc_modbusTcp_address_length] = {116,      132,       204,  242,       2000,       2001,         2002,         2024,         2026};//2000 loss code | 2001 sate loss| 2002 name (2024 - 22 ký tự) | 2024 mã nhân viên () | 2026 Product Code (2048 - 22 ký tự) 




boolean modbusTcp_coils[50]= {false};
uint16_t modbusTcpString[20] = {0}; // Array to hold string data
uint16_t modbusTcpName[20] = {0}; // Array to hold string data
uint16_t modbusTcpProduct[20] = {0}; // Array to hold string data


uint16_t modbusTcp_holdingRegisters[200]= {0};
uint16_t modbusTcp_inputRegisters[200] = {0};

IPAddress remote(192, 168, 3, 9);  // Address of Modbus Slave device
ModbusIP mbTcp;  //ModbusIP object

const long modbusTcp_interval_ = 3000;
unsigned long modbusTcp_previousMillis_ = 0;
const int REG = 0;               // Modbus Hreg Offset
uint16_t res = 0;

uint16_t modbusTcp_Port = 502;
uint8_t modbusTcp_UnitId = 1;

uint8_t modbusTcp_MasterSlave = modbusTcp_Slave;
JSONVar jsonSettings;
bool is_enable_printf_debug = false;
uint8_t n_count_delay = 0;
/***********************************************************
 * ********* PRIVATE FUNCTIONS DECLARATION ******************
 *************************************************************/
uint16_t ModbusTcp_HoldregGet (TRegister* reg, uint16_t val0);
uint16_t ModbusTcp_CoilregGet(TRegister* reg, uint16_t val0);
bool cbConn(IPAddress ip);


/***********************************************************
 * ********* PRIVATE FUNCTIONS IMPLEMENTATION ***************
 *************************************************************/
uint16_t ModbusTcp_HoldregGet (TRegister* reg, uint16_t val0) {
  modbusTcp_holdingRegisters[reg->address.address] = val0;

  if (is_enable_printf_debug == true)
  {
    Serial.println("HregGet "+ String(reg->address.address) + " val:" + String(modbusTcp_holdingRegisters[reg->address.address]));
  }
  return val0;
}




// Callback function for client connect. Returns true to allow connection.
uint16_t ModbusTcp_CoilregGet(TRegister* reg, uint16_t val0) {
  if (is_enable_printf_debug == true)  
    Serial.println("CoilGet "+ String(reg->address.address) + " val:" + String(val0));

  if(val0 > 0){
    modbusTcp_coils[reg->address.address] = true;
  }else{
    modbusTcp_coils[reg->address.address] = false;
  }
  return val0;
}

// Callback function for client connect. Returns true to allow connection.
bool cbConn(IPAddress ip) {
  return true;
}

/***********************************************************
 * ********* GLOBAL FUNCTIONS IMPLEMENTATION ***************
 *************************************************************/

void ModbusTcp_Prog::modbus_set_print_debug(bool value){
  is_enable_printf_debug = value;
}
void ModbusTcp_Prog::modbus_setup(String ModbusParameter, uint8_t master_slave, String IP, uint16_t Port, uint8_t unitId) {
  uint8_t i = 0;

  n_count_delay = 0;
  modbusTcp_MasterSlave = master_slave;
  modbusTcp_Port = Port;
  modbusTcp_UnitId = unitId;
  remote.fromString(IP);
  //
  jsonSettings = JSON.parse(ModbusParameter);

  if (JSON.typeof(jsonSettings) == "undefined"){
    Serial.println("[jsonSettings] Parsing input failed!");
  }
  else{
    Serial.println("[jsonSettings] Parsing input ok!");
  }


  if (modbusTcp_MasterSlave == modbusTcp_Slave){
    mbTcp.client();    
  }
  else if (modbusTcp_MasterSlave == modbusTcp_Master){
    mbTcp.onConnect(cbConn);   // Add callback on connection event
    mbTcp.server();
    //------------------    
    for(i = 0; i < HOLDINGS_LEN; i++)
    {
      mbTcp.addHreg(i);
      mbTcp.onGetHreg(i, ModbusTcp_HoldregGet); // Add callback on Coils value get
    }
    //------- COIL
    for(i = 0; i < COILS_LEN; i++)
    {
      mbTcp.addCoil(i);
      mbTcp.onGetCoil(i, ModbusTcp_CoilregGet); // Add callback on Coils value get
    }
   
  }
}
void ModbusTcp_Prog::modbus_loop(int Timeout) {
  {
    if (millis() - modbusTcp_previousMillis_ >= Timeout) {
      modbusTcp_previousMillis_ = millis();     

      // if (JSON.typeof(jsonSettings) == "undefined"){
      //     Serial.println("[modbus_loop-jsonSettings] Parsing input failed!");
      //   }
      //   else{
      //     Serial.println("[modbus_loop-jsonSettings] Parsing input ok!");
      //   }
      // Serial.println("TCP is connected: " + String(mbTcp.isConnected(remote)));
      // Serial.println("TCP remote: " + remote.toString());


      if (modbusTcp_MasterSlave == modbusTcp_Slave){
        if (mbTcp.isConnected(remote)) {   // Check if connection to Modbus Slave is established

          for(uint8_t i = 0; i < plc_modbusTcp_address_length; i++)
          { 
            mbTcp.readHreg(remote, plc_modbusTcp_address[i], &modbusTcp_holdingRegisters[i], 1, nullptr, 1);  // Initiate Read Coil from Modbus Slave  
            if (is_enable_printf_debug)
              Serial.println("[" + String(plc_modbusTcp_address[i]) +"]: " + String(modbusTcp_holdingRegisters[i]) );
          }
          
          //delay 6 time after reset, anh then we read data
          // if (n_count_delay>30)
          // {            
          //   for(uint8_t i = 0; i < plc_modbusTcp_address_length; i++)
          //   { 
          //     mbTcp.readHreg(remote, plc_modbusTcp_address[i], &modbusTcp_holdingRegisters[i], 1, nullptr, 1);  // Initiate Read Coil from Modbus Slave  
          //     if (is_enable_printf_debug)
          //       Serial.println("[" + String(plc_modbusTcp_address[i]) +"]: " + String(modbusTcp_holdingRegisters[i]) );
          //   }
          // }
          // else
          // {
          //   n_count_delay += 1;
          //   for(uint8_t i = 0; i < plc_modbusTcp_address_length; i++)
          //   { 
          //     mbTcp.readHreg(remote, plc_modbusTcp_address[i], &res, 1, nullptr, 1);  // Initiate Read Coil from Modbus Slave 
          //   } 
          // }
      
        } 
        else {
            mbTcp.connect(remote, modbusTcp_Port);           // Try to connect if no connection            
        }
      }
      else if (modbusTcp_MasterSlave == modbusTcp_Master)
      {        
        //mbTcp.addHreg(REG, 0x0);
      }
      // MonitorData();
    }
    // #ifdef MASTER_MODBUS
      {
        mbTcp.task();
        yield();delay(10);
      }
      yield();
  }
}

uint16_t ModbusTcp_Prog::GetHoldingReg(uint16_t addr) {
  return modbusTcp_holdingRegisters[addr];
}


bool ModbusTcp_Prog::WriteHoldingReg(uint16_t addr, uint16_t value) {
  if(addr < sizeof(plc_modbusTcp_address)/sizeof(plc_modbusTcp_address[0])) {
    modbusTcp_holdingRegisters[addr] = value;
    uint16_t ModbusAddr = plc_modbusTcp_address[addr % plc_modbusTcp_address_length]; // Ensure we are within the defined PLC address range
    if (is_enable_printf_debug){
      Serial.println("Write Holding Register: " + String(ModbusAddr) + " Value: " + String(value));
    }
    mbTcp.writeHreg(remote, ModbusAddr, value, nullptr, modbusTcp_UnitId);  // Write Holding Register to Modbus Slave
    return true;
  }
  return false;
}

bool ModbusTcp_Prog::WriteHoldingReg(uint16_t addr, uint16_t value, uint16_t len) {
  uint16_t DataArray[len] = {0}; // Array to hold string data
  if(addr < sizeof(plc_modbusTcp_address)/sizeof(plc_modbusTcp_address[0])) {
    for(int i = 0 ; i < len ; i++){ DataArray[i] = value;}
    uint16_t ModbusAddr = plc_modbusTcp_address[addr % plc_modbusTcp_address_length]; // Ensure we are within the defined PLC address range
      Serial.println("Write Holding Register: " + String(ModbusAddr) + " Value: " + String(value) + " Length: " + String(len));

    if (is_enable_printf_debug){
      Serial.println("Write Holding Register: " + String(ModbusAddr) + " Value: " + String(value) + " Length: " + String(len));
    }
    vTaskDelay(200 / portTICK_PERIOD_MS);
    mbTcp.writeHreg(remote, ModbusAddr, DataArray, len, nullptr, modbusTcp_UnitId);  // Write Holding Register to Modbus Slave
    vTaskDelay(200 / portTICK_PERIOD_MS);
    return true;
  }
  return false;
}

bool ModbusTcp_Prog::WriteNameToHoldregister(uint16_t addr, char *name) {
  if(addr < sizeof(plc_modbusTcp_address)/sizeof(plc_modbusTcp_address[0])) {
  uint16_t ModbusAddr = plc_modbusTcp_address[addr % plc_modbusTcp_address_length]; // Ensure we are within the defined PLC address range
    if (is_enable_printf_debug)
      Serial.println("Write Name to Holding Register: " + String(ModbusAddr) + " Name: " + String(name));
    char buffer[40] = {' '}; // Initialize buffer with zeroes
    for(int i = 0; i < sizeof(modbusTcpString); i++) {
      modbusTcpString[i] = 0; // Clear the modbusTcpString array
      // mbTcp.writeHreg(remote, ModbusAddr, modbusTcpString, 20,  nullptr, modbusTcp_UnitId);  // Write Holding Register to Modbus Slave
    }
    snprintf(buffer, sizeof(buffer), "%s", name);
    for(int i = 0; i < sizeof(buffer); i++) {
      if (buffer[i] == '\0') {
        for(int j = i; j < sizeof(buffer); j++) {
          buffer[j] = 0; // Fill the rest with zeroes
        }
        break;} // Stop at null terminator
      uint16_t Dchar = buffer[i*2+1] << 8; // Shift character to high byte
      Dchar |= buffer[i*2]; // Combine with next character if available
      modbusTcpString[i] = Dchar;
    }
    vTaskDelay(200 / portTICK_PERIOD_MS);
    mbTcp.writeHreg(remote, ModbusAddr, modbusTcpString, 20,  nullptr, modbusTcp_UnitId);  // Write Holding Register to Modbus Slave
    vTaskDelay(200 / portTICK_PERIOD_MS);

    return true;
  }
  return false;
}

String ModbusTcp_Prog::getNameFromHoldregister(uint16_t addr) {
  char name[40] = {0}; // Initialize name with zeroes

  // Clear modbusTcpString array before reading new data
  // Do NOT clear modbusTcpString here, otherwise the value read will be empty.
  // The array should be cleared only after reading and converting the value.
  // for(int i = 0; i < sizeof(modbusTcpString)/sizeof(modbusTcpString[0]); i++) {
  //   modbusTcpString[i] = 0;
  // }
  // Clear name array
  for(int i = 0; i < sizeof(name); i++) {
    name[i] = 0;
  }

  if(addr < sizeof(plc_modbusTcp_address)/sizeof(plc_modbusTcp_address[0])) {
    uint16_t ModbusAddr = plc_modbusTcp_address[addr % plc_modbusTcp_address_length]; // Ensure we are within the defined PLC address range
    if (is_enable_printf_debug){
      Serial.println("Get Name from Holding Register: " + String(ModbusAddr));
    }   
    if(ModbusAddr == 2002){
      vTaskDelay(200 / portTICK_PERIOD_MS);
      mbTcp.readHreg(remote, ModbusAddr, modbusTcpName, 20, nullptr, modbusTcp_UnitId);  // Read Holding Register from Modbus Slave
      vTaskDelay(200 / portTICK_PERIOD_MS);

      // Convert modbusTcpString to name
      for(int i = 0; i < 20; i++) {
        uint16_t Dchar = modbusTcpName[i];
        name[i*2] = Dchar & 0xFF; // Low byte
        name[i*2+1] = (Dchar >> 8) & 0xFF; // High byte
        // Stop at double null
        if (name[i*2] == '\0' && name[i*2+1] == '\0') {
          name[i*2] = '\0';
          break;
        }
      }
    }
    if(ModbusAddr == 2026){
      vTaskDelay(200 / portTICK_PERIOD_MS);
      mbTcp.readHreg(remote, ModbusAddr, modbusTcpProduct, 20, nullptr, modbusTcp_UnitId);  // Read Holding Register from Modbus Slave
      vTaskDelay(200 / portTICK_PERIOD_MS);

      // Convert modbusTcpString to name
      for(int i = 0; i < 20; i++) {
        uint16_t Dchar = modbusTcpProduct[i];
        name[i*2] = Dchar & 0xFF; // Low byte
        name[i*2+1] = (Dchar >> 8) & 0xFF; // High byte
        // Stop at double null
        if (name[i*2] == '\0' && name[i*2+1] == '\0') {
          name[i*2] = '\0';
          break;
        }
      }
    }

    Serial.println("String from [" + String(ModbusAddr) + "] " + String(name));
  }else{
    return String("Invalid Address");
  }
  return String(name);
}