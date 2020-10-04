#ifndef GWDATAPOINT
#define GWDATAPOINT

#define datalog_max_length 5

struct DhtDataPoint {
  uint timestamp;
  float temperature;
  float humidity;
  float dewpoint;
};

class DhtDatalog {
  private:

  public:
    DhtDataPoint data[datalog_max_length];
    uint i;
    uint len;
    DhtDatalog(){i=0;len=0;}
    DhtDataPoint getData(){return data[i];}
    
    void newDp(float t, float h, float d) {
      uint old_ts = data[i].timestamp;
      i = ++i >= datalog_max_length ? 0 : i;
      len = ++len >= datalog_max_length ? datalog_max_length : len;

      data[i].temperature = t;
      data[i].humidity = h;
      data[i].dewpoint = d;
      data[i].timestamp = old_ts + 1;
    }

    DhtDataPoint popData() {
      DhtDataPoint retval = data[i];
      len--;
      i = i == 0 ? datalog_max_length - 1 : i - 1;
      return retval;
    }
};



struct MoistDataPoint {
  uint timestamp;
  float moisture;
  float volts;
  float raw;
};

struct MoistDatalog {
  private:

  public:
    MoistDataPoint data[datalog_max_length];
    uint i;
    uint len;
    MoistDatalog(){i=0;len=0;}
    MoistDataPoint getData(){return data[i];}
    
    void newDp(float m, float v, float r) {
      uint old_ts = data[i].timestamp;
      i = ++i >= datalog_max_length ? 0 : i;
      len = ++len >= datalog_max_length ? datalog_max_length : len;

      data[i].moisture = m;
      data[i].volts = v;
      data[i].raw = r;
      data[i].timestamp = old_ts + 1;
    }

    MoistDataPoint popData() {
      MoistDataPoint retval = data[i];
      len--;
      i = i == 0 ? datalog_max_length - 1 : i - 1;
      return retval;
    }
};


#endif