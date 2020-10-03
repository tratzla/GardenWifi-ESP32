/*
 * From Heltech Example
 * https://github.com/HelTecAutomation/Heltec_ESP32/blob/master/examples/ESP32/ADC_Read_Voltage/Battery_power/Battery_power.ino
 *
 * and also this random guy
 * https://github.com/3KUdelta/heltec_wifi_kit_32_batt_monitor/blob/master/Wifi_Kit_32_Batt_Watch.ino
 *
 * and also this dude from a forum
 * http://community.heltec.cn/t/heltec-wifi-lora-v2-battery-management/147/35
 */
 

#ifndef GW_BATTERY
#define GW_BATTERY

#include <Arduino.h>
#include <WiFiMulti.h>
#include "GW-influx.h"

#define MAXBATT 4200    // The default Lipo is 4200mv when the battery is fully charged.
#define MINBATT 3200    // The default Lipo is 3200mv when the battery is empty...this WILL be low on the 3.3v rail specs!
#define VOLTAGE_DIVIDER 3.20    // Board has 220k/100k voltage divider
#define VBATT_SAMPLE 500     // Battery sample rate in ms
#define VBATT_SMOOTH 50      // Number of averages in sample
#define ADC_READ_STABILIZE 5       // in ms (delay from GPIO control and ADC connections times)
#define VEXT_GPIO 21      // Heltec GPIO to toggle Vext off and allow VBatt read connection
#define VBATT_GPIO 37 // Heltec GPIO for reading Vbatt on wifi kit 32


#define FUDGE_OFFSET -475
Point battStatusInflux("Battery_Status");
float battVoltage = 0;
int   battPercent = 0;


int calcBattPercent(float v){
  int p = 0;
  // Low discharge, say <0.6c
  if (v>3.40) p = 10;
  if (v>3.60) p = 20;
  if (v>3.65) p = 30;
  if (v>3.70) p = 40;
  if (v>3.75) p = 50;
  if (v>3.80) p = 60;
  if (v>3.85) p = 70;
  if (v>3.90) p = 80;
  if (v>4.05) p = 90;
  if (v>4.10) p = 100;
  if (v>4.15) p = 101;
  return p;
}


int meth1 = 0;
int meth2 = 0;
int raw_counts = 0;

float readBattLevel() { 
  static uint8_t idx = 0;
  static uint16_t readings[VBATT_SMOOTH];
  static int32_t total = 0;
  static bool f = true; // first run flag
  if(f){ for(uint8_t c=0;c<VBATT_SMOOTH;c++){ readings[c]=0; } f=false; }   // Initialize the samples array first run
  total = total - readings[idx];  // subtract the last reading to get ready for new reading
  
  //adcStart(VBATT_GPIO);
  //while(adcBusy(VBATT_GPIO));
  uint16_t newreading = analogRead(VBATT_GPIO); // read from the analog battery pin
  raw_counts = newreading;
  //adcEnd(37);
  
  // I calculated that this *should* work //
  readings[idx] = (newreading * 3300 / 4095 * VOLTAGE_DIVIDER) + FUDGE_OFFSET;  // scale to mV from ADC and undo voltage divider to battery
  meth1 = readings[idx];

  // Heltec had this formula //
  meth2 = (newreading * 0.0025*1000.0) - 375;
  // readings[idx] = newreading * 0.0025*1000.0;
  //Serial.printf("ADC Raw Reading[%d]: %d mV\n", newreading, readings[idx]);

  
  total = total + readings[idx];
  idx++;
  if (idx >= VBATT_SMOOTH) { idx=0; } // Check if idx is past the end of buffer, if so then reset
  
  float voltage = ((float)total / 1000.0 / (float)VBATT_SMOOTH);
  //Serial.printf("Battery Voltage (avg): %.2f\n", voltage);
  
  return voltage;  
}


void initBatteryMonitor()
{
  pinMode(VEXT_GPIO, OUTPUT);   // This pin turns on Vext. It should be on (low)
  digitalWrite(VEXT_GPIO, LOW); // to Read Vbatt on pin VBATT_GPIO
  //adcAttachPin(VBATT_GPIO);   // Used to read voltage from battery

  Serial.println("Initiating battery monitor...\n   Taking initial sample of 'Vbatt'...");
  for (uint8_t i = 0;i < VBATT_SMOOTH;i++) {
    readBattLevel();
  }

  battStatusInflux.addTag("Battery", "MAIN");
}

void sendBatteryToInflux() {

  battStatusInflux.clearFields();
  battStatusInflux.addField("voltage", battVoltage);
  battStatusInflux.addField("percent", battPercent);

  // Write point
  //Serial.print("\nWriting Battery Status: ");
  //Serial.println(battStatusInflux.toLineProtocol());
  if (!writeNewPoint(battStatusInflux)) {
    Serial.print("InfluxDB write Battery Status failed: ");
    Serial.println(getLastErrorMessage());
  } else {
    //Serial.println("  ...done.\n");
  }
  
}


void readLogBattery()
{
  battVoltage = readBattLevel();
  battPercent = calcBattPercent(battVoltage);
  sendBatteryToInflux();

}



#endif