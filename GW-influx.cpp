#include "GW-influx.h"

InfluxDBClient client(INFLUXDB_URL, INFLUXDB_DB_NAME);
Point wifistatus("Wifi_Status");

void initializeGWinflux() {
  //tt100.addTag("name", "Sensor1");
  //tt100.addTag("type", "dht11");
  wifistatus.addTag("device", DEVICE);
  wifistatus.addTag("SSID", WiFi.SSID());


  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }

}


void sendSystemStatusToInflux(){

  // Store measured value into point
  wifistatus.clearFields();
  // Report RSSI of currently connected network
  wifistatus.addField("rssi", WiFi.RSSI());
  // Print what are we exactly writing
  Serial.print("Writing System Status: ");
  Serial.println(wifistatus.toLineProtocol());
  // If no Wifi signal, try to reconnect it
  if (WiFi.status() != WL_CONNECTED)
    Serial.println("Wifi connection lost, can't send to influx");
  // Write point
  if (!client.writePoint(wifistatus)) {
    Serial.print("InfluxDB write wifistatus failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  
}

bool writeNewPoint(Point point) {
    return client.writePoint(point);
}


String getLastErrorMessage() {
    return client.getLastErrorMessage();
}
