#ifndef GW_WIFI
#define GW_WIFI

#include <WiFiMulti.h>
#include "GW-influx.h"

WiFiMulti wifiMulti;
//bool wifi_connected = false;
//bool wifi_connected() {
//   return wifiMulti.run() != WL_CONNECTED;
//   return WiFi.status() != WL_CONNECTED;
// }

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
  
}

#endif