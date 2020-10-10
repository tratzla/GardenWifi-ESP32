#ifndef GW_DATALOG_H
#define GW_DATALOG_H


#define DL_MAX_LEN 5

#define SENS_TYP_DHT   0
#define SENS_TYP_MOIST 1

#define TEMPERATURE  0
#define HUMIDITY     1
#define DEWPOINT     2

#define MOISTURE     0
#define VOLTS        1
#define RAW          2

#define SENSOR_IDLE      0
#define SENSOR_READING   1
#define SENSOR_UPLOADING 2
#define SENSOR_BUSY      3
#define SENSOR_FAULT     4

#define CFG_CHNG_INUSE 1
#define CFG_CHNG_SP    2
#define CFG_CHNG_COMP  3

enum compare_enum {COMP_GT, COMP_LT, COMP_EQ};
enum alarm_enum {NORMAL, ALARM, UNKNOWN};


/* Holds alarm config information for storing in NVS during power off/sleep */
struct alm_cfg_t {
  bool in_use = false;
  float sp;
  compare_enum compare;
  alarm_enum state = UNKNOWN;
};


struct data_point_t {
  uint timestamp;
  float val[3];
};

/* Holds log information for storing in RTC during sleep */
struct data_log_t {
  data_point_t data[DL_MAX_LEN];
  uint i;
  uint len;
};




class SensorNG {
  private:
    // String strAlarmRepr;
    // String strAlarmState;
    // String strAlarmCompare;
    char strAlarmRepr[64];
    char strAlarmState[64];
    char strAlarmCompare[64];

    char strName[64];
    char strShortRepr[64];
    char strRepr[128];

    char buf[128];


  public:
    alm_cfg_t *AlarmConfig;
    data_log_t *DataLog;

    int pin;
    uint activity;
    uint type;
    String name;

    /* Datalog methods */
    data_point_t  getData() {return DataLog->data[DataLog->i];}
    data_point_t  popData();
    void          newData(float a, float b, float c);
    uint          length()  {return DataLog->len;}

    /* String Representation methods */
    const char*   strAlarm_Compare();
    const char*   strAlarm_State();
    const char*   strAlarm_repr();
    const char*   srepr();
    const char*   repr();

    /* Alarming methods */
    alarm_enum checkAlarm();
    bool alm_inUse() {return this->AlarmConfig->in_use;}
    void changeConfig(uint whichParam);

    void init(const char *n, int p, uint t, data_log_t *dl);
    void loadAlmConfig_nvs();
    void saveAlmConfig_nvs();
    // SensorNG(alm_cfg_t *almcfg) {AlarmConfig = almcfg;}
    // SensorNG(alm_cfg_t &almcfg) {AlarmConfig = *almcfg;}

    SensorNG()  {this->AlarmConfig = new alm_cfg_t;}
    ~SensorNG() {delete AlarmConfig;}  
};




// void init_alarm(){AlarmConfig->in_use = 0;}
// void SensorNG::init_alarm() {
//   AlarmConfig->len = 0;
//   AlarmConfig->i = 0;
//   AlarmConfig->in_use = false;
// }

// class DataLog {
//   private:

//   public:
//     data_point_t data[DL_MAX_LEN];
//     uint i;
//     uint len;
//     DhtDatalog(){i=0;len=0;}
//     DhtDataPoint getData(){return data[i];}
    
//     void newDp(float t, float h, float d) {
//       uint old_ts = data[i].timestamp;
//       i = ++i >= DL_MAX_LEN ? 0 : i;
//       len = ++len >= DL_MAX_LEN ? DL_MAX_LEN : len;

//       data[i].temperature = t;
//       data[i].humidity = h;
//       data[i].dewpoint = d;
//       data[i].timestamp = old_ts + 1;
//     }

//     DhtDataPoint popData() {
//       DhtDataPoint retval = data[i];
//       len--;
//       i = i == 0 ? DL_MAX_LEN - 1 : i - 1;
//       return retval;
//     }
// };



// class alarm_config {
//   public:
//     float vals[3];
//     uint i;
//     uint len;
//     float sp;
//     compare_enum compare;
//     alarm_enum state;
//     bool in_use = false;
//     bool unsaved_change = false;
//     String strStr;
//     String strSte;
//     String strCmp;

