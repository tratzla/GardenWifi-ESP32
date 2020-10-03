#include "Arduino.h"
#include "heltec.h"
#include "GW-sensors.h"
#include "GW-display.h"
#include "GW-wifi.h"
#include "GW-influx.h"
//#include "GW-readsensortask.h"
#include "GW-datalog.h"
#include "TickerScheduler.h"
#include "battery.h"
#include "GW-sleep.h"
#include "GW-touch.h"



#define SCREEN_TT100_RH 1
#define SCREEN_TT100_DP 2
#define SCREEN_TT101_RH 3
#define SCREEN_TT101_DP 4
#define SCREEN_WIFI 5

#define SCREEN_END 6




#define SCROLL_TASK 0
#define LOG_WIFI_TASK 1
#define READ_DHT_TASK 2
#define LOG_DHT_TASK 3
#define READLOG_BATT_TASK 4
#define DEEP_SLEEP_TASK 5
#define CHK_TOUCH_TASK 6
#define RETURN_TO_AUTOSCROLL_TASK 10
#define MANUAL_FASTSCROLL_TASK 11
TickerScheduler ts(12);



#define RETURN_TO_AUTOSCROLL 15000
bool autoScroll = true;
bool manScrollNext = false;


void goToDeepSleep();

void startAutoScroll() {
  ts.disable(MANUAL_FASTSCROLL_TASK);
  ts.add(DEEP_SLEEP_TASK, 75000, [&](void *) { goToDeepSleep();}, nullptr, false);
  ts.remove(RETURN_TO_AUTOSCROLL_TASK);
  // ts.enable(DEEP_SLEEP_TASK);
  autoScroll = true;
  manScrollNext = false;
}



void scrollDisplay(){
  if (!autoScroll && !manScrollNext) return;
  if (manScrollNext) {
    ts.remove(RETURN_TO_AUTOSCROLL_TASK);
  }
  refreshDataDisplay();

  if (manScrollNext) {
    ts.add(RETURN_TO_AUTOSCROLL_TASK, 10000, [&](void *) { startAutoScroll();}, nullptr, false);
    manScrollNext = false;
  }
}

void refreshDataDisplay() {
  /* 
  *REFRESH DISPLAY
  * clear and re-write the display to show new data 
  */
  static uint screenSequence = 1;

  //simulate(); // refresh values with random data
  Heltec.display->clear();

  drawWifiStatus();
  drawBattery(battPercent);
  
  if (screenSequence == SCREEN_TT100_RH || screenSequence == SCREEN_TT100_DP) {

    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
    Heltec.display->drawString(screen_width/2, 0, "TT-100");
    Heltec.display->drawString(screen_width/4, 0, autoScroll?" A":" M");

    drawTemperatureStatus(0, 16, TT100.getData());
    drawTempIcon(TT100.state != SENSOR_IDLE);

    if (screenSequence == SCREEN_TT100_DP) {
      drawHumidityStatus(0, 40, USE_DEWPOINT, TT100.getData());
    } else {
      drawHumidityStatus(0, 40, TT100.getData());
    }
    drawHumidityIcon(TT100.state != SENSOR_IDLE);
  }
  
  if (screenSequence == SCREEN_TT101_RH || screenSequence == SCREEN_TT101_DP) {
    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
    Heltec.display->drawString(screen_width/2, 0, "TT-101");
    Heltec.display->drawString(screen_width/4, 0, autoScroll?" A":" M");
    drawTemperatureStatus(0, 16, TT101.getData());
    drawTempIcon(TT101.state != SENSOR_IDLE);

    if (screenSequence == SCREEN_TT101_DP) {
      drawHumidityStatus(0, 40, USE_DEWPOINT, TT101.getData());
    } else {
      drawHumidityStatus(0, 40, TT101.getData());
    }
      drawHumidityIcon(TT101.state != SENSOR_IDLE);
  }

  if (screenSequence == SCREEN_WIFI) {
    drawWifiSDetails();
  }

  Heltec.display->display();

  screenSequence++;
  if (screenSequence == SCREEN_END) 
      screenSequence = 1;
}








void setup() {
  /* Check the reason for wakeup */
  print_wakeup_reason();

  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, true /*Serial Enable*/);


  initializeGWDisplay();
  initializeGWwifi(); // This will also initialize influx if wifi connection is succesful
  initBatteryMonitor();
  initSensors();
  initSleep();
  initTouchInput();

  delay(1500);



  delay(1500);
  autoScroll = true;

  ts.add(SCROLL_TASK,       3200, [&](void *) { scrollDisplay(); }, nullptr, false);
  ts.add(LOG_WIFI_TASK,     6000, [&](void *) { logWifiStatus(); }, nullptr, false);
  ts.add(READ_DHT_TASK,     2000, [&](void *) { readDhtSensors(); }, nullptr, false);
  ts.add(LOG_DHT_TASK,      3500, [&](void *) { queueSensorDataLogs(); }, nullptr, false);
  ts.add(READLOG_BATT_TASK, 3600, [&](void *) { readLogBattery();}, nullptr, true);
  ts.add(DEEP_SLEEP_TASK,   1000, [&](void *) { goToDeepSleep();}, nullptr, false);
  ts.add(CHK_TOUCH_TASK,     250, [&](void *) { checkTouchInput();}, nullptr, false);



  // ts.add(8,  1000, [&](void *) { printTouchVal();}, nullptr, false);
  //ts.add(RETURN_TO_AUTOSCROLL_TASK, 10000, [&](void *) { startAutoScroll();}, nullptr, true);
  ts.add(MANUAL_FASTSCROLL_TASK, 350, [&](void *) { scrollDisplay(); }, nullptr, true);

  ts.disable(MANUAL_FASTSCROLL_TASK);
  //ts.disable(RETURN_TO_AUTOSCROLL_TASK);

  delay(2500);

  readingTaskEnabled = true;
  Serial.println("Done Setup!\n");
}

void loop() {


  ts.update();
}

