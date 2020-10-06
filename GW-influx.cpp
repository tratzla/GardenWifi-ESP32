#include "GW-influx.h"

InfluxDBClient client(INFLUXDB_URL, INFLUXDB_DB_NAME);


void initializeGWinflux() {

  // Check server connection
  if (client.validateConnection()) {
    log_i("Connected to InfluxDB: %s", client.getServerUrl());
  } else {
    log_e("InfluxDB connection failed: %s", client.getLastErrorMessage());
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
  log_w("Forcing a flush of write buffer: ");
  if (client.isBufferEmpty()) {
    log_w("Buffer is EMPTY. Nothing to do.\n");
    return true;
  }
    log_i("Buffer %s.\n", client.isBufferFull() ? "is FULL" : "has SOME DATA...");

  if (!client.flushBuffer()) {
    log_e("   InfluxDB flush failed: %s", client.getLastErrorMessage());
    log_e("Full buffer: %s", client.isBufferFull() ? "Yes" : "No");
    return false;
  } 
  log_i("   ...Success!");
  return true;
}

String getLastErrorMessage() {
    return client.getLastErrorMessage();
}