//     const char *charCompare() {
//       strCmp = String(compare==COMP_GT?"> [gt]": compare==COMP_LT?"< [lt]":"= [eq]");
//       return strCmp.c_str();
//     }

//     const char *strState(){
//       strSte = String(state==NORMAL?"OK": state==ALARM?"ALARM": "?");
//       return strSte.c_str();
//     }

//     const char *str() {
//       if (!this->in_use) return "";
//       char buf[32];
//       sprintf(buf, " [ALM:=%s%.1f|%s]", 
//           charCompare(), sp, strState());
//       strStr = String(buf);
//       return strStr.c_str();
//     }

//     alarm_config(){in_use = false;}
//     void newDp(float new_val){
//       i = ++i >= 3 ? 0 : i;
//       len = ++len >= 3 ? 3 : len;
//       vals[i] = new_val;
//       check_alarm();
//     }

//     uint check_alarm() {
//       if(len < 2) {
//         state = UNKNOWN;
//       } else {
//         float t = 0;
//         for (int j=0;j<len;j++){
//           t+=vals[j];
//         }
//         t = t/len;
//         switch(compare) {
//           case COMP_GT :
//             state = t > sp ? ALARM : NORMAL;
//               break; 
//           case COMP_LT :
//             state = t < sp ? ALARM : NORMAL;
//               break; 
//           case COMP_EQ :
//             state = t == sp ? ALARM : NORMAL;
//               break; 
//         }
//       }
//       return state;
//     }

//     bool configureAlarm(compare_enum comp, float setp){
//       sp = setp;
//       i = 0;
//       len = 0;
//       compare = comp;
//       in_use = true;
//     }

//     void loadFromRTC(persist_alm &alm){
//       for(int j=0;j<3;j++){
//         vals[j] = alm.vals[j];
//       }
//       i = alm.i;
//       len = alm.len;
//       sp = alm.sp;
//       compare = alm.compare;
//       state = alm.state;
//     }
//     void copyToRTC(persist_alm &alm){
//       for(int j=0;j<3;j++){
//         alm.vals[j] = vals[j];
//         alm.vals[j] = vals[j];
//       }
//       alm.i = i;
//       alm.len = len;
//       alm.sp = sp;
//       alm.compare = compare;
//       alm.state = state;
//     }
// };


// struct DhtDataPoint {
//   uint timestamp;
//   float temperature;
//   float humidity;
//   float dewpoint;
// };

// class DhtDatalog {
//   private:

//   public:
//     DhtDataPoint data[DL_MAX_LEN];
//     uint i;
//     uint len;
//     DhtDatalog(){i=0;len=0;}
//     DhtDataPoint getData(){return data[i];}
    
//     void newDp(float t, float h, float d) {
//       uint old_ts = data[i].timestamp;
//       i = ++i >= DL_MAX_LEN ? 0 : i;
//       len = ++len >= DL_MAX_LEN ? DL_MAX_LEN : len;

//       data[i].temperature = t;
//       data[i].humidity = h;
//       data[i].dewpoint = d;
//       data[i].timestamp = old_ts + 1;
//     }

//     DhtDataPoint popData() {
//       DhtDataPoint retval = data[i];
//       len--;
//       i = i == 0 ? DL_MAX_LEN - 1 : i - 1;
//       return retval;
//     }
// };



// struct MoistDataPoint {
//   uint timestamp;
//   float moisture;
//   float volts;
//   float raw;
// };

// struct MoistDatalog {
//   private:

//   public:
//     MoistDataPoint data[DL_MAX_LEN];
//     uint i;
//     uint len;
//     MoistDatalog(){i=0;len=0;}
//     MoistDataPoint getData(){return data[i];}
    
//     void newDp(float m, float v, float r) {
//       uint old_ts = data[i].timestamp;
//       i = ++i >= DL_MAX_LEN ? 0 : i;
//       len = ++len >= DL_MAX_LEN ? DL_MAX_LEN : len;

//       data[i].moisture = m;
//       data[i].volts = v;
//       data[i].raw = r;
//       data[i].timestamp = old_ts + 1;
//     }

//     MoistDataPoint popData() {
//       MoistDataPoint retval = data[i];
//       len--;
//       i = i == 0 ? DL_MAX_LEN - 1 : i - 1;
//       return retval;
//     }
// };


#endif