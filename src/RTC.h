#include "Arduino.h"
#include <Wire.h>
#include <ds3231.h>
#include <time.h>


#ifndef RTCTime_
#define RTCTime_
class RTCTime 
{
  public:
byte hours;
byte mins;
byte secs;
byte Days;
byte Months;
int16_t Years;
#define DEBUG_OUTPUT_SERIAL

#ifdef DEBUG_OUTPUT_SERIAL
#define DEBUG_PIPE SERIAL_PIPE
#define LOG(string) {Serial.print(string);}
#define LOGLN(string) {Serial.println(string);}

#else
#define LOG(string) {}
#define LOGLN(string) {}
#endif 
struct ts datetime;
void RTCSetup(){
  LOGLN("=====================================");
  LOGLN("RTC init");
#ifdef RTC_DS3231
  Wire.begin();DS3231_init(DS3231_CONTROL_INTCN);
// delay(3000);
//   DS3231_get(&datetime);
// #endif// RTC_DS3231
//   Serial.print("RTC HW: ");
//   Serial.print(String(datetime.hour) + ":" + String(datetime.min) + ":" + String( datetime.sec) + " | ");
//   Serial.println(String(datetime.mday) + "/" + String(datetime.mon) + "/" + String(datetime.year));
#endif//RTC_DS3231
  }
ts GetRTC()
{
#ifdef RTC_DS3231
  DS3231_get(&datetime);
#endif// RTC_DS3231
  secs = datetime.sec;
  mins = datetime.min;
  hours = datetime.hour;
  Days = datetime.mday;
  Months = datetime.mon;
  Years = datetime.year;
  return datetime;
}
void SetRTC(int16_t year, byte month, byte day, byte hour, byte minute, byte second, uint32_t EpochT)
{
  Wire.begin();DS3231_init(DS3231_CONTROL_INTCN);
    LOG("Time:" + String(hour)+':'+String(minute) + '\n');
    LOG("Date:" + String(day)+"/"+String(month)+"/"+String(year) + '\n');
    struct ts time_RTC;
        time_RTC.sec = second;
        time_RTC.min = minute;
        time_RTC.hour = hour;
        time_RTC.mday = day;
        time_RTC.mon = month;
        time_RTC.year = year;
        time_RTC.unixtime = EpochT;
#ifdef RTC_DS3231
        DS3231_set(time_RTC);
#endif// RTC_DS3231
   
}
};
#endif//RTCTime_ 