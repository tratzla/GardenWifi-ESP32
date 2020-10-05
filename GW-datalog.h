#ifndef GWDATAPOINT
#define GWDATAPOINT

#define datalog_max_length 5

enum compare_type {
  gt, // >
  lt, // <
  eq, // =
};

enum alarm_states {
  NORMAL,
  ALARM,
  UNKNOWN,
};

struct persist_alm {
  float vals[3];
  uint i;
  uint len;
  float sp;
  compare_type compare;
  alarm_states state;
};

class alarm_config {
  public:
    float vals[3];
    uint i;
    uint len;
    float sp;
    compare_type compare;
    alarm_states state;
    bool in_use = false;
    String strStr;
    String strSte;
    String strCmp;

    const char *charCompare() {
      strCmp = String(compare==gt?"> [gt]": compare==lt?"< [lt]":"= [eq]");
      return strCmp.c_str();
    }

    const char *strState(){
      strSte = String(state==NORMAL?"OK": state==ALARM?"ALARM": "?");
      return strSte.c_str();
    }

    const char *str() {
      if (!this->in_use) return "";
      char buf[32];
      sprintf(buf, " [ALM:=%s%.1f|%s]", 
          charCompare(), sp, strState());
      strStr = String(buf);
      return strStr.c_str();
    }

    alarm_config(){in_use = false;}
    void newDp(float new_val){
      i = ++i >= 3 ? 0 : i;
      len = ++len >= 3 ? 3 : len;
      vals[i] = new_val;
      check_alarm();
    }

    alarm_states check_alarm() {
      if(len < 2) {
        state = UNKNOWN;
      } else {
        float t = 0;
        for (int j=0;j<len;j++){
          t+=vals[j];
        }
        t = t/len;
        switch(compare) {
          case gt :
            state = t > sp ? ALARM : NORMAL;
              break; 
          case lt :
            state = t < sp ? ALARM : NORMAL;
              break; 
          case eq :
            state = t == sp ? ALARM : NORMAL;
              break; 
        }
      }
      return state;
    }
    void loadFromRTC(persist_alm &alm){
      for(int j=0;j<3;j++){
        vals[j] = alm.vals[j];
      }
      i = alm.i;
      len = alm.len;
      sp = alm.sp;
      compare = alm.compare;
      state = alm.state;
    }
    void copyToRTC(persist_alm &alm){
      for(int j=0;j<3;j++){
        alm.vals[j] = vals[j];
        alm.vals[j] = vals[j];
      }
      alm.i = i;
      alm.len = len;
      alm.sp = sp;
      alm.compare = compare;
      alm.state = state;
    }
};


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