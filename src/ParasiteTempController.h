
#ifndef PARASITE_TEMP_CONTROLLER_H
#define PARASITE_TEMP_CONTROLLER_H

#include "TempControl.h"
#include "BPLSettings.h"

class ParasiteTempController
{
public:
    void run();
    void init();

    String getSettings();
    bool updateSettings(String json);
    
    void setTemperatureRange(float lower,float upper);

    char getMode();
    uint32_t getTimeElapsed();

    int getTemp(){ return(int)(_currentTemp * 100.0);}
    int getLowerBound(){return(int)(_settings->setTemp * 100.0);}
    int getUpperBound(){return(int)(_settings->maxIdleTemp * 100.0);}

    static Actuator *cooler;

protected:
    ParasiteTempControlSettings *_settings{};
    
    bool _validSetting{};
    float _currentTemp{};
    uint32_t _lastSwitchedTime{};
    uint32_t _lastSensorValidTime{};

    void _setCooling(bool cool);
    bool checkSettings();
};

extern ParasiteTempController parasiteTempController;

#endif
