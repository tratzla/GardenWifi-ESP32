#ifndef GW_DISPLAY
#define GW_DISPLAY

#include "Arduino.h"
#include "heltec.h"
#include <WiFiMulti.h>
#include "icons.h"
#include "font_dialog.h"
#include "datapoint.h"
#include "GW-readsensortask.h"
#include "battery.h"

#define screen_width 128
#define screen_height 64

#define font_value DialogInput_plain_20
#define font_units Dialog_plain_14
#define font_dew ArialMT_Plain_10
#define font_ipaddr ArialMT_Plain_10

#define icon_offset 14
#define units_offset -8
#define dew_offset -2

bool USE_DEWPOINT = true;


void displayBattDebug(){
  Heltec.display->clear();
  Heltec.display->setFont(Dialog_plain_14);

  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  String toPrint = String("  Trent's: ") + String(meth1) + String(" mV");
  Heltec.display->drawString(0, 0, toPrint);

  String toPrint2 = String("  Heltec's: ") + String(meth2) + String(" mV");
  Heltec.display->drawString(0, 16, toPrint2);


  Heltec.display->setFont(ArialMT_Plain_10);
  toPrint = String(" RAW: ") + String(raw_counts);
  Heltec.display->drawString(0, 50, toPrint);

  Heltec.display->display();

}

void drawBattery(int8_t batt) {
  if        (batt > 100) {
    Heltec.display->drawXbm(100, 0, battery_level_w, battery_level_h, battery_nobatt_16);
  } else if (batt > 80)  {
    Heltec.display->drawXbm(100, 0, battery_level_w, battery_level_h, battery_fourbars_16);
  } else if (batt > 60 ) {
    Heltec.display->drawXbm(100, 0, battery_level_w, battery_level_h, battery_threebars_16);    
  } else if (batt > 40 ) {
    Heltec.display->drawXbm(100, 0, battery_level_w, battery_level_h, battery_twobars_16);    
  } else if (batt > 20 ) {
    Heltec.display->drawXbm(100, 0, battery_level_w, battery_level_h, battery_onebars_16);    
  } else if (batt > 0 )  {
    Heltec.display->drawXbm(100, 0, battery_level_w, battery_level_h, battery_zerobars_16);    
  } else                 {
    Heltec.display->drawXbm(100, 0, battery_level_w, battery_level_h, battery_nobatt_16);    
  }
}

void drawSignalbars(int8_t sig) {
  if (sig > -60) {
    Heltec.display->drawXbm(0, 0, signal_bars_w, signal_bars_h, signal_fourbars_16);
  } else if (sig > -65 ) {
    Heltec.display->drawXbm(0, 0, signal_bars_w, signal_bars_h, signal_threebars_16);    
  } else if (sig > -67 ) {
    Heltec.display->drawXbm(0, 0, signal_bars_w, signal_bars_h, signal_twobars_16);    
  } else if (sig > -70 ) {
    Heltec.display->drawXbm(0, 0, signal_bars_w, signal_bars_h, signal_onebars_16);    
  } else if (sig > -80 ) {
    Heltec.display->drawXbm(0, 0, signal_bars_w, signal_bars_h, signal_zerobars_16);    
  } else {
    Heltec.display->drawXbm(0, 0, signal_bars_w, signal_bars_h, signal_zerobarsx_16);    
  }
}

void drawTempIcon(bool busy) {
  uint x = screen_width/2 + icon_offset;
  uint y = 16; // TODO this needs to be a config/member
  if (busy) {
    Heltec.display->drawXbm(x, y, thermometer_24_w, thermometer_24_h, thermometer_wait_24);
  } else {
    Heltec.display->drawXbm(x, y, thermometer_24_w, thermometer_24_h, thermometer_24);
  }
}

