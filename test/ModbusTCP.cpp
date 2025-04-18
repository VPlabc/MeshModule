//Client = Master    Server = Slave

#include <Arduino.h>

#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include <ModbusIP_ESP8266.h>

#include <WiFi.h>
// #include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ModbusRTU.h>
#include <Wiegand.h>
#include <SoftwareSerial.h>

//VCC TX RX GND

#define LOG(string) {Serial.print(string);}
#define DB_LN(string) {Serial.println(string);}

#define SET_DEBUG true

// #define MBUS_TXD_PIN 16
// #define MBUS_RXD_PIN 17
#define MBUS_TXD_PIN 14
#define MBUS_RXD_PIN 12
// #include <ModbusEthernet.h>


//-----------------------------------------------------------------------------------------------------------------------------------------------
//Holding Registers ESP32  WECON LX5S SERVER Table Word Address D0=4096  D1=4097 D2=4098 D3=4099
uint16_t D0_Read = 0;
uint16_t D1_Read = 1;
uint16_t D2_Read = 2;
uint16_t D3_Read = 3;

uint16_t D0_value_Read = 0;
uint16_t D1_value_Read = 0;
uint16_t D2_value_Read = 0;
uint16_t D3_value_Read = 0;


uint16_t D4_Write = 4;
uint16_t D5_Write = 5;//
uint16_t D6_Write = 6;//
uint16_t D7_Write = 7;//

//ใช้ Count +1 Up ในการ เขียน  ค่าของ Holding Registers จาก ESP32 ไปยัง PLC WECON LX5S SERVER
int INCP_D4 = -1;
int INCP_D5 = -1;
int INCP_D6 = -1;
int INCP_D7 = -1;

//-----------------------------------------------------------------------------------------------------------------------------------------------

const int M0 = 8;  
const int M1 = 9;  

const int M2 = 10;  
const int M3 = 11;   

#ifdef BOARD_4I10O2AER
const int ledPin = RL1; //GPIO0
const int ledPin1 = RL2; //GPIO0
const int ledPin2 = RL3; //GPIO0
const int ledPin3 = RL4; //GPIO0
#endif//BOARD_4I10O2AER

const uint16_t REG = 0;               // Modbus Hreg Offset
const uint16_t PORT = 502;               // Modbus Hreg Offset
//Modbus Registers Offsets
const int BIT_ISTS = 5;
//Used Pins
const int switchPin = 0; //GPIO0
//Used Pins
// const int ledPin = RL1; //GPIO0
// const int ledPin1 = RL2; //GPIO0
// const int ledPin2 = RL3; //GPIO0
// const int ledPin3 = RL4; //GPIO0

IPAddress remote(192, 168, 2, 255);  // Address of Modbus Slave device
const int32_t showDelay = 2000;   // Show result every n'th mellisecond

// Enter a MAC address and IP address for your controller below.
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE
};
ModbusIP Emb;               // Declare ModbusTCP instance


// // const char *ssid = "MLTECH_SHOP";
// // const char *password = "mltech@2019";
const char *url = "http://api.qtct.vn/Home/getDataById";

unsigned long previousMillis = 0;
// const long interval = 2*60*1000;
const long interval = 5000;
unsigned long currentMillis = 0;

uint16_t res = 0;
uint16_t res1 = 0;
uint16_t Res[10];
uint32_t showLast = 0;

// HTTPClient http;
ModbusRTU mb;
// String inputString = "";         // a String to hold incoming data
// bool stringComplete = false;  // whether the string is complete
// void serialEvent();
String sendData(String command, const int timeout, boolean debug);
bool moduleStateCheck();


ModbusIP ModbusClient;



#include "0_CoilReg.h"


//--- สร้าง Task
TaskHandle_t Task1;
#include "31_Tsk1.h"



