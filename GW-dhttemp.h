#ifndef GW_DHTTEMP
#define GW_DHTTEMP


//#include "GW-wifi.h"

/* Will do the actual work of reading the decvices. Defined in Sensors.h */
int getAllTemperatures();

/* CPU Task Variables */
TaskHandle_t tempTaskHandle = NULL; /** Task handle pointing to reader task */
bool readingTaskEnabled = false; /** Flag for main program to enable the sensor read task */

/* CPU Task Functions */
void taskReadOnSecondCore(void *pvParameters);
void readDhtSensors(); //Will wake the CPU core for reading DHT sensors



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



void taskReadOnSecondCore(void *pvParameters) {
  Serial.println("Sensor reading Task initiated.");
  while (1) // tempTask loop
  {
    if (readingTaskEnabled) {
      getAllTemperatures();
    }
    // Got sleep again
    vTaskSuspend(NULL);
  }
}

void readDhtSensors() {
  if (tempTaskHandle != NULL) {
     xTaskResumeFromISR(tempTaskHandle);
  }
}

bool initDhtBackgroundReaderTask() {
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
  }
  return true;
}



#endif