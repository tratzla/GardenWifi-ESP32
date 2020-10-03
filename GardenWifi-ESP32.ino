#include "Arduino.h"
#include "heltec.h"
#include "GW-display.h"
#include "GW-wifi.h"
#include "GW-influx.h"
#include "GW-readsensortask.h"
#include "datapoint.h"
#include "TickerScheduler.h"

short sim_batt = -12;
bool sim_usedp = false;


#define SCREEN_TT100_RH 1
#define SCREEN_TT100_DP 2
#define SCREEN_TT101_RH 3
#define SCREEN_TT101_DP 4
#define SCREEN_WIFI 5

#define SCREEN_END 6


/* SIMULATION (just write random values) */
void simulate() {
  sim_batt = random(-50, 100); // negative means no battery detected
  sim_usedp = (random(0, 10) > 5); // If humidity should be RH or dew point
}

uint next_refresh = 0;
TickerScheduler ts(5);
//Ticker displayTicker; /** Ticker(timer) to periodically refresh display */
//Ticker refreshWifiTicker; /** Ticker(timer) to periodically update wifi system status */
bool flagDisplayRefresh = false;
bool flagWifiRefresh = false;

void refreshDataDisplay() {
  /* 
  *REFRESH DISPLAY
  * clear and re-write the display to show new data 
  */
  static uint screenSequence = 1;

  simulate(); // refresh values with random data
  Heltec.display->clear();

  drawWifiStatus();
  drawBattery(sim_batt);
  
  if (screenSequence == SCREEN_TT100_RH || screenSequence == SCREEN_TT100_DP) {

    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
    Heltec.display->drawString(screen_width/2, 0, "TT-100");

    drawTemperatureStatus(0, 16, TT100.getData());
    drawTempIcon(TT100.fetching);

    if (screenSequence == SCREEN_TT100_DP) {
      drawHumidityStatus(0, 40, USE_DEWPOINT, TT100.getData());
    } else {
      drawHumidityStatus(0, 40, TT100.getData());
    }
    drawHumidityIcon(TT100.fetching);
  }
  
  if (screenSequence == SCREEN_TT101_RH || screenSequence == SCREEN_TT101_DP) {
    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
    Heltec.display->drawString(screen_width/2, 0, "TT-101");
    drawTemperatureStatus(0, 16, TT101.getData());
    drawTempIcon(TT101.fetching);

    if (screenSequence == SCREEN_TT101_DP) {
      drawHumidityStatus(0, 40, USE_DEWPOINT, TT101.getData());
    } else {
      drawHumidityStatus(0, 40, TT101.getData());
    }
      drawHumidityIcon(TT101.fetching);
  }

  if (screenSequence == SCREEN_WIFI) {
    drawWifiSDetails();
  }

  Heltec.display->display();

  screenSequence++;
  if (screenSequence == SCREEN_END) 
      screenSequence = 1;
}



void refreshWifiStatus() {
  sendSystemStatusToInflux();
}



void setup() {
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, true /*Serial Enable*/);
  initializeGWDisplay();
  initializeGWwifi(); // This will also initialize influx if wifi connection is succesful

  delay(1500);
  initReadSensors();
  
  ts.add(0,  3000, [&](void *) { refreshDataDisplay(); }, nullptr, false);
  ts.add(1, 30000, [&](void *) { sendSystemStatusToInflux(); }, nullptr, false);
  ts.add(2, 10000, [&](void *) { wakeCPUCore(); }, nullptr, false);
  ts.add(3, 11000, [&](void *) { sendSensorDataToInflux(TT100); }, nullptr, false);
  ts.add(4, 12000, [&](void *) { sendSensorDataToInflux(TT101); }, nullptr, false);
  delay(2500);
  readingTaskEnabled = true;
  Serial.println("Done Setup!\n");
}

void loop() {
  

  ts.update();
}

