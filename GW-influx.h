#ifndef gw_influx
#define gw_influx

#include "Arduino.h"
#include <WiFiMulti.h>
#define DEVICE "ESP32"
#include <InfluxDbClient.h>

#define INFLUXDB_URL "http://52.201.110.70:8086"
#define INFLUXDB_DB_NAME "gwdb"
#define INFLUX_BATCH_WRITE_SIZE 40


void initializeGWinflux();

bool writeNewPoint(Point point);

String getLastErrorMessage();

#endif