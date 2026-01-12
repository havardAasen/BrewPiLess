#include "AutoCapControl.h"
#include "BPLSettings.h"
#include "Config.h"

#include <cstdint>


extern ValueActuator defaultActuator;
Actuator *AutoCapControl::capper = &defaultActuator;


AutoCapControl autoCapControl;

AutoCapMode AutoCapControl::mode() const
{
    if (capper != &defaultActuator)
        return static_cast<AutoCapMode>(_settings->autoCapMode);
    return AutoCapModeNone;
}


void AutoCapControl::begin()
{
    _settings = theSettings.autoCapSettings();

    // check config
    if (_settings->autoCapMode == AutoCapModeManualClose) {
        if (capper != &defaultActuator)
            capper->setActive(true);

    } else if (_settings->autoCapMode == AutoCapModeManualOpen) {
        if (capper != &defaultActuator)
            capper->setActive(false);
    }
}


void AutoCapControl::capAtTime(const std::uint32_t now) const
{
    _settings->autoCapMode = AutoCapModeTime;
    _settings->condition.targetTime = now;
    saveConfig();
}


void AutoCapControl::catOnGravity(const float sg) const
{
    _settings->autoCapMode = AutoCapModeGravity;
    _settings->condition.targetGravity = sg;
    saveConfig();
}


void AutoCapControl::capManualSet(const bool capped) const
{
    _settings->autoCapMode = capped ? AutoCapModeManualClose : AutoCapModeManualOpen;

    if (capper != &defaultActuator)
        capper->setActive(capped);
    saveConfig();
}


bool AutoCapControl::isPhysicalCapOn()
{
    if (capper == &defaultActuator)
        return false;
    return capper->isActive();
}


void AutoCapControl::setPhysicalCapOn(const bool on)
{
    if (capper == &defaultActuator || capper->isActive() == on) {
        return;
    }
    DBG_PRINTF("capper CAP:%d\n", on);
    capper->setActive(on);
}


void AutoCapControl::setCapOn(const bool on)
{
    if (capper == &defaultActuator) {
        return;
    }

    const CapStatus status = on ? CapStatusActive : CapStatusInactive;
    if (status == _capStatus) {
        return;
    }
    _capStatus = status;
    setPhysicalCapOn(on);
}


// one thing to keep in mind: the actuactor might be assigned after power on
// before the actuactor is assigned, the status is "unknown", and
// the action to check and set capping status should be done.
bool AutoCapControl::autoCapOn(const std::uint32_t current, const float gravity)
{
    if (capper == &defaultActuator)
        return false;

    switch (_settings->autoCapMode) {
        case AutoCapModeNone:
            // asigned. auto change to open
            // not necessary for it is check at first statement
            //    if( AutoCapControl::capper != &defaultActuator )
            _settings->autoCapMode = AutoCapModeManualOpen;
            return true;
        case AutoCapModeManualClose:
            if (_capStatus != CapStatusActive)
                setCapOn(true);
            break;
        case AutoCapModeManualOpen:
            if (_capStatus != CapStatusInactive)
                setCapOn(false);
            break;
        case AutoCapModeTime:
            if (current > _settings->condition.targetTime) {
                if (_capStatus != CapStatusActive) {
                    DBG_PRINTF("times up, capped. act:%d\n", _capStatus);
                    setCapOn(true);
                    return true;
                }
            } else if (_capStatus != CapStatusInactive)
                setCapOn(false);
            break;
        case AutoCapModeGravity:
            if (gravity <= _settings->condition.targetGravity) {
                if (_capStatus != CapStatusActive) {
                    DBG_PRINTF("gravity meet, capped.\n");
                    setCapOn(true);
                    return true;
                }
            } else if (_capStatus != CapStatusInactive) {
                setCapOn(false);
            }
            break;
    }
    return false;
}
