#include "RTC_Online.h"


WiFiUDP ntpUDP;
#include "RTC.h"
RTCTime rtctime;
struct ts dtime;

NTPClient timeClient(ntpUDP);

byte Gethour = 0;
byte Getmin = 0;
byte Getsec = 0;
byte Getday = 0;
byte Getmonth = 0;
int Getyear = 0;
long GetEpoch = 0;

// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;
void RTCTimeOnline::RTCGetTime(){
    #ifdef RTC_DS3231
    dtime = rtctime.GetRTC();
    #else
    LOGLN("RTC_DS3231 not defined");
    return;
    #endif//RTC_DS3231
    Getyear = dtime.year;
    Getmonth = dtime.mon;
    Getday = dtime.mday;
    Gethour = dtime.hour;
    Getmin = dtime.min;
    Getsec = dtime.sec;
    GetEpoch = dtime.unixtime;
}
void Time_setup();
void Time_loop();
void  SetTimeForRTC();
void RTCTimeOnline::GetTime(){
  #ifdef RTC_Onl
  // DB_LN("Wifimode: " + String(WifiMode));
// if(WifiMode < 2){
    if(WiFi.status() == WL_CONNECTED) { 
    // while(!timeClient.update()) {
    //   timeClient.forceUpdate();
    // }
    for(int i = 0; i < 5; i++){
    Time_loop();
    // The formattedDate comes with the following format:
    // 2018-05-28T16:00:13Z
    // We need to extract date and time
    formattedDate = timeClient.getFormattedDate();
    Getyear = formattedDate.substring(0, 4).toInt(); //Serial.print("year: ");Serial.print(Getyear);
    Getmonth = formattedDate.substring(5, 7).toInt(); //Serial.print(" | month: ");Serial.print(Getmonth);
    Getday = formattedDate.substring(8, 10).toInt(); //Serial.print(" | day: ");Serial.println(Getday);
    Gethour = formattedDate.substring(11, 13).toInt(); //Serial.print("h: ");Serial.print(Gethour);
    Getmin = formattedDate.substring(14, 16).toInt(); //Serial.print(" m: ");Serial.print(Getmin);
    Getsec = formattedDate.substring(17, 19).toInt(); //Serial.print(" s: ");Serial.println(Getsec);
    GetEpoch = timeClient.getEpochTime();
    if(Getyear == 1900 || Getyear == 1970){}else{i = 6;}
    }
    if(Getyear == 1900 || Getyear == 1970){RTCGetTime();}
    }else{
      Serial.println("Error in WiFi connection");
      RTCGetTime();
    }
    formattedDate.clear();
  // }
  #endif//RTC_Onl
}
RTCTime cfRTCtime;
struct ts RTCdatetime;
void  RTCTimeOnline::SetTimeForRTC(){
#ifdef RTC_DS3231
  DB_LN("Check Time RTC");
  cfRTCtime.RTCSetup();
  RTCdatetime = cfRTCtime.GetRTC();
  DB("Time RTC ");DB_LN(String(RTCdatetime.hour) + ":" + String(RTCdatetime.min) + " | " + String(RTCdatetime.mday) + "/" + String(RTCdatetime.mon) + "/" + String(RTCdatetime.year));
  GetTime();
  DB("Time Online ");DB_LN(String(Gethour) + ":" + String(Getmin) + " | " + String(Getday) + "/" + String(Getmonth) + "/" + String(Getyear));
  if((RTCdatetime.hour == Gethour && RTCdatetime.min == Getmin && RTCdatetime.mday == Getday && RTCdatetime.mon == Getmonth && RTCdatetime.year == Getyear)&&(Getyear != 1900 && Getyear != 1970)){DB_LN("Real Timne ok!");}
  else{if((Getyear != 1900 && Getyear != 1970)){DB_LN("SetTimeForRTC");cfRTCtime.SetRTC(Getyear, Getmonth, Getday, Gethour, Getmin, Getsec, GetEpoch);RTCdatetime = cfRTCtime.GetRTC();
  DB_LN(String(RTCdatetime.hour) + ":" + String(RTCdatetime.min) + " | " + String(RTCdatetime.mday) + "/" + String(RTCdatetime.mon) + "/" + String(RTCdatetime.year)); }
  }
#endif//RTC_DS3231
}
void RTCTimeOnline::Time_setup() {
  // Initialize Serial Monitor
  if(WiFi.status() == WL_CONNECTED){
  // Initialize a NTPClient to get time
    timeClient.begin();
    // Set offset time in seconds to adjust for your timezone, for example:
    // GMT +1 = 3600
    // GMT +8 = 28800
    // GMT -1 = -3600
    // GMT 0 = 0
    timeClient.setTimeOffset(+7);
  }
}
bool TimeOnce = 1;
bool TimeDSOnce = 1;
void RTCTimeOnline::Time_loop() {
 if(WiFi.status() == WL_CONNECTED){
    if(!timeClient.update()) {
      timeClient.forceUpdate();
    }else{
    // The formattedDate comes with the following format:
    // 2018-05-28T16:00:13Z
    // We need to extract date and time
    formattedDate = timeClient.getFormattedDate();
    // if(TimeOnce){Serial.println(formattedDate);}

    // Extract date
    int splitT = formattedDate.indexOf("T");
    if(Getyear != 1900 && Getyear != 1970){
      dayStamp = formattedDate.substring(0, splitT);
      timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
      if(TimeOnce){
      Serial.print("DATE: ");
      Serial.println(dayStamp);}
      // Date = dayStamp;
      // Extract time
      if(TimeOnce){
      Serial.print("TIME: ");
      Serial.println(timeStamp);TimeOnce = false;}
      // Time = timeStamp;
      GetEpoch = timeClient.getEpochTime();
      }
      // dayStamp.clear();timeStamp.clear();
    }
  //   delay(1000);
 }
}