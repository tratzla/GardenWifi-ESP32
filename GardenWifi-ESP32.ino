#include "Arduino.h"
#include "heltec.h"
#include "GW-display.h"
//#include "GW-wifi.h"
//#include "GW-influx.h"
#include "DHTesp.h"
#include "Ticker.h"
#include "datapoint.h"



/* Sensor Configuration Variables */
#define tt100_pin 5
//DHTesp tt100;
DataPoint tt100_data;
int tt100_status = 0;
bool tt100_fetching = false;


//#define tt101_pin 18
//DHTesp tt101;
//bool tt101_fetching = false;


/* CPU Task Variables */
DHTesp dht;
/** Task handle for the light value read task */
TaskHandle_t tempTaskHandle = NULL;
/** Ticker for temperature reading */
Ticker tempTicker;
/** Comfort profile */
ComfortState cf;
/** Flag if task should run */
bool tasksEnabled = false;
int dhtPin = tt100_pin;

void tempTask(void *pvParameters);
bool getTemperature();
void triggerGetTemp();



/* Sensor Functions */
int getTemperature(DHTesp &dht, DataPoint &datapoint);

bool initReadSensors();



short sim_batt = -12;
bool sim_usedp = false;

/* SIMULATION (just write random values) */
void simulate() {
  sim_batt = random(-50, 100); // negative means no battery detected
  sim_usedp = (random(0, 10) > 5); // If humidity should be RH or dew point
}

uint next_refresh = 0;

void setup() {
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, true /*Serial Enable*/);
  initializeGWDisplay();
  //initializeGWwifi();



  delay(2500);
  initTemp();
  // Signal end of setup() to tasks
  tasksEnabled = true;

  //initReadSensors();
  //Serial.print("Done initReadSensors.");


  delay(2500);
  //readingTaskEnabled = true;
  Serial.print("Done Setup.");
}

void loop() {

  if (millis() > next_refresh) {
    next_refresh = millis() + 3000;

    simulate(); // refresh values with random data
    //sendSystemStatusToInflux();
    /* 
    *REFRESH DISPLAY
    * clear and re-write the display to show new data 
    */
    Heltec.display->clear();

    drawWifiStatus();
    drawBattery(sim_batt);
    DataPoint dp = tt100_data;
    drawTemperatureStatus(0, 16, dp);
    if (sim_usedp) {
      drawHumidityStatus(0, 40, USE_DEWPOINT,dp);
    } else {
      drawHumidityStatus(0, 40,dp);
    }
      
    

    
    Heltec.display->display();
  }

  //yeild();
}






// /* Sensor Functions */
// int getTemperature(DHTesp &dht, DataPoint &datapoint) {
//     TempAndHumidity newValues = dht.getTempAndHumidity();
    
//     // Check if any reads failed and exit early (to try again).
//     if (dht.getStatus() != 0) {
//         Serial.println("DHT11 error status: " + String(dht.getStatusString()));
//         return dht.getStatus();
//     }

//     Serial.println("Got T: " + String(newValues.temperature,1));
//     Serial.println("Got H: " + String(newValues.humidity,0));
//     float dewpoint = dht.computeDewPoint(newValues.temperature, newValues.humidity);
//     datapoint.temperature = newValues.temperature;
//     datapoint.humidity = newValues.humidity;
//     datapoint.dewpoint = dewpoint;
//     //GwSensors.sendDataToInflux(i);
//     return 0;
// }



// /* CPU Task Functions */
// void wakeCPUCore() {
//   if (readSensorTaskHandle != NULL) { // if tempTask exists (was started)
//     Serial.println("Resuming readSensorTaskHandle");
// 	   xTaskResumeFromISR(readSensorTaskHandle);
//   }
// }

// void taskReadOnSecondCore(void *pvParameters) {
// 	Serial.println("tempTask loop started");
// 	while (1) { // tempTask loop
//     if (readingTaskEnabled) {
// 	    Serial.println("New Temp needed, not yet sleeping.");
//       Serial.println("Getting temp from TT-100...");
//       tt100_fetching = true;
//       tt100_status = getTemperature(tt100, tt100_data);
//       tt100_fetching = false;
//       Serial.println("done.");
//     }
//   }
//   // Go to sleep again
//   Serial.println("Nothing more to do, CPU 1 going to sleep.");
//   vTaskSuspend(NULL);
// }


void tempTask(void *pvParameters) {
	Serial.println("tempTask loop started");
	while (1) // tempTask loop
  {
    if (tasksEnabled) {
      // Get temperature values
			getTemperature();
		}
    // Got sleep again
		vTaskSuspend(NULL);
	}
}

void triggerGetTemp() {
  if (tempTaskHandle != NULL) {
	   xTaskResumeFromISR(tempTaskHandle);
  }
}

bool initTemp() {
  byte resultValue = 0;
  // Initialize temperature sensor
	dht.setup(dhtPin, DHTesp::DHT11);
	Serial.println("DHT initiated");

  // Start task to get temperature
	xTaskCreatePinnedToCore(
			tempTask,                       /* Function to implement the task */
			"tempTask ",                    /* Name of the task */
			4000,                           /* Stack size in words */
			NULL,                           /* Task input parameter */
			5,                              /* Priority of the task */
			&tempTaskHandle,                /* Task handle. */
			1);                             /* Core where the task should run */

  if (tempTaskHandle == NULL) {
    Serial.println("Failed to start task for temperature update");
    return false;
  } else {
    // Start update of environment data every 20 seconds
    tempTicker.attach(20, triggerGetTemp);
  }
  return true;
}


bool getTemperature() {
	// Reading temperature for humidity takes about 250 milliseconds!
	// Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
  TempAndHumidity newValues = dht.getTempAndHumidity();
	// Check if any reads failed and exit early (to try again).
	if (dht.getStatus() != 0) {
		Serial.println("DHT11 error status: " + String(dht.getStatusString()));
		return false;
	}

	float heatIndex = dht.computeHeatIndex(newValues.temperature, newValues.humidity);
  float dewPoint = dht.computeDewPoint(newValues.temperature, newValues.humidity);

  

  Serial.println(" T:" + String(newValues.temperature) + " H:" + String(newValues.humidity) + " I:" + String(heatIndex) + " D:" + String(dewPoint));
	return true;
}


// bool initReadSensors() {
// 	tt100.setup(tt100_pin, DHTesp::DHT11);
// 	Serial.println("DHT initiated");

//   // Setup CPU1 to run the sensor reading task
// 	xTaskCreatePinnedToCore(
// 			taskReadOnSecondCore,           /* Function to implement the task */
// 			"temperatureTask ",             /* Name of the task */
// 			4000,                           /* Stack size in words */
// 			NULL,                           /* Task input parameter */
// 			5,                              /* Priority of the task */
// 			&readSensorTaskHandle,          /* Task handle. */
// 			1);                             /* Core where the task should run */  

//   Serial.println("Done xTaskCreatePinnedToCore");
      
//   if (readSensorTaskHandle == NULL) {
//     Serial.println("Failed to start task for temperature update");
//     return false;
//   } else {
//     Serial.println("Setting Ticker...");
//     // Use wakeCPUCore to trigger interrupt that resumes excecution
//     updateSensorsTicker.attach(15, wakeCPUCore);
//     Serial.println("Ticker set.");
//     return true;
//   }
// }