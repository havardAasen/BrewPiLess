#ifndef BPLSettings_H
#define BPLSettings_H
#include <FS.h>
#include <time.h>
#include "Config.h"
//*****************************************************
// 156 bytes
struct SystemConfiguration{
    char  username[32];
    char  password[32];
    char  hostnetworkname[32];
    char  titlelabel[32];
    uint32_t  backlite;
    uint32_t  ip;
    uint32_t  gw;
    uint32_t  netmask;
    uint16_t  port;
    uint8_t passwordLcd;
    uint8_t wifiMode;
    uint32_t dns;
};

//*****************************************************
// time information
// 12
struct TimeInformation{
    uint32_t savedTime;
    uint32_t timezoneoffset;
};

//*****************************************************
// gravity device
//  36
struct GravityDeviceConfiguration{
    float ispindelCoefficients[4];
    float   lpfBeta;
	uint32_t  numberCalPoints;
    
    uint8_t  ispindelEnable;
    uint8_t  ispindelTempCal;
    uint8_t  calculateGravity;
    uint8_t  ispindelCalibrationBaseTemp;

	uint8_t  stableThreshold;
	uint8_t  usePlato;
};

//*****************************************************
// Beer Profile

// 
#define InvalidStableThreshold 0xFF
#define MaximumSteps 7
// datys are encoded by *100
#define ScheduleDayFromJson(d)  ((uint16_t) ((d) * 100.0))
#define ScheduleDayToJson(d)  ((float)(d)/100.0)
#define ScheduleDayToTime(d) ((uint32_t)(d) * 864) //((uint32_t)((float)(d)/100.0 * 86400))
#define ScheduleTempFromJson(t) ((int16_t)((t)*100.0))
#define ScheduleTempToJson(t) ((float)(t)/100.0)

#define ScheduleTemp(t) ((float)t/100.0)
typedef int16_t Gravity;

#define INVALID_GRAVITY -1
#define IsGravityValid(g) ((g)>0)

#define PointToGravity(p) (1000+(Gravity)((p)+0.5))

#define PlatoToGravity(p) ((uint16_t)((p)*10.0 + 0.5))
#define SGToGravity(p) ((uint16_t)((p)*1000.0 + 0.5))
#define GravityToSG(g) (((float)(g) / 1000.0))
#define GravityToPlato(g) (((float)(g) / 10.0))

// 12
struct ScheduleStep{
    int16_t   temp;
    uint16_t  days;
    union GravityT{
        uint16_t  sg;
        uint16_t  attenuation;
    } gravity;
    struct _StableT{
        uint8_t  stableTime;
        uint8_t  stablePoint;
    } stable;
    uint8_t  attSpecified;
    char     condition;
}; // 12bytes

// 12 * 7 +12 = 96
struct BeerTempSchedule{
	ScheduleStep steps[MaximumSteps];
	time_t   startDay;
    uint8_t  numberOfSteps;
    char     unit;
};

struct BrewStatus{
	time_t   startingDate;
	time_t   timeEnterCurrentStep;
	time_t   currentStepDuration;
	uint16_t  OGPoints;
	uint8_t  currentStep;
};

//*****************************************************
// Local logging
#define MAX_LOG_FILE_NUMBER 10

#define MaximumLogFileName 24

struct FileIndexEntry{
	char name[MaximumLogFileName];
	unsigned long time;
};

struct FileIndexes
{
	FileIndexEntry files[MAX_LOG_FILE_NUMBER];
	char logname[MaximumLogFileName];
	unsigned long starttime;
};

//*****************************************************
// Remote logging
#define MaximumContentTypeLength 48
#define MaximumUrlLength 128
#define MaximumFormatLength 256
#define mHTTP_GET 0 
#define mHTTP_POST 1
#define mHTTP_PUT 2 

struct RemoteLoggingInformation{
	char url[MaximumUrlLength];
	char format[MaximumFormatLength];
	char contentType[MaximumContentTypeLength];
	time_t period;
	uint8_t method;
	uint8_t enabled;
	uint8_t service;
};


//*****************************************************
// Auto Cap

struct AutoCapSettings{
    union _condition{
        uint32_t targetTime;
        float    targetGravity;
    }condition;
    uint8_t autoCapMode;
};

//*****************************************************
// Parasite temp control
struct ParasiteTempControlSettings{
    float setTemp;
    float maxIdleTemp;
    uint32_t minCoolingTime;
    uint32_t minIdleTime;
};

//*****************************************************
// MQtt remote control
// too many strings. fixed allocation wastes too much.
// server, user, pass, 4x path = 128 * 7 
// Furthermore, ArduinoJson will modify the "source" buffer.
// so additional buffer is neede to decode.
// So let's store the strings in  a compact way 
#if SupportMqttRemoteControl

#define MqttModeOff 0
#define MqttModeControl 1
#define MqttModeLogging 2
#define MqttModeBothControlLoggging 3

#define MqttReportIndividual 0
#define MqttReportJson 1

#define MqttSettingStringSpace 320
struct MqttRemoteControlSettings{
    uint16_t port;
    uint8_t  mode;
    uint8_t  reportFormat;

    uint16_t  serverOffset;
    uint16_t  usernameOffset;
    uint16_t  passwordOffset;
    uint16_t  modePathOffset;
    uint16_t  beerSetPathOffset;
    uint16_t  capControlPathOffset;
    uint16_t  ptcPathOffset;
    uint16_t  fridgeSetPathOffset;

    uint16_t  reportBasePathOffset;
    uint16_t  reportPeriod;

    uint8_t   _strings[MqttSettingStringSpace];
};
#endif

//*****************************************************
// Pressure Sensor
#if SupportPressureTransducer
#define PMModeOff 0
#define PMModeMonitor 1
#define PMModeControl 2

struct PressureMonitorSettings{
    float fa;
    uint16_t fb;
    uint8_t mode;
    uint8_t psi;
};
#endif

struct WiFiConfiguration{
    char ssid[33];
    char pass[33];
};

//####################################################
// whole structure
struct Settings{
    SystemConfiguration syscfg; //0:156
    TimeInformation  timeinfo; //156:12
    GravityDeviceConfiguration gdc;  //168:36
    BeerTempSchedule tempSchedule; // 204:96
    BrewStatus  brewStatus; // 300:20
    FileIndexes  logFileIndexes; // 320:316
    RemoteLoggingInformation remoteLogginInfo; // 636: 444
    AutoCapSettings autoCapSettings; // 1080: 12
    ParasiteTempControlSettings parasiteTempControlSettings; //1092: 20
    WiFiConfiguration wifiConfiguration;

#if SupportPressureTransducer
    PressureMonitorSettings pressureMonitorSettings; // 16
#endif
#if SupportMqttRemoteControl
    MqttRemoteControlSettings mqttRemoteControlSettings;
#endif
};

class BPLSettings
{
public:
    void load();
    void save();
    // system configuration
    SystemConfiguration* systemConfiguration();
    bool dejsonSystemConfiguration(String json);
    String jsonSystemConfiguration();
    // time info
    TimeInformation* timeInformation(){ return &_data.timeinfo;}
    // gravity device
    GravityDeviceConfiguration* GravityConfig(){return &_data.gdc; }
    bool dejsonGravityConfig(char* json);
    String jsonGravityConfig();
    // beer profile
    BeerTempSchedule* beerTempSchedule(){ return &_data.tempSchedule;}
    BrewStatus* brewStatus(){ return &_data.brewStatus;}
    bool dejsonBeerProfile(String json);
    String jsonBeerProfile();

    // local log
    FileIndexes* logFileIndexes(){ return &_data.logFileIndexes; }

    // Remote logging
    RemoteLoggingInformation *remoteLogInfo(){return &_data.remoteLogginInfo;}
    bool dejsonRemoteLogging(String json);
    String jsonRemoteLogging();

    // autocap
    AutoCapSettings *autoCapSettings(){ return &_data.autoCapSettings;}
    //ParasiteTempControlSettings
    ParasiteTempControlSettings *parasiteTempControlSettings(){ return &_data.parasiteTempControlSettings;}
    bool dejsonParasiteTempControlSettings(String json);
    String jsonParasiteTempControlSettings(bool enabled);

    void preFormat();
    void postFormat();

    WiFiConfiguration *getWifiConfiguration(){ return &_data.wifiConfiguration; }
    void setWiFiConfiguration(const char* ssid, const char* pass);

#if SupportPressureTransducer
    //pressure monitor
    PressureMonitorSettings *pressureMonitorSettings(){return &_data.pressureMonitorSettings;}
    bool dejsonPressureMonitorSettings(String json);
    String jsonPressureMonitorSettings();
#endif

#if SupportMqttRemoteControl
    MqttRemoteControlSettings *mqttRemoteControlSettings(){ return & _data.mqttRemoteControlSettings;}
    bool dejsonMqttRemoteControlSettings(String json);
    String jsonMqttRemoteControlSettings();
#endif
protected:
    Settings _data{};

    void    setDefault();

    void defaultSystemConfiguration();
    void defaultTimeInformation();
    void defaultGravityConfig();
    void defaultBeerProfile();
    void defaultLogFileIndexes();
    void defaultRemoteLogging();
    void defaultAutoCapSettings();
#if EanbleParasiteTempControl   
    void defaultParasiteTempControlSettings();
#endif
};

extern BPLSettings theSettings;
#endif //BPLSettings_H