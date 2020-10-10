
#include "Arduino.h"
#include "GW-datalog.h"



void SensorNG::init(const char *n, int p, uint t, data_log_t *dl) {
  strcpy(this->strName, n);
  this->name = String(n);
  this->pin = p;
  this->type = t;
  // this->AlarmConfig = alm;
  this->DataLog = dl;

  this->loadAlmConfig_nvs();
}



const char* SensorNG::srepr(){
  sprintf(this->strShortRepr, "<%s [pin=%d len=%d]>", 
      this->name, this->pin, this->DataLog->len);
  return strShortRepr;
}

const char* SensorNG::repr(){
  data_point_t point = this->getData();
  sprintf(strRepr, "<%s%s [pin=%d len=%d] {v1=%.1f|v2=%.1f|v3=%.1f}>",
      this->name, this->strAlarm_repr(), this->pin, this->DataLog->len,
      point.val[0], point.val[1], point.val[2]);
  return strRepr;
}

alarm_enum SensorNG::checkAlarm() {
  if(DataLog->len < 2) {
    AlarmConfig->state = UNKNOWN;
  } else {
    float avg = 0;   // taking the average of the datapoints
    for (int j=0; j < DataLog->len; j++){
      avg += DataLog->data[j].val[0];
    }
    avg = avg/DataLog->len;

    switch(AlarmConfig->compare) {
      case COMP_GT :
        AlarmConfig->state = avg > AlarmConfig->sp ? ALARM : NORMAL;
        break; 
      case COMP_LT :
        AlarmConfig->state = avg < AlarmConfig->sp ? ALARM : NORMAL;
        break; 
      case COMP_EQ :
        AlarmConfig->state = avg == AlarmConfig->sp ? ALARM : NORMAL;
        break; 
    }
  }
  return AlarmConfig->state;
}

void SensorNG::changeConfig(uint whichParam){
  switch(whichParam)
  {
    case CFG_CHNG_INUSE:
      this->AlarmConfig->in_use = !this->AlarmConfig->in_use;
      return;
    case CFG_CHNG_SP:
      if(this->type == SENS_TYP_MOIST)
        this->AlarmConfig->sp += this->AlarmConfig->sp > 100.0 ? -75.0 : 5.0;
      if(this->type == SENS_TYP_DHT)
        this->AlarmConfig->sp += this->AlarmConfig->sp >  30.0 ? -31.0 : 0.5;
      return;
    case CFG_CHNG_COMP:
      if        (this->AlarmConfig->compare == COMP_GT) {
        this->AlarmConfig->compare = COMP_LT;
      } else if (this->AlarmConfig->compare == COMP_LT) {
        this->AlarmConfig->compare = COMP_EQ;
      } else if (this->AlarmConfig->compare == COMP_EQ) {
        this->AlarmConfig->compare = COMP_GT;
      }    
      return;
  }
}

const char* SensorNG::strAlarm_Compare() {
  switch(AlarmConfig->compare) {
    // case COMP_GT: this->strAlarmCompare = "> [gt]"; break;
    // case COMP_LT: this->strAlarmCompare = "< [lt]"; break;
    // case COMP_EQ: this->strAlarmCompare = "= [eq]"; break;
    case COMP_GT: strcpy(this->strAlarmCompare, "> [gt]"); break;
    case COMP_LT: strcpy(this->strAlarmCompare, "< [lt]"); break;
    case COMP_EQ: strcpy(this->strAlarmCompare, "= [eq]"); break;
  }
  // return strAlarmCompare.c_str();
  return this->strAlarmCompare;
}

const char* SensorNG::strAlarm_State() {
  switch(AlarmConfig->state) {
    // case NORMAL : this->strAlarmState = "OK"   ; break;
    // case ALARM  : this->strAlarmState = "ALARM"; break;
    // case UNKNOWN: this->strAlarmState = "??"   ; break;
    case NORMAL : strcpy(this->strAlarmState,"OK")   ; break;
    case ALARM  : strcpy(this->strAlarmState,"ALARM"); break;
    case UNKNOWN: strcpy(this->strAlarmState,"??")   ; break;
  }
  // return strAlarmState.c_str();
  return this->strAlarmState;
}

const char* SensorNG::strAlarm_repr() {
  if (!AlarmConfig->in_use) 
    this->strAlarmRepr[0] = '\0';

  sprintf(this->strAlarmRepr, " [ALM:=%s%.1f|%s]", 
      this->strAlarm_Compare(), 
      AlarmConfig->sp,
      this->strAlarm_State());
  // strAlarmRepr = String(buf);
  // return strAlarmRepr.c_str();
  return this->strAlarmRepr;
  
}

void SensorNG::newData(float a, float b, float c){
  uint old_ts = DataLog->data[DataLog->i].timestamp;
  DataLog->i = ++DataLog->i >= DL_MAX_LEN ? 0 : DataLog->i;
  DataLog->len = ++DataLog->len >= DL_MAX_LEN ? DL_MAX_LEN : DataLog->len;

  DataLog->data[DataLog->i].val[0] = a;
  DataLog->data[DataLog->i].val[1] = b;
  DataLog->data[DataLog->i].val[2] = c;
  DataLog->data[DataLog->i].timestamp = old_ts + 1;
  if(this->AlarmConfig->in_use)
    this->checkAlarm();
}


data_point_t SensorNG::popData() {
  data_point_t retval = DataLog->data[DataLog->i];
  DataLog->len--;
  DataLog->i = DataLog->i == 0 ? DL_MAX_LEN - 1 : DataLog->i - 1;
  return retval;
}