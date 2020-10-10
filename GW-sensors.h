#ifndef GW_SENSORS
#define GW_SENSORS




#include "GW-dhttemp.h"
#include "GW-datalog.h"
#include "DHTesp.h"
#include "GW-influx.h"

#define SENSOR_IDLE 0
#define SENSOR_READING 1
#define SENSOR_UPLOADING 2
#define SENSOR_BUSY 3
#define SENSOR_FAULT 4


#define MIN_MOIST_ADC_READING 500

bool alarm_config_unsaved_change = false;

/* DHT Sensor Item, including datalog */
// class DhtSensor {
//   public:
//     DhtDatalog log;
//     DHTesp dht;
//     alarm_config alarm;

//     uint state;
//     int pin;
//     String name;
//     String strRepr;
//     String strStr;
//     const char *str() {
//       char buf[64];
//       sprintf(buf, "<%s [pin=%d len=%d]>", name, pin, log.len);
//       strStr = String(buf);
//       return strStr.c_str();
//     }
//     const char *repr() {
//       char buf[128];
//       DhtDataPoint point = log.getData();
//       sprintf(buf, "<%s%s [pin=%d len=%d] {T=%.1f|H=%.1f|D=%.1f|ts=%d}>",
//           name, alarm.str(),pin, log.len,
//           point.temperature, 
//           point.humidity, 
//           point.dewpoint,
//           point.timestamp);
//       strRepr = String(buf);
//       return strRepr.c_str();
//     }

//     void init(const char *str, int pin){
//       name = String(str);
//       this->pin = pin;
//       state = SENSOR_IDLE;
//       log_v("Starting Sensor %s...", this->repr());
//       this->dht.setup(pin, DHTesp::DHT11);
//       log_v("done.");
//     }
//     DhtSensor(){}
//     void newDp(float t, float h, float d) {
//       log.newDp(t,h,d);
//       if (alarm.in_use) alarm.newDp(t);
//     }
//     DhtDataPoint getData(){return log.getData();}
//     DhtDataPoint popData(){return log.popData();}
//     uint length(){return log.len;}
// };







/* Soil Moisture Sensor Item, including datalog */
// class MoistSensor {
//   public:
//     MoistDatalog log;
//     alarm_config alarm;

//     uint state;
//     int pin;
//     String name;
//     String strRepr;
//     String strStr;
//     const char *str() {
//       char buf[64];
//       sprintf(buf, "<%s [pin=%d len=%d]>", name, pin, log.len);
//       strStr = String(buf);
//       return strStr.c_str();
//     }
//     const char *repr() {
//       char buf[128];
//       MoistDataPoint point = log.getData();
//       sprintf(buf, "<%s%s [pin=%d len=%d] {M=%.1f|v=%.1f|r=%.1f|ts=%d}>",
//           name, alarm.str(), pin, log.len,

//           point.moisture,
//           point.volts,
//           point.raw,
//           point.timestamp);
//       strRepr = String(buf);
//       return strRepr.c_str();
//     }
//     void init(const char *str, int pin){
//       name = String(str);
//       this->pin = pin;
//       state = SENSOR_IDLE;
//       log_v("Starting Moist Sensor %s...", this->repr());
//       log_v("done.");
//     }

//     MoistSensor(){}
//     void newDp(float m, float v, float r) {
//       log.newDp(m,v,r);
//       if (alarm.in_use) alarm.newDp(m);
//     }
//     MoistDataPoint getData(){return log.getData();}
//     MoistDataPoint popData(){return log.popData();}
//     uint length(){return log.len;}
// };



// #define tt100_pin 5
// DhtSensor TT100;

// #define tt101_pin 18
// DhtSensor TT101;

// #define mt200_pin 35
// MoistSensor MT200;

// #define mt201_pin 34
// MoistSensor MT201;


#define tt100_pin 5
#define tt101_pin 18
#define mt200_pin 35
#define mt201_pin 34

