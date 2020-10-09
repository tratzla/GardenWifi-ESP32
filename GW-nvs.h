#ifndef GW_NVS
#define GW_NVS

#include "Preferences.h"
#include "GW-sensors.h"
#include "GW-datalog.h"




/* We will try to save and restore alarm SP, 
 * State, Data, Output through deepsleeps using RTC
 */
RTC_DATA_ATTR persist_alm rtcTT100;
RTC_DATA_ATTR persist_alm rtcTT101;
RTC_DATA_ATTR persist_alm rtcMT200;
RTC_DATA_ATTR persist_alm rtcMT201;

#define NVS_NAMESPACE "GardenWifi"
Preferences prefs;

void writeAlarmToNvs(String name, alarm_config &alarm){
  log_w("Attempting to save into nvs:<%s>",name);
  
  // prefs.remove(String(name + "in_use").c_str());
  prefs.putBool(String(name + "in_use").c_str(), alarm.in_use);
  
  // prefs.remove(String(name + "compare").c_str());
  prefs.putUChar(String(name + "compare").c_str(), alarm.compare);
  
  // prefs.remove(String(name + "sp").c_str());
  prefs.putFloat(String(name + "sp").c_str(), alarm.sp);

  alarm.unsaved_change = false;
}

void saveAlarmData(){
  // if( !TT100.alarm.unsaved_change && !TT101.alarm.unsaved_change &&
  //     !MT200.alarm.unsaved_change && !MT201.alarm.unsaved_change ){
  //     return;
  //     }
      
  if(TT100.alarm.unsaved_change) log_w("New alarm Config! it's for %s",TT100.name);
  if(TT101.alarm.unsaved_change) log_w("New alarm Config! it's for %s",TT101.name);
  if(MT200.alarm.unsaved_change) log_w("New alarm Config! it's for %s",MT200.name);
  if(MT201.alarm.unsaved_change) log_w("New alarm Config! it's for %s",MT201.name);

  
  // if(SERIAL_ENABLED) Serial.flush();Serial.end();
  bool nvs_started = prefs.begin("GardenWifi", false);
  delay(150);
  if(!nvs_started) {
    log_e("Problem starting NVS for saveAlarmData data");
    return;
  }
  
  // writeAlarmToNvs(TT100.name, TT100.alarm);
  // writeAlarmToNvs(TT101.name, TT101.alarm);
  // writeAlarmToNvs(MT200.name, MT200.alarm);
  // writeAlarmToNvs(MT201.name, MT201.alarm);

  if(TT100.alarm.unsaved_change) writeAlarmToNvs(TT100.name, TT100.alarm);
  if(TT101.alarm.unsaved_change) writeAlarmToNvs(TT101.name, TT101.alarm);
  if(MT200.alarm.unsaved_change) writeAlarmToNvs(MT200.name, MT200.alarm);
  if(MT201.alarm.unsaved_change) writeAlarmToNvs(MT201.name, MT201.alarm);

  prefs.end();
  // if(SERIAL_ENABLED) Serial.begin(SERIALSPEED);  

  log_w("   Done Saving to NVS.");

}

void loadAlarmFromNvs(String name, alarm_config &alarm) {
  alarm.in_use = prefs.getBool(String(name + "in_use").c_str(), false);
  alarm.compare = compare_states(prefs.getUChar(String(name + "compare").c_str(), COMP_EQ));
  alarm.sp = prefs.getFloat(String(name + "sp").c_str(), NAN);
}


/*
 * ALARMING
 * Some simple alarming action. Nothing fancy
 * this should be improved. Also manually save  
 *  and load alarm data to RTC memory.
 */
void checkAlarmActions(){
  if( TT100.alarm.state == ALARM || 
      TT101.alarm.state == ALARM   ) {
        digitalWrite(13, HIGH);
  } else {
        digitalWrite(13, LOW);
  }

  if( MT200.alarm.state == ALARM || 
      MT201.alarm.state == ALARM   ) {
        digitalWrite(12, HIGH);
  } else {
        digitalWrite(12, LOW);
  }

}

void loadAlarmData(){

  // MT200.alarm.configureAlarm(COMP_GT, 60);
  // MT201.alarm.configureAlarm(COMP_GT, 65);
  // TT100.alarm.configureAlarm(COMP_LT, 26);
  // TT101.alarm.configureAlarm(COMP_LT, 20);

  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);

  if ( esp_reset_reason() == ESP_RST_DEEPSLEEP) {
    log_w("Looks like we just woke up from sleep. Try to load alarm data");
    if(TT100.alarm.in_use) TT100.alarm.loadFromRTC(rtcTT100);
    if(TT101.alarm.in_use) TT101.alarm.loadFromRTC(rtcTT101);
    if(MT200.alarm.in_use) MT200.alarm.loadFromRTC(rtcMT200);
    if(MT201.alarm.in_use) MT201.alarm.loadFromRTC(rtcMT201);

    gpio_hold_dis(GPIO_NUM_12);
    gpio_hold_dis(GPIO_NUM_13);
  } else {
    log_w("Looks like we are booting fresh: Load alarm Prefs from nvs::%s", NVS_NAMESPACE);
    bool nvs_started = prefs.begin("GardenWifi", true);
    delay(150);

    if(!nvs_started) {
      log_e("Problem starting NVS for loading data");
      return;
    }
    
    
    loadAlarmFromNvs(TT100.name, TT100.alarm);
    log_w("Loaded ALM CONFIG for %s :: %s", TT100.name, TT100.alarm.str());
      
    loadAlarmFromNvs(TT101.name, TT101.alarm);
    log_w("Loaded ALM CONFIG for %s :: %s", TT101.name, TT101.alarm.str());
      
    loadAlarmFromNvs(MT200.name, MT200.alarm);
    log_w("Loaded ALM CONFIG for %s :: %s", MT200.name, MT200.alarm.str());
      
    loadAlarmFromNvs(MT201.name, MT201.alarm);
    log_w("Loaded ALM CONFIG for %s :: %s", MT201.name, MT201.alarm.str());

    prefs.end();
    delay(150);
  }
      checkAlarmActions();
}


//////
void saveAlarmDataInRTC(){
  if(TT100.alarm.in_use) TT100.alarm.copyToRTC(rtcTT100);
  if(TT101.alarm.in_use) TT101.alarm.copyToRTC(rtcTT101);
  if(MT200.alarm.in_use) MT200.alarm.copyToRTC(rtcMT200);
  if(MT201.alarm.in_use) MT201.alarm.copyToRTC(rtcMT201);
}
/**********************/




void queueSensorDataLogs() { 
  readingTaskEnabled = false;
  if(alarm_config_unsaved_change){
    saveAlarmData();
    alarm_config_unsaved_change = false;
  }

    
  getMoisture(MT200);
  getMoisture(MT201);
  logDhtData(TT100);
  logDhtData(TT101);
  checkAlarmActions();
  readingTaskEnabled = true;
}










#endif