void drawTemperatureStatus(uint startx, uint starty, DataPoint datapoint){
  String toPrint = String("");
  String unit = String("°C");
  uint x = startx;
  uint y = starty;

  //Write value itself
  if (datapoint.timestamp == 0) {
    toPrint = String("---");
  } else {
    toPrint = String(abs(datapoint.temperature), 1);
  }
  Heltec.display->setFont(font_value);
  Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
  Heltec.display->drawString(screen_width/4, y, toPrint);
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);

  /* Format value strings */
  // Write negative sign separate
  if (datapoint.temperature < 0 ) {
    Heltec.display->setFont(font_value);
    Heltec.display->drawString(x, y, String("-"));
  }

  /* Units */
  x = screen_width/2 + units_offset;
  Heltec.display->setFont(font_units);
  Heltec.display->drawString(x, y, unit);
}


void drawHumidityIcon(bool busy) {
  uint x = screen_width/2 + icon_offset;
  uint y = 40; // TODO this needs to be a config/member
  if (busy) {
    Heltec.display->drawXbm(x, y, drop_24_w, drop_24_h, drop_wait_24);
  } else {
    Heltec.display->drawXbm(x, y, drop_24_w, drop_24_h, drop_24);
  }
}

void drawHumidityStatus(uint startx, uint starty, bool use_dewpoint, DataPoint datapoint) {
  String toPrint = String("");
  String unit = String("%");
  uint x = startx;
  uint y = starty;

  /* print value */
  float value = use_dewpoint ? datapoint.dewpoint : datapoint.humidity;
  if (datapoint.timestamp == 0) {
    toPrint = String("---");
  } else {    
    toPrint = String(abs(value), 1);
  }

  if (value < 0 ) {
    Heltec.display->setFont(font_value);
    Heltec.display->drawString(x, y, String("-"));
  }
  
  Heltec.display->setFont(font_value);
  Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
  Heltec.display->drawString(screen_width/4, y, toPrint);
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);


  /* Units */
  x = screen_width/2 - 5;
  if ( use_dewpoint ) {
    unit = String("°C");
    // Also print extra 'dew' text below unit
    Heltec.display->setFont(font_dew);
    Heltec.display->drawString(x + dew_offset, y+10, "dew");
  }
  Heltec.display->setFont(font_units);
  Heltec.display->drawString(x, y, unit);
}

// Humidity function for printing in RELATIVE HUMIDITY
void drawHumidityStatus(uint startx, uint starty, DataPoint datapoint){
  drawHumidityStatus(startx, starty, !USE_DEWPOINT, datapoint);
}

void drawWifiStatus() {
  if (WiFi.status() == WL_CONNECTED) {
    drawSignalbars(WiFi.RSSI());
    // Heltec.display->setFont(font_ipaddr);
    // Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
    // Heltec.display->drawString(screen_width/2, 0, WiFi.localIP().toString());
  } else {
    drawSignalbars(-100);
  }
}

void drawWifiSDetails() {
    uint wifiCentre = screen_width/2 - 16;
    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
    Heltec.display->drawString(screen_width/2, 0, "WiFi");

    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->setTextAlignment(TEXT_ALIGN_RIGHT);
    Heltec.display->drawString(wifiCentre, 16, "IP:");
    Heltec.display->setFont(Dialog_plain_14);
    Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
    Heltec.display->drawString(wifiCentre, 16, WiFi.localIP().toString());

    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->setTextAlignment(TEXT_ALIGN_RIGHT);
    Heltec.display->drawString(wifiCentre, 30, "Gateway:");
    Heltec.display->setFont(Dialog_plain_14);
    Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
    Heltec.display->drawString(wifiCentre, 30, WiFi.gatewayIP().toString());

    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->setTextAlignment(TEXT_ALIGN_RIGHT);
    Heltec.display->drawString(wifiCentre, 44, "AP:");
    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
    Heltec.display->drawString(wifiCentre, 44, WiFi.SSID());
}


void initializeGWDisplay(){
  Heltec.display->flipScreenVertically();
  Heltec.display->setFont(ArialMT_Plain_10);
}

#endif