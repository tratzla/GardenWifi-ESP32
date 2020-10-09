#ifndef GWDATAPOINT
#define GWDATAPOINT

// typedef uint8_t compare_type;
// typedef uint8_t alarm_states;

enum compare_states {COMP_GT, COMP_LT, COMP_EQ};
enum alarm_states {NORMAL, ALARM, UNKNOWN};

// #define COMP_GT 1
// #define COMP_LT 2
// #define COMP_EQ 3
// #define NORMAL  4
// #define ALARM   5
// #define UNKNOWN 6

#define datalog_max_length 5

/* Holds all state information for storing in RTC during sleep */
struct persist_alm {
  float vals[3];
  uint i;
  uint len;
  float sp;
  compare_states compare;
  alarm_states state;
};


class alarm_config {
  public:
    float vals[3];
    uint i;
    uint len;
    float sp;
    compare_states compare;
    alarm_states state;
    bool in_use = false;
    bool unsaved_change = false;
    String strStr;
    String strSte;
    String strCmp;

    const char *charCompare() {
      strCmp = String(compare==COMP_GT?"> [gt]": compare==COMP_LT?"< [lt]":"= [eq]");
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

    uint check_alarm() {
      if(len < 2) {
        state = UNKNOWN;
      } else {
        float t = 0;
        for (int j=0;j<len;j++){
          t+=vals[j];
        }
        t = t/len;
        switch(compare) {
          case COMP_GT :
            state = t > sp ? ALARM : NORMAL;
              break; 
          case COMP_LT :
            state = t < sp ? ALARM : NORMAL;
              break; 
          case COMP_EQ :
            state = t == sp ? ALARM : NORMAL;
              break; 
        }
      }
      return state;
    }

    bool configureAlarm(compare_states comp, float setp){
      sp = setp;
      i = 0;
      len = 0;
      compare = comp;
      in_use = true;
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