#define TT_ALM_OUTPUT_PIN 12
#define MT_ALM_OUTPUT_PIN 13



/* Datalogs use the special RTC_DATA_ATTR to be stored
 * in the RTC and thus persist through Deep Sleep
 */
RTC_DATA_ATTR data_log_t rtcTT100;
RTC_DATA_ATTR data_log_t rtcTT101;
RTC_DATA_ATTR data_log_t rtcMT200;
RTC_DATA_ATTR data_log_t rtcMT201;

SensorNG TT100;
SensorNG TT101;
SensorNG MT200;
SensorNG MT201;


void initSensors() {
  log_v("Starting Sensors...");
    
  TT100.init("TT-100", tt100_pin, SENS_TYP_DHT, &rtcTT100);
  TT101.init("TT-101", tt101_pin, SENS_TYP_DHT, &rtcTT101);
  initDhtBackgroundReaderTask(); // Will setup background task and put it to sleep
  log_v("  ...DHT Sensors initiated.");

  MT200.init("MT-200", mt200_pin, SENS_TYP_MOIST, &rtcMT200);
  MT201.init("MT-201", mt201_pin, SENS_TYP_MOIST, &rtcMT201);
  log_v("  ...MOIST Sensors initiated.");

  pinMode(TT_ALM_OUTPUT_PIN, OUTPUT);
  pinMode(MT_ALM_OUTPUT_PIN, OUTPUT);

  gpio_hold_dis(GPIO_NUM_12); // ensure pins are not longer held from a sleep-hold
  gpio_hold_dis(GPIO_NUM_13);
}

bool logMoistData(SensorNG &sensor) {
  if (sensor.activity != SENSOR_IDLE) {
    log_v("\nSensor %s busy <%d>, log later.\n", sensor.srepr(), sensor.activity);
    return SENSOR_BUSY;
  }
  sensor.activity = SENSOR_UPLOADING;
  // uint len = sensor.length();
  // for (int j=0;j<len;j++) {
    // data_point_t dp = sensor.popData();
    data_point_t dp = sensor.getData();
    Point point(sensor.name);

    point.addTag(  "name",     sensor.name);
    point.addTag(  "pin",      String(sensor.pin));
    point.addField("moisture", dp.val[MOISTURE]);
    point.addField("volts",    dp.val[VOLTS]);
    point.addField("raw",      dp.val[RAW]);

    if (sensor.alm_inUse()) {
      point.addField("alm_SP",    sensor.AlarmConfig->sp);
      point.addField("alm_COMP",  sensor.AlarmConfig->compare);
      point.addField("alm_STATE", sensor.AlarmConfig->state);
    }

    if (!writeNewPoint(point)) {
      log_i("  InfluxDB write status failed: %s", getLastErrorMessage());
      sensor.activity = SENSOR_IDLE;
      return false;
    }
  // }
  log_v("  Data QUEUED for upload");
  sensor.activity = SENSOR_IDLE;
  return true;
}

int getMoisture(SensorNG &sensor){
  if (sensor.activity == SENSOR_UPLOADING) {
    log_v("\nSensor %s: Busy UPLOADING, read sensor later.\n", sensor.srepr());
    return SENSOR_UPLOADING;
  }
  sensor.activity = SENSOR_READING;

  delay(15);
  uint reading = analogRead(sensor.pin);

  if (reading < MIN_MOIST_ADC_READING) {
      log_d("\nSensor %s: Problem Reading ANALOG INPUT. RAW=%d.\n", sensor.srepr(),reading);

        sensor.activity = SENSOR_IDLE;
        return SENSOR_BUSY;
  }
  /* Heltec Docs thought this polynomial was more accurate. I can't confirm that */
  double volts = -0.000000000000016 * pow(reading,4) + 0.000000000118171 * pow(reading,3)- 0.000000301211691 * pow(reading,2)+ 0.001109019271794 * reading + 0.034143524634089;
  int imoist = map((int)(volts*1000), 955, 3100, 100000, 10000);
  float moist = (float)imoist/1000.0;
  sensor.newData(moist, volts, reading);

  log_i("Sensor %s: READ data ", sensor.repr());
  sensor.activity = SENSOR_IDLE;
  if (logMoistData(sensor)) return SENSOR_IDLE;
  return SENSOR_FAULT;
}

