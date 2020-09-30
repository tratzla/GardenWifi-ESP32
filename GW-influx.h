#ifndef gw_influx
#define gw_influx

#include "Arduino.h"
#include <WiFiMulti.h>
#define DEVICE "ESP32"
#include <InfluxDbClient.h>

#define INFLUXDB_URL "http://52.201.110.70:8086"
#define INFLUXDB_DB_NAME "gwdb"


void initializeGWinflux();

void sendSystemStatusToInflux();

bool writeNewPoint(Point point);

String getLastErrorMessage();

#endif