void MasterTCPsetup() {
  Serial.begin(115200);

  WiFi.begin("Lau B.", "12345678");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  DB_LN("");
  DB_LN("WiFi connected");
  DB_LN("IP address: ");
  DB_LN(WiFi.localIP());

  //------------------------------------------------------------------------
  //Task1 :
  xTaskCreatePinnedToCore(
    Task1code,   /* Task function. */
    "Task1",     /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task*/
    &Task1,      /* Task handle to keep track of created task */
    1);          /* pin task to core 0/1 */
  delay(500);
  //------------------------------------------------------------------------

  pinMode(ledPin, OUTPUT);
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  digitalWrite(ledPin, HIGH);
  digitalWrite(ledPin3, HIGH);
  ModbusClient.client();


  //ESP32 Client  รับค่า Coil M2-M3  ของ  PLC WECON LX5S SERVER
  ModbusClient.addCoil(M2);
  ModbusClient.onSetCoil(M2, M2CoilGet);
  ModbusClient.addCoil(M3);
  ModbusClient.onSetCoil(M3, M3CoilGet);

}



void MasterTCPloop() {
 
      digitalWrite(ledPin1,!digitalRead(ledPin1));
      digitalWrite(ledPin2,!digitalRead(ledPin2));
      
      digitalWrite(ledPin,!digitalRead(ledPin));
      digitalWrite(ledPin3,!digitalRead(ledPin3));

  delay(1000);

}


// #define MASTER




// Callback receives raw data 


void SlaveTCPsetup()
{
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, MBUS_TXD_PIN, MBUS_RXD_PIN);
  pinMode(switchPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  digitalWrite(ledPin, HIGH);
  digitalWrite(ledPin3, HIGH);
  mb.begin(&Serial2);
  managerInit();
  WiFi.begin(ssid, password);
  static byte counterConnect = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    DB_LN("Connecting to WiFi...");
    counterConnect++;if(counterConnect > 20){break;}
  }
  DB_LN("Connected to WiFi");
  DB_LN("IP: " + WiFi.localIP().toString());
  
  mb.slave(1);
  mb.addHreg(0, 0, 10);
#ifdef MASTER
 Emb.server(PORT);              // Act as Modbus TCP server
 Emb.addReg(HREG(REG),10);     // Add Holding register #100
 DB_LN("Init Modbus TCP master");
 Emb.addIsts(BIT_ISTS);
#else
  delay(1000);              // give the Ethernet shield a second to initialize
  Emb.client();              // Act as Modbus TCP server
 DB_LN("Init Modbus TCP slave");
  Emb.addReg(HREG(REG));     // Add Holding register #100

  Emb.addCoil(BIT_ISTS);
#endif//MASTER

inputString.reserve(200);


}

void SlaveTCPloop()
{
  mb.task();
  Emb.task();                // Server Modbus TCP queries
  
  // Initiate Read Hreg from Modbus Slave
      if (millis() - showLast > showDelay) { // Display register value every 5 seconds (with default settings)
      showLast = millis();
      res = millis()/1000;
      DB_LN(res);
      digitalWrite(switchPin,!digitalRead(switchPin));
      Emb.Ists(BIT_ISTS, digitalRead(switchPin));
      Emb.Hreg(REG, res);
      DB_LN("REG 4 :"+ String(Emb.Hreg(4)));
      ;
    }

  #else
  if (Emb.isConnected(remote)) {   // Check if connection to Modbus Slave is established
    Emb.readHreg(remote, REG, &res, 10);  // Initiate Read Hreg from Modbus Slave
    } else {
      // DB_LN("port: " + String(ethernet.Port.toInt()));
      Emb.connect(remote, ethernet.Port.toInt());           // Try to connect if not connected
    }
    // delay(100);                     // Pulling interval
    Emb.task();                      // Common local Modbus task
    if (millis() - showLast > showDelay) { // Display register value every 5 seconds (with default settings)
      showLast = millis();
      DB_LN(res);
      DB_LN("Connect: " + String(Emb.isConnected(remote)))
    }
    // digitalWrite(ledPin, Emb.Coil(BIT_ISTS));
    
      res = millis()/1000;
      DB_LN(res);
      digitalWrite(switchPin,!digitalRead(switchPin));
      Emb.writeHreg(remote, REG, res);
      Emb.writeHreg(remote, REG+1, res);
      Emb.writeHreg(remote, REG+2, res);
      Emb.readHreg(remote,  REG+3, &res1);
      Serial.print("D3_value_Read = "); DB_LN(res1);
      digitalWrite(ledPin1,!digitalRead(ledPin1));
      digitalWrite(ledPin2,!digitalRead(ledPin2));
      
      digitalWrite(ledPin,!digitalRead(ledPin));
      digitalWrite(ledPin3,!digitalRead(ledPin3));

//   #endif//MASTER
}
