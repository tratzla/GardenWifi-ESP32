#ifndef GW_READSNESORTASK
#define GW_READSNESORTASK

#include "DHTesp.h"
#include "datapoint.h"
#include "GW-influx.h"
#include "GW-wifi.h"

class SensorInfo {
  public:
    DataPoint data[datalog_max_length];
    DHTesp dht;
    uint i;
    uint len;
    bool fetching;
    bool saving;
    int status;
    int pin;
    String name;
    String strRepr;
    String strStr;
    const char *str() {
      char buf[64];
      sprintf(buf, "<%s [pin=%d len=%d idx=%d]>", name, pin, len, i);
      strStr = String(buf);
      return strStr.c_str();
    }
    const char *repr() {
      char buf[128];
      sprintf(buf, "<%s [pin=%d len=%d idx=%d] {T=%.1f|H=%.1f|D=%.1f|ts=%d}>",
          name, pin, len, i,
          data[i].temperature, 
          data[i].humidity, 
          data[i].dewpoint,
          data[i].timestamp);
      strRepr = String(buf);
      return strRepr.c_str();
    }

    void init(const char *str, int pin){
      name = String(str);//name;
      this->pin = pin;
      fetching = false;
      saving = false;
      status = 0;
      i = 0;
      len = 0;
      Serial.printf("Starting Sensor %s...", this->repr());
      this->dht.setup(pin, DHTesp::DHT11);
      Serial.println("done.");
    }
    SensorInfo(){}
    DataPoint getData(){return data[i];}
};


/* Sensor Configuration Variables */
#define DHT_READ_PERIOD 15
#define SENSOR_BUSY 12

#define tt100_pin 5
SensorInfo TT100;

#define tt101_pin 18
SensorInfo TT101;


/* Sensor Functions */
int getTemperature(SensorInfo &sensor);
bool initReadSensors();


/* CPU Task Variables */
TaskHandle_t tempTaskHandle = NULL; /** Task handle pointing to reader task */
bool readingTaskEnabled = false; /** Flag for main program to enable the sensor read task */

/* CPU Task Functions */
void taskReadOnSecondCore(void *pvParameters);
void wakeCPUCore();



void sendSensorDataToInflux(SensorInfo &sensor) {
  if (sensor.fetching) {
    Serial.printf("\nSensor %s busy READING, upload later.\n", sensor.str());
    return;
  }
  sensor.saving = true;
  Serial.printf("\nSending data from Sensor %s to Influx...\n", sensor.str());
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("...Wifi connection lost, can't send DATA to Influx.");
    sensor.saving = false;
    return;
  }
  if (sensor.len <= 0 ) {
    Serial.println("  ...sensor has no new data. Nothing to upload, done.");
    sensor.saving = false;
    return;
  }

  Point influxPoint(sensor.name);
  influxPoint.addTag("name", sensor.name);
  influxPoint.addTag("pin", String(sensor.pin));
  influxPoint.addField("temperature", sensor.data[sensor.i].temperature);
  influxPoint.addField("humidity", sensor.data[sensor.i].humidity);
  influxPoint.addField("dewpoint", sensor.data[sensor.i].dewpoint);
  Serial.print("writing to Influx: ");
  Serial.println(influxPoint.toLineProtocol());
  if (!writeNewPoint(influxPoint)) {
    Serial.print("  InfluxDB write status failed: ");
    Serial.println(getLastErrorMessage());
  } else {
    sensor.len--;
    sensor.i = sensor.i == 0 ? datalog_max_length - 1 : sensor.i - 1;
    Serial.println("  ...Success.\n");
  }  
  sensor.saving = false;      
}



void taskReadOnSecondCore(void *pvParameters) {
  Serial.println("Sensor reading Task initiated.");
  while (1) // tempTask loop
  {
    if (readingTaskEnabled) {
      TT100.status = getTemperature(TT100);
      TT101.status = getTemperature(TT101);
    }
    // Got sleep again
    vTaskSuspend(NULL);
  }
}

void wakeCPUCore() {
  if (tempTaskHandle != NULL) {
     xTaskResumeFromISR(tempTaskHandle);
  }
}

bool initReadSensors() {
  byte resultValue = 0;

  Serial.println("Starting Sensors...");
  TT100.init("TT-100", tt100_pin);
  TT101.init("TT-101", tt101_pin);
  // Initialize temperature sensor
  //dht_tt100.setup(tt100_pin, DHTesp::DHT11);
  //tt101.setup(tt101_pin, DHTesp::DHT11);
  Serial.println("  ...DHT Sensors initiated.");

  // Start task to get temperature
  xTaskCreatePinnedToCore(
      taskReadOnSecondCore,            /* Function to implement the task */
      "tempTask ",                    /* Name of the task */
      4000,                           /* Stack size in words */
      NULL,                           /* Task input parameter */
      5,                              /* Priority of the task */
      &tempTaskHandle,                /* Task handle. */
      0);                             /* Core where the task should run */

  if (tempTaskHandle == NULL) {
    Serial.println("Failed to start task for temperature update");
    return false;
  } else {
    // Start update of environment data every DHT_READ_PERIOD seconds
    //tempTicker.attach(DHT_READ_PERIOD, wakeCPUCore);
  }
  return true;
}



int getTemperature(SensorInfo &sensor) {
    if (sensor.saving) {
      Serial.printf("\nSensor %s busy SAVING, read sensor later.\n", sensor.str());
      return SENSOR_BUSY;
    }
    sensor.fetching = true;
    TempAndHumidity newValues = sensor.dht.getTempAndHumidity();
    sensor.status = sensor.dht.getStatus();

    if (sensor.status != 0) {
        Serial.printf("\nSensor %s: DHTesp error status: %s\n", 
            sensor.str(), sensor.dht.getStatusString());
        sensor.fetching = false;
        return sensor.status;
    }
    float dewpoint = sensor.dht.computeDewPoint(newValues.temperature, newValues.humidity);

    uint old_ts = sensor.data[sensor.i].timestamp;
    sensor.i = ++sensor.i >= datalog_max_length ? 0 : sensor.i;
    sensor.len = ++sensor.len >= datalog_max_length ? datalog_max_length : sensor.len;

    sensor.data[sensor.i].temperature = newValues.temperature;
    sensor.data[sensor.i].humidity = newValues.humidity;
    sensor.data[sensor.i].dewpoint = dewpoint;
    sensor.data[sensor.i].timestamp = old_ts + 1;

    Serial.printf("Got and saved data for %s\n", sensor.repr());
    sensor.fetching = false;
    return 0;
}
#endif