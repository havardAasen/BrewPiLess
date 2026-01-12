#ifndef AUTO_CAP_CONTROL_H
#define AUTO_CAP_CONTROL_H

#include "Brewpi.h"
#include "Actuator.h"
#include "BPLSettings.h"

#define AutoCapModeNone 0
#define AutoCapModeManualOpen 1
#define AutoCapModeManualClose 2
#define AutoCapModeTime 3
#define AutoCapModeGravity 4


#define CapStatusInactive 0
#define CapStatusActive  1
#define CapStatusUnknown 2
typedef uint8_t CapStatus;

class AutoCapControl
{
public:
    void begin();

    bool isCapOn();
    bool isPhysicalCapOn();
    void setPhysicalCapOn(bool on);

    bool autoCapOn(uint32_t current, float gravity);
    void capManualSet(bool capped);
    void capAtTime(uint32_t now);
    void catOnGravity(float sg);
    
    uint32_t targetTime(){return _settings->condition.targetTime;}
    float    targetGravity(){return _settings->condition.targetGravity;}
    uint8_t mode();

    static Actuator* capper;

private:
    AutoCapSettings *_settings{};
    CapStatus _capStatus{CapStatusUnknown};

    void setCapOn(bool on);

    void saveConfig();
};

extern AutoCapControl autoCapControl;
#endif