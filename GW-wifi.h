#ifndef GW_WIFI
#define GW_WIFI

#include <WiFiMulti.h>
#include "GW-influx.h"

WiFiMulti wifiMulti;
Point pointWifiStatus("Wifi_Status");

//wifi event handler
void WiFiEvent(WiFiEvent_t event){
    switch(event) {
      case SYSTEM_EVENT_STA_GOT_IP:
          log_i("WiFi connected! IP address: %s", WiFi.localIP());
          log_i("Gateway: %s", WiFi.gatewayIP());
          initializeGWinflux();
          break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
          log_e("WiFi lost connection");
          break;
      default: break;
    }
}

void logWifiStatus() {
  pointWifiStatus.clearFields();
  pointWifiStatus.clearTags();

  pointWifiStatus.addTag("device", DEVICE);
  pointWifiStatus.addTag("SSID", WiFi.SSID());
  
  if (WiFi.status() != WL_CONNECTED) {
    pointWifiStatus.addField("rssi", -99);
    pointWifiStatus.addTag("DISCONNECTED", "true");
  } else {
    pointWifiStatus.addField("rssi", WiFi.RSSI());
  }

  log_i("\nWriting Wifi Status: %s", String(pointWifiStatus.toLineProtocol()));
  if (!writeNewPoint(pointWifiStatus)) {
    log_e("InfluxDB write wifistatus failed: %s", getLastErrorMessage());
  }
}


void initializeGWwifi() {
  log_i("Connecting to a WiFi network: " );

  WiFi.mode(WIFI_STA);
  wifiMulti.addAP("COMPEX-DEN", "gypsyflies6008");
  wifiMulti.addAP("COMPEX_US", "gypsyflies6008");
  wifiMulti.addAP("AwesomeNetUS", "trentis##1");
  wifiMulti.addAP("AwesomeNetDS", "trentis##1");

  WiFi.onEvent(WiFiEvent);
  log_i("Waiting for WIFI connection...");
  wifiMulti.run();

  pointWifiStatus.addTag("device", DEVICE);
  pointWifiStatus.addTag("SSID", WiFi.SSID());
  
}

#endif