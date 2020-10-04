#ifndef GW_SENSORS
#define GW_SENSORS

#define _dbg 1


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

/* DHT Sensor Item, including datalog */
class DhtSensor {
  public:
    DhtDatalog log;
    DHTesp dht;
    uint state;
    int pin;
    String name;
    String strRepr;
    String strStr;
    const char *str() {
      char buf[64];
      sprintf(buf, "<%s [pin=%d len=%d]>", name, pin, log.len);
      strStr = String(buf);
      return strStr.c_str();
    }
    const char *repr() {
      char buf[128];
      DhtDataPoint point = log.getData();
      sprintf(buf, "<%s [pin=%d len=%d] {T=%.1f|H=%.1f|D=%.1f|ts=%d}>",
          name, pin, log.len,
          point.temperature, 
          point.humidity, 
          point.dewpoint,
          point.timestamp);
      strRepr = String(buf);
      return strRepr.c_str();
    }

    void init(const char *str, int pin){
      name = String(str);
      this->pin = pin;
      state = SENSOR_IDLE;
      Serial.printf("Starting Sensor %s...", this->repr());
      this->dht.setup(pin, DHTesp::DHT11);
      Serial.println("done.");
    }
    DhtSensor(){}
    DhtDataPoint getData(){return log.getData();}
    DhtDataPoint popData(){return log.popData();}
    uint length(){return log.len;}
};

#define tt100_pin 5
DhtSensor TT100;

#define tt101_pin 18
DhtSensor TT101;


/* Soil Moisture Sensor Item, including datalog */

/* DHT Sensor Item, including datalog */
class MoistSensor {
  public:
    MoistDatalog log;
    uint state;
    int pin;
    String name;
    String strRepr;
    String strStr;
    const char *str() {
      char buf[64];
      sprintf(buf, "<%s [pin=%d len=%d]>", name, pin, log.len);
      strStr = String(buf);
      return strStr.c_str();
    }
    const char *repr() {
      char buf[128];
      MoistDataPoint point = log.getData();
      sprintf(buf, "<%s [pin=%d len=%d] {M=%.1f|v=%.1f|r=%.1f|ts=%d}>",
          name, pin, log.len,
          point.moisture,
          point.volts,
          point.raw,
          point.timestamp);
      strRepr = String(buf);
      return strRepr.c_str();
    }

    void init(const char *str, int pin){
      name = String(str);
      this->pin = pin;
      state = SENSOR_IDLE;
      Serial.printf("Starting Moist Sensor %s...", this->repr());
      Serial.println("done.");
    }
    MoistSensor(){}
    MoistDataPoint getData(){return log.getData();}
    MoistDataPoint popData(){return log.popData();}
    uint length(){return log.len;}
};


#define mt200_pin 35
MoistSensor MT200;

#define mt201_pin 34
MoistSensor MT201;


void initSensors() {
  Serial.println("Starting Sensors...");
  TT100.init("TT-100", tt100_pin);
  TT101.init("TT-101", tt101_pin);
  initDhtBackgroundReaderTask(); // Will setup background task and put it to sleep
  Serial.println("  ...DHT Sensors initiated.");

  MT200.init("MT-200", mt200_pin);
  MT201.init("MT-201", mt201_pin);
  Serial.println("  ...MOIST Sensors initiated.");
}


bool logMoistData(MoistSensor &sensor) {
  if (sensor.state != SENSOR_IDLE) {
    Serial.printf("\nSensor %s busy <%d>, log later.\n", sensor.str(), sensor.state);
    return SENSOR_BUSY;
  }
  sensor.state = SENSOR_UPLOADING;
  uint len = sensor.length();
  for (int j=0;j<len;j++) {
    MoistDataPoint dp = sensor.popData();
    Point point(sensor.name);

    point.addTag("name", sensor.name);
    point.addTag("pin", String(sensor.pin));
    point.addField("moisture", dp.moisture);
    point.addField("volts", dp.volts);
    point.addField("raw", dp.raw);

    if (!writeNewPoint(point)) {
      Serial.print("  InfluxDB write status failed: ");
      Serial.println(getLastErrorMessage());
      sensor.state = SENSOR_IDLE;
      return false;
    }
  }
  sensor.state = SENSOR_IDLE;
  return true;
}

