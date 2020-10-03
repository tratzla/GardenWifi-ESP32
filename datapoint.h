#ifndef GWDATAPOINT
#define GWDATAPOINT

#define datalog_max_length 5

struct DataPoint {
  uint timestamp;
  float temperature;
  float humidity;
  float dewpoint;
};


#endif