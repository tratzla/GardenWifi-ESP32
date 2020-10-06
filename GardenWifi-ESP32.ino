#ifdef CONFIG_ARDUHAL_ESP_LOG
  #undef CONFIG_ARDUHAL_ESP_LOG
  #define CONFIG_ARDUHAL_ESP_LOG (4)
#endif


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
#include "GW-menu.h"


// #define _dbg_en 1
// #include "GW-header.h"


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

static const char* TAG = "Main";

TickerScheduler ts(12);
long timeForAutoScroll;
 
#define SCREEN_SET_NORMAL 1
#define SCREEN_SET_MENU 2

byte screenSet = SCREEN_SET_NORMAL;



#define TOUCH_INPUT_SELECT_PIN 27 // GPIO27 aka TOUCH6
#define TOUCH_INPUT_CHANGE_PIN 14 // GPIO14 aka TOUCH7

uint TOUCH_THRESHOLD = 25;
bool inputSelTouched;
bool inputChgTouched;

void inputSelect_cb() {
  inputSelTouched = true;
}

void inputChange_cb() {
  inputChgTouched = true;
}

void initTouchInput() {
  inputSelTouched = false;
  inputChgTouched = false;
  touchAttachInterrupt(TOUCH_INPUT_SELECT_PIN, inputSelect_cb, TOUCH_THRESHOLD);
  touchAttachInterrupt(TOUCH_INPUT_CHANGE_PIN, inputChange_cb, TOUCH_THRESHOLD);
}

int printTouchVal(int pin) {
  int val = touchRead(pin);
  log_i("Touched PIN#%d: %d \n\n",pin, val);
  return val;
}

void readUserControls(){
  // log_i(" menuCTL=%d, inSel=%d  inChg=%d\n\n ",uiControl,inputSelTouched,inputChgTouched );
  
  naptime = millis() + TIME_AWAKE * SEC_TO_MILLI_SEC;
  timeForAutoScroll = millis() + RETURN_TO_AUTOSCROLL;

  /* Reset autoscroll settings since we still have user interaction */
  if(autoScroll) {
      autoScroll = false;
      ts.disable(SCROLL_TASK);
    }

  /* Change screenset if necessary */
  if ( screenSet==SCREEN_SET_NORMAL && uiControl==UI_CTL_BOTH ) {
    screenSet = SCREEN_SET_MENU;
    uiControl = 0;
  }

  Heltec.display->displayOn();
  scrollDisplay();
}


void checkTouchInput() {

  static bool prev_inputSelTouched = false;
  static bool prev_inputChgTouched = false;

  
  if(inputSelTouched && inputChgTouched) {
    if (!(prev_inputChgTouched && prev_inputSelTouched) ) {
      uiControl = UI_CTL_BOTH;
      readUserControls();
    }
    inputChgTouched = touchRead(TOUCH_INPUT_CHANGE_PIN) < TOUCH_THRESHOLD;
    inputSelTouched = touchRead(TOUCH_INPUT_SELECT_PIN) < TOUCH_THRESHOLD;
    prev_inputSelTouched = inputSelTouched;
    prev_inputChgTouched = inputChgTouched;
  }

  if(inputSelTouched) {
    if (!prev_inputSelTouched) {
      uiControl = UI_CTL_SELECT;
      readUserControls();
    }
    inputSelTouched = touchRead(TOUCH_INPUT_SELECT_PIN) < TOUCH_THRESHOLD;
    prev_inputSelTouched = inputSelTouched;
  }

  if(inputChgTouched) {
    if (!prev_inputChgTouched) {
      uiControl = UI_CTL_CHANGE;
      readUserControls();
    }
    inputChgTouched = touchRead(TOUCH_INPUT_CHANGE_PIN) < TOUCH_THRESHOLD;
    prev_inputChgTouched = inputChgTouched;
  }
  
}


/* Task that's scheduled to make screen choices */
void scrollDisplay() {
  ulong now = millis();
  if( now > timeForAutoScroll) {
    ts.enable(SCROLL_TASK);
    autoScroll = true;
    screenSet = SCREEN_SET_NORMAL;
  }


  if (screenSet==SCREEN_SET_NORMAL) {
    // if (!autoScroll && uiControl&UI_CTL_SELECT==0) {return;}
    if (autoScroll) {
      refreshDataDisplay();
    } else if (uiControl&UI_CTL_SELECT) {
      refreshDataDisplay();
      uiControl &= ~UI_CTL_SELECT;
    }
    uiControl &= ~UI_CTL_CHANGE; // Actually we don't do anythign yet with CHANGE in NORMAL
  }
  
  if (screenSet==SCREEN_SET_MENU) {
    refreshMenuDisplay();
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
    log_i("Looks like we just woke up from sleep. Try to load alarm data");
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
  if(shutdownFlags) 
    goToDeepSleep(); // If we've already prepped for sleep, actually do it now

  now = millis();
  if(naptime > now) return; // Not naptime yet

  log_i("\nNaptime:%d > now:%d    Time to Sleep!\n", naptime, now);

  log_i("Stopping all tasks: ");
  ts.disableAll();
  shutdownFlags |= SHTDN_SLEEP_NOW;

  ts.enable(SCROLL_TASK); //Now with shutdownFlag, this displays warning

  log_i("Flushing datalog buffer: ");
  flushInfluxBuffer();

  log_i("Stopping Wifi: ");
  //Wifi.end();

  //backup alarm states to RTC
  saveAlarmData();

  // Re-enable this task so we give another 5s for anything else to finish
  ts.enable(DEEP_SLEEP_TASK);
}


void initializeGWDisplay(){
  if(esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_TOUCHPAD) {
    Heltec.display->displayOff();
  }

  uiControl = 0;
  autoScroll = true;
  screenSet = SCREEN_SET_NORMAL;
}

void setup() {
  /* Initialize manufacturer libraries */
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, true /*Serial Enable*/);

  /* Check the reason for wakeup */
  print_wakeup_reason();

  if(esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_TOUCHPAD) {
    log_i("Someone Woke me through Touch!");
  }

  /* Initialize Application parts */
  initializeGWDisplay();
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

  delay(500);
  readingTaskEnabled = true;
  log_i("Done Setup!\n");
}

void loop() {

  ts.update();

}