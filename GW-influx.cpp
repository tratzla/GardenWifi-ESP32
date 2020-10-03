#include "GW-influx.h"

InfluxDBClient client(INFLUXDB_URL, INFLUXDB_DB_NAME);


void initializeGWinflux() {

  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  client.setWriteOptions(WriteOptions().writePrecision(WritePrecision::MS));
  configTzTime("UTC", "pool.ntp.org", "time.nis.gov");
  client.setWriteOptions(WriteOptions().batchSize(INFLUX_BATCH_WRITE_SIZE));

}

bool writeNewPoint(Point point) {
    return client.writePoint(point);
}


String getLastErrorMessage() {
    return client.getLastErrorMessage();
}