// int getAllMoistures() {
//   getMoisture(MT200);
//   getMoisture(MT201);
// }


int getDhtTemperature(SensorNG &sensor) {
    if (sensor.activity == SENSOR_UPLOADING) {
      log_v("\nSensor %s: Busy UPLOADING, read sensor later.\n", sensor.srepr());
      return SENSOR_UPLOADING;
    }
    sensor.activity = SENSOR_READING;
    // TempAndHumidity newValues = sensor.dht.getTempAndHumidity();

    DHTesp dht;
    dht.setup(sensor.pin, DHTesp::DHT11);
    TempAndHumidity newValues = dht.getTempAndHumidity();

    if (dht.getStatus() != 0) {
        log_i("Sensor %s: DHTesp error status: %s\n", 
           sensor.srepr(), dht.getStatusString());
        sensor.activity = SENSOR_IDLE;
        return dht.getStatus();
    }
    float dewpoint = dht.computeDewPoint(newValues.temperature, newValues.humidity);
    sensor.newData(newValues.temperature, newValues.humidity, dewpoint);

    log_i("Sensor %s: Read data\n", sensor.repr());
    sensor.activity = SENSOR_IDLE;
    return SENSOR_IDLE;
}

/* This function must be declared as is. This will get called by the
 * background task reading the DHT temperature sensors 
 */
int getAllTemperatures() {
  getDhtTemperature(TT100);
  getDhtTemperature(TT101);
}



void logDhtData(SensorNG &sensor) {
  if (sensor.activity != SENSOR_IDLE) {
    log_v("\nSensor %s busy <%d>, log later.\n", sensor.srepr(), sensor.activity);
    return;
  }
  sensor.activity = SENSOR_UPLOADING;
  // uint len = sensor.length();
  // for (int j=0;j<len;j++) {
    // data_point_t dp = sensor.popData();
    data_point_t dp = sensor.getData();
    Point point(sensor.name);

    point.addTag("name", sensor.name);
    point.addTag("pin", String(sensor.pin));
    point.addField("temperature", dp.val[TEMPERATURE]);
    point.addField("humidity",    dp.val[HUMIDITY]);
    point.addField("dewpoint",    dp.val[DEWPOINT]);

    if (sensor.alm_inUse()) {
      point.addField("alm_SP",    sensor.AlarmConfig->sp);
      point.addField("alm_COMP",  sensor.AlarmConfig->compare);
      point.addField("alm_STATE", sensor.AlarmConfig->state);
    }

    if (!writeNewPoint(point)) {
      log_i("  InfluxDB write status failed: %s", getLastErrorMessage());
    }
  // }
  sensor.activity = SENSOR_IDLE;
}



/*
 * ALARMING
 * Some simple alarming action. Nothing fancy
 * this should be improved. Also manually save  
 *  and load alarm data to RTC memory.
 */
void checkAlarmActions(){
  if( TT100.AlarmConfig->state == ALARM || 
      TT101.AlarmConfig->state == ALARM   ) {
        digitalWrite(13, HIGH);
  } else {
        digitalWrite(13, LOW);
  }

  if( MT200.AlarmConfig->state == ALARM || 
      MT201.AlarmConfig->state == ALARM   ) {
        
        digitalWrite(12, HIGH);
  } else {
        digitalWrite(12, LOW);
  }

}



void queueSensorDataLogs() { 
  readingTaskEnabled = false;
    
  getMoisture(MT200);
  getMoisture(MT201);
  logDhtData(TT100);
  logDhtData(TT101);
  checkAlarmActions();
  readingTaskEnabled = true;
}



#endif