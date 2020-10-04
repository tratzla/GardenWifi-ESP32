#include "Arduino.h"
#include "heltec.h"
#include "GW-display.h"
#include "GW-sensors.h"
#include "GW-wifi.h"
#include "GW-influx.h"
#include "GW-datalog.h"
#include "TickerScheduler.h"
#include "battery.h"
#include "GW-sleep.h"
#include "GW-touch.h"


#define SCROLL_TASK 0
#define LOG_WIFI_TASK 1
#define READ_DHT_TASK 2
#define LOG_DHT_TASK 3
#define READLOG_BATT_TASK 4
#define DEEP_SLEEP_TASK 5
#define CHK_TOUCH_TASK 6
#define READLOG_MOIST_TASK 7
#define RETURN_TO_AUTOSCROLL_TASK 10
#define MANUAL_FASTSCROLL_TASK 11
TickerScheduler ts(12);



long timeForAutoScroll;




/* Task that's scheduled to make screen choices */
void scrollDisplay() {
  if(timeForAutoScroll > millis()) {
    ts.disable(MANUAL_FASTSCROLL_TASK);
    autoScroll = true;
    manScrollNext = false;
  }
  if (!autoScroll && !manScrollNext) return;
  
  refreshDataDisplay();
  manScrollNext = false;
}


void checkTouchInput() {
  if (!pin27Touched) return;

  pin27Touched = false;
  naptime = millis() + TIME_TO_WAKE_MS;

  if(autoScroll && manScrollNext) {
    autoScroll = false;
    manScrollNext = true;

    timeForAutoScroll = millis() + RETURN_TO_AUTOSCROLL;
    ts.enable(MANUAL_FASTSCROLL_TASK);
  }
}

void getReadyForSleep(){
  if(shutdownFlag) goToDeepSleep(); // If we've already prepped for sleep, actually do it now

  now = millis();
  if(naptime > now) return; // Not naptime yet

  Serial.printf("\nNaptime:%d > now:%d    Time to Sleep!\n", naptime, now);

  Serial.println("Stopping all tasks: ");
  ts.disableAll();
  shutdownFlag = true;

  ts.enable(SCROLL_TASK); //Now with shutdownFlag, this displays warning

  Serial.println("Flushing datalog buffer: ");
  flushInfluxBuffer();

  // Serial.println("Stopping Wifi: ");
  //Wifi.end();

  // Re-enable this task so we give another 5s for anything else to finish
  ts.enable(DEEP_SLEEP_TASK);
}


void setup() {
  /* Check the reason for wakeup */
  print_wakeup_reason();

  /* Initialize manufacturer libraries */
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, true /*Serial Enable*/);

  /* Initialize Application parts */
  initializeGWDisplay();
  initScreenSwitcher();
  initializeGWwifi(); // This will also initialize influx if wifi connection is succesful
  initBatteryMonitor();
  initSensors();
  initSleep();
  initTouchInput();
  delay(1500);
  

  /* Schedule all the Application tasks */
  ts.add(SCROLL_TASK,       3200, [&](void *) { scrollDisplay();       }, nullptr, false);
  ts.add(LOG_WIFI_TASK,     6000, [&](void *) { logWifiStatus();       }, nullptr, false);
  ts.add(READ_DHT_TASK,     2000, [&](void *) { readDhtSensors();      }, nullptr, false);
  ts.add(LOG_DHT_TASK,      3500, [&](void *) { queueSensorDataLogs(); }, nullptr, false);
  ts.add(READLOG_BATT_TASK, 3600, [&](void *) { readLogBattery();      }, nullptr, true );
  ts.add(DEEP_SLEEP_TASK,   6000, [&](void *) { getReadyForSleep();    }, nullptr, false);
  ts.add(CHK_TOUCH_TASK,     250, [&](void *) { checkTouchInput();     }, nullptr, false);
  ts.add(READLOG_MOIST_TASK,2200, [&](void *) { getAllMoistures();     }, nullptr, true );

  ts.add(MANUAL_FASTSCROLL_TASK, 350, [&](void *) { scrollDisplay(); }, nullptr, true);
  ts.disable(MANUAL_FASTSCROLL_TASK);

  delay(2500);

  readingTaskEnabled = true;
  Serial.println("Done Setup!\n");
}

void loop() {


  ts.update();
}

