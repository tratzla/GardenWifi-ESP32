#ifndef GW_DHTTEMP
#define GW_DHTTEMP



/* Will do the actual work of reading the decvices. 
 *Defined in Sensors.h */
int getAllTemperatures();

/* CPU Task Variables */
TaskHandle_t tempTaskHandle = NULL; /** Task handle pointing to reader task */
bool readingTaskEnabled = false; /** Flag for main program to enable the sensor read task */

/* CPU Task Functions */
void taskReadOnSecondCore(void *pvParameters);
void readDhtSensors(); //Will wake the CPU core for reading DHT sensors


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