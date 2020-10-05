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
#define CHK_ALARMS_TASK 8
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


/*
 * ALARMING
 * Some simple alarming action. Nothing fancy
 * this should be improved. Also manually save  
 *  and load alarm data to RTC memory.
 */
void checkAlarmActions(){
  if( TT100.alarm.state == ALARM || 
      TT101.alarm.state == ALARM   ) {
        digitalWrite(13, HIGH);
  } else {
        digitalWrite(13, LOW);
  }

  if( MT200.alarm.state == ALARM || 
      MT201.alarm.state == ALARM   ) {
        digitalWrite(12, HIGH);
  } else {
        digitalWrite(12, LOW);
  }

}

/* We will try to save and restore alarm SP, 
 * State, Data, Output through deepsleeps using RTC
 */
RTC_DATA_ATTR persist_alm rtcTT100;
RTC_DATA_ATTR persist_alm rtcTT101;
RTC_DATA_ATTR persist_alm rtcMT200;
RTC_DATA_ATTR persist_alm rtcMT201;
void loadAlarmData(){

  MT200.configureAlarm(gt, 60);
  MT201.configureAlarm(gt, 65);
  TT100.configureAlarm(lt, 26);
  TT101.configureAlarm(lt, 20);

  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);

  if ( esp_reset_reason() == ESP_RST_DEEPSLEEP) {
    Serial.println("Looks like we just woke up from sleep. Try to load alarm data");
    if(TT100.alarm.in_use) TT100.alarm.loadFromRTC(rtcTT100);
    if(TT101.alarm.in_use) TT101.alarm.loadFromRTC(rtcTT101);
    if(MT200.alarm.in_use) MT200.alarm.loadFromRTC(rtcMT200);
    if(MT201.alarm.in_use) MT201.alarm.loadFromRTC(rtcMT201);
    
    checkAlarmActions();
    gpio_hold_dis(GPIO_NUM_12);
    gpio_hold_dis(GPIO_NUM_13);
  }
  
}
void saveAlarmData(){
  if(TT100.alarm.in_use) TT100.alarm.copyToRTC(rtcTT100);
  if(TT101.alarm.in_use) TT101.alarm.copyToRTC(rtcTT101);
  if(MT200.alarm.in_use) MT200.alarm.copyToRTC(rtcMT200);
  if(MT201.alarm.in_use) MT201.alarm.copyToRTC(rtcMT201);
}
/**********************/




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

  //backup alarm states to RTC
  saveAlarmData();

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
  loadAlarmData();
  initSleep();
  initTouchInput();
  delay(1500);
  

  /* Schedule all the Application tasks */
  ts.add(SCROLL_TASK,       3200, [&](void *) { scrollDisplay();       }, nullptr, false);
  ts.add(LOG_WIFI_TASK,     6000, [&](void *) { logWifiStatus();       }, nullptr, false);
  ts.add(READ_DHT_TASK,     2000, [&](void *) { readDhtSensors();      }, nullptr, false);
  ts.add(LOG_DHT_TASK,      3500, [&](void *) { queueSensorDataLogs(); }, nullptr, false);
  ts.add(CHK_ALARMS_TASK,   2500, [&](void *) { checkAlarmActions();   }, nullptr, true );
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

