#ifndef GW_NVS
#define GW_NVS

#include "Preferences.h"

#define NVS_NAMESPACE "GardenWifi"
Preferences prefs;

void saveSleeps_nvs(uint timeAwake, uint timeSleeping){
  bool nvs_started = prefs.begin(NVS_NAMESPACE, false);
  if(nvs_started) {
    prefs.putUInt("time_awake", timeAwake);
    prefs.putUInt("time_sleeping", timeSleeping);
    
    prefs.end();
    log_i("Saved new sleep setpoints to NVS :: TIME_AWAKE=%ds  TIME_SLEEPING=%dmins", timeAwake, timeSleeping);
  }
}

void loadSleeps_nvs(uint &timeAwake, uint &timeSleeping){
  bool nvs_started = prefs.begin(NVS_NAMESPACE, true);
  if(nvs_started) {
    timeAwake = prefs.getUInt("time_awake", timeAwake);
    timeSleeping = prefs.getUInt("time_sleeping", timeSleeping);

    prefs.end();
    log_i("Read sleep setpoints from NVS :: TIME_AWAKE=%ds  TIME_SLEEPING=%dmins", timeAwake, timeSleeping);
  }
}


void SensorNG::loadAlmConfig_nvs(){
  this->AlarmConfig->in_use = false;  
  this->AlarmConfig->sp = 0.0;
  this->AlarmConfig->compare = COMP_EQ;
  this->AlarmConfig->state = UNKNOWN;
  
  bool nvs_started = prefs.begin(NVS_NAMESPACE, true);
  if(nvs_started) {
    sprintf(this->buf, "%s-%s", this->name, "in_use");
    this->AlarmConfig->in_use = prefs.getBool(String(this->buf).c_str(), false);

    sprintf(this->buf, "%s-%s", this->name, "compare");
    this->AlarmConfig->compare = compare_enum(prefs.getUChar(String(this->buf).c_str(), COMP_EQ));

    sprintf(this->buf, "%s-%s", this->name, "state");
    this->AlarmConfig->state = alarm_enum(prefs.getUChar(String(this->buf).c_str(), UNKNOWN));

    sprintf(this->buf, "%s-%s", this->name, "sp");
    this->AlarmConfig->sp = prefs.getFloat(String(this->buf).c_str(), 0.0);

    log_i("Loaded sensor %s alarm from NVS  ::  %s", this->srepr(), this->strAlarm_repr());
    prefs.end();
  }
}

void SensorNG::saveAlmConfig_nvs(){  
  log_w("Attempting to save config for %s :: %s", this->srepr(), this->strAlarm_repr());
  bool nvs_started = prefs.begin(NVS_NAMESPACE, false);

  if(nvs_started) {
    sprintf(this->buf, "%s-%s", this->name, "in_use");
    // prefs.remove(String(this->buf).c_str());
    prefs.putBool(String(this->buf).c_str(), this->AlarmConfig->in_use);

    sprintf(this->buf, "%s-%s", this->name, "sp");
    // prefs.remove(String(this->buf).c_str());
    prefs.putFloat(String(this->buf).c_str(), this->AlarmConfig->sp);

    sprintf(this->buf, "%s-%s", this->name, "compare");
    // prefs.remove(String(this->buf).c_str());
    prefs.putUChar(String(this->buf).c_str(), this->AlarmConfig->compare);

    sprintf(this->buf, "%s-%s", this->name, "state");
    // prefs.remove(String(this->buf).c_str());
    prefs.putUChar(String(this->buf).c_str(), this->AlarmConfig->state);


    log_w("Finishing NVS write.");
    prefs.end();
  }
  
}





#endif