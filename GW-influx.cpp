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
  configTzTime("UTC", "pool.ntp.org", "time.nis.gov"); //This will try to sync time in background
  // timeSync("UTC", "pool.ntp.org", "time.nis.gov"); // This will block until synced/timeout
  client.setWriteOptions(WriteOptions().batchSize(INFLUX_BATCH_WRITE_SIZE));

}

bool writeNewPoint(Point point) {
    return client.writePoint(point);
}

bool flushInfluxBuffer(){
  Serial.printf("Forcing a flush of write buffer: ");
  if (client.isBufferEmpty()) {
    Serial.printf("Buffer is EMPTY. Nothing to do.\n");
    return true;
  }
    Serial.printf("Buffer %s.\n", client.isBufferFull() ? "is FULL" : "has SOME DATA...");

  if (!client.flushBuffer()) {
    Serial.print("   InfluxDB flush failed: ");
    Serial.println(client.getLastErrorMessage());
    Serial.print("Full buffer: ");
    Serial.println(client.isBufferFull() ? "Yes" : "No");
    return false;
  } 
  Serial.println("   ...Success!");
  return true;
}

String getLastErrorMessage() {
    return client.getLastErrorMessage();
}