int getMoisture(MoistSensor &sensor){
  if (sensor.state == SENSOR_UPLOADING) {
    Serial.printf("\nSensor %s: Busy UPLOADING, read sensor later.\n", sensor.str());
    return SENSOR_UPLOADING;
  }
  sensor.state = SENSOR_READING;

  delay(15);
  uint reading = analogRead(sensor.pin);

  if (reading < MIN_MOIST_ADC_READING) {
      Serial.printf("\nSensor %s: Problem Reading ANALOG INPUT. RAW=%d.\n", sensor.str(),reading);

        sensor.state = SENSOR_IDLE;
        return SENSOR_BUSY;
  }
  /* Heltec Docs thought this polynomial was more accurate. I can't confirm that */
  double volts = -0.000000000000016 * pow(reading,4) + 0.000000000118171 * pow(reading,3)- 0.000000301211691 * pow(reading,2)+ 0.001109019271794 * reading + 0.034143524634089;
  int imoist = map((int)(volts*1000), 955, 3100, 100000, 10000);
  float moist = (float)imoist/1000.0;
  sensor.log.newDp(moist, volts, reading);
  // Serial.print( "\n    RAW     VOLTS      MOIST     \n");
  // Serial.printf("   %d     %.2fV      %.1f%%           \n", reading, volts, moist);

  Serial.printf("Sensor %s: READ data", sensor.repr());
  sensor.state = SENSOR_IDLE;
  if (logMoistData(sensor)) return SENSOR_IDLE;
  return SENSOR_FAULT;
}

int getAllMoistures() {
  getMoisture(MT200);
  getMoisture(MT201);
}


int getDhtTemperature(DhtSensor &sensor) {
    if (sensor.state == SENSOR_UPLOADING) {
      Serial.printf("\nSensor %s: Busy UPLOADING, read sensor later.\n", sensor.str());
      return SENSOR_UPLOADING;
    }
    sensor.state = SENSOR_READING;
    TempAndHumidity newValues = sensor.dht.getTempAndHumidity();

    if (sensor.dht.getStatus() != 0) {
        Serial.printf("\nSensor %s: DHTesp error status: %s\n", 
            sensor.str(), sensor.dht.getStatusString());
        sensor.state = SENSOR_IDLE;
        return sensor.dht.getStatus();
    }
    float dewpoint = sensor.dht.computeDewPoint(newValues.temperature, newValues.humidity);
    sensor.log.newDp(newValues.temperature, newValues.humidity, dewpoint);

    Serial.printf("Sensor %s: Read data\n", sensor.repr());
    sensor.state = SENSOR_IDLE;
    return SENSOR_IDLE;
}

/* This function must be declared as is. This will get called by the
 * background task reading the DHT temperature sensors 
 */
int getAllTemperatures() {
  getDhtTemperature(TT100);
  getDhtTemperature(TT101);
}



void logDhtData(DhtSensor &sensor) {
  if (sensor.state != SENSOR_IDLE) {
    Serial.printf("\nSensor %s busy <%d>, log later.\n", sensor.str(), sensor.state);
    return;
  }
  sensor.state = SENSOR_UPLOADING;
  uint len = sensor.length();
  for (int j=0;j<len;j++) {
    DhtDataPoint dp = sensor.popData();
    Point point(sensor.name);

    point.addTag("name", sensor.name);
    point.addTag("pin", String(sensor.pin));
    point.addField("temperature", dp.temperature);
    point.addField("humidity", dp.humidity);
    point.addField("dewpoint", dp.dewpoint);

    if (!writeNewPoint(point)) {
      Serial.print("  InfluxDB write status failed: ");
      Serial.println(getLastErrorMessage());
    }
  }
  sensor.state = SENSOR_IDLE;
}

void queueSensorDataLogs() { 
  logDhtData(TT100);
  logDhtData(TT101);
}



#endif