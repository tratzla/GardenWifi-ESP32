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

struct DhtSensors {
  const DhtSensor *sensors[2];
};

/* Soil Moisture Sensor Item, including datalog */
// class SoilMoistSensorItem {


// };



void initSensors() {
  Serial.println("Starting Sensors...");
  TT100.init("TT-100", tt100_pin);
  TT101.init("TT-101", tt101_pin);
  initDhtBackgroundReaderTask(); // Will setup background task and put it to sleep
  Serial.println("  ...DHT Sensors initiated.");
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

    Serial.printf("Sensor %s: Read and Saved data\n", sensor.repr());
    sensor.state = SENSOR_IDLE;
    return 0;
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


// void sendSensorDataToInflux(SensorInfo &sensor) {
//   if (sensor.fetching) {
//     Serial.printf("\nSensor %s busy READING, upload later.\n", sensor.str());
//     return;
//   }
//   sensor.saving = true;
//   Serial.printf("\nSending data from Sensor %s to Influx...\n", sensor.str());
//   if (WiFi.status() != WL_CONNECTED) {
//     Serial.println("...Wifi connection lost, can't send DATA to Influx.");
//     sensor.saving = false;
//     return;
//   }
//   if (sensor.len <= 0 ) {
//     Serial.println("  ...sensor has no new data. Nothing to upload, done.");
//     sensor.saving = false;
//     return;
//   }

//   Point influxPoint(sensor.name);
//   influxPoint.addTag("name", sensor.name);
//   influxPoint.addTag("pin", String(sensor.pin));
//   influxPoint.addField("temperature", sensor.data[sensor.i].temperature);
//   influxPoint.addField("humidity", sensor.data[sensor.i].humidity);
//   influxPoint.addField("dewpoint", sensor.data[sensor.i].dewpoint);
//   Serial.print("writing to Influx: ");
//   Serial.println(influxPoint.toLineProtocol());
//   if (!writeNewPoint(influxPoint)) {
//     Serial.print("  InfluxDB write status failed: ");
//     Serial.println(getLastErrorMessage());
//   } else {
//     sensor.len--;
//     sensor.i = sensor.i == 0 ? datalog_max_length - 1 : sensor.i - 1;
//     Serial.println("  ...Success.\n");
//   }  
//   sensor.saving = false;      
// }


#endif