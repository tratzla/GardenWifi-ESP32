#ifndef GWDATAPOINT
#define GWDATAPOINT

struct DataPoint {
  uint timestamp;
  float temperature;
  float humidity;
  float dewpoint;
};

#endif