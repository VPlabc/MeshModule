#ifndef RTC_Online_h
#define RTC_Online_h

#include <Arduino.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#define RTC_Onl
#define DEBUG_OUTPUT_SERIAL

#ifdef DEBUG_OUTPUT_SERIAL
#define DEBUG_PIPE SERIAL_PIPE
#define LOG(string) {Serial.print(string);}
#define LOGLN(string) {Serial.println(string);}

#else
#define LOG(string) {}
#define LOGLN(string) {}
#endif 
extern byte Gethour;
extern byte Getmin;
extern byte Getsec;
extern byte Getday;
extern byte Getmonth;
extern int Getyear;
extern long GetEpoch;

class RTCTimeOnline {
  public:
    void RTCGetTime();
    void Time_setup();
    void Time_loop();
    void GetTime();
    void SetTimeForRTC();
};

#endif//RTC_Online_h