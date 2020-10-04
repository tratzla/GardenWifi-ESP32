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
          //When connected set 
          Serial.print("WiFi connected! IP address: ");
          Serial.println(WiFi.localIP());
          Serial.print("Gateway: ");
          Serial.println(WiFi.gatewayIP());
          //wifi_connected = true;
          initializeGWinflux();
          break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
          Serial.println("WiFi lost connection");
          //wifi_connected = false;
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

  Serial.print("\nWriting Wifi Status: ");
  Serial.println(pointWifiStatus.toLineProtocol());
  if (!writeNewPoint(pointWifiStatus)) {
    Serial.print("InfluxDB write wifistatus failed: ");
    Serial.println(getLastErrorMessage());
  }
}


void initializeGWwifi() {
  Serial.println("Connecting to a WiFi network: " );

  WiFi.mode(WIFI_STA);
  wifiMulti.addAP("COMPEX-DEN", "gypsyflies6008");
  wifiMulti.addAP("COMPEX_US", "gypsyflies6008");
  wifiMulti.addAP("AwesomeNetUS", "trentis##1");
  wifiMulti.addAP("AwesomeNetDS", "trentis##1");

  WiFi.onEvent(WiFiEvent);
  Serial.println("Waiting for WIFI connection...");
  wifiMulti.run();

  pointWifiStatus.addTag("device", DEVICE);
  pointWifiStatus.addTag("SSID", WiFi.SSID());
  
}

#endif