#include "TimeKeeper.h"
#include "BPLSettings.h"
#include "Config.h"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ctime>
#include <sntp.h>


TimeKeeperClass TimeKeeper;

void TimeKeeperClass::setCurrentTime(const std::time_t now)
{
    reference_epoc_ = now;
    reference_system_time_ = millis();
    last_saved_ = reference_epoc_;
    saveTime(now);
}


std::time_t TimeKeeperClass::_queryServer()
{
    DBG_PRINTF("TimeKeeper: Query NTP server\n");
    constexpr std::int8_t attempts{20};
    std::time_t secs{};

    for (std::int8_t i = 0; i < attempts; i++) {
#ifdef ESP32
        time(&secs);
#else
        secs = sntp_get_current_timestamp();
#endif
        if (secs > 1546265623) {
            DBG_PRINTF("TimeKeeper: Time from NTP: %lld\n", secs);
            ntp_synced_ = true;
            break;
        }
        delay(750);
    }
    return secs;
}


void TimeKeeperClass::updateTime()
{
    const std::time_t secs = _queryServer();
    if (secs < 1546265623) {
        return;
    }

    reference_system_time_ = millis();
    reference_epoc_ = secs;
}


void TimeKeeperClass::begin()
{
    reference_epoc_ = loadTime();
    reference_epoc_ += 300; // add 5 minutes.
    reference_system_time_ = millis();
    last_saved_ = reference_epoc_;
    DBG_PRINTF("Load saved time: %lld\n", reference_epoc_);
}


void TimeKeeperClass::begin(const char *server1, const char *server2, const char *server3)
{
    if (server1)
        sntp_setservername(0, server1);
    else
        sntp_setservername(0, "time.nist.gov");
    if (server2)
        sntp_setservername(1, server2);
    if (server3)
        sntp_setservername(2, server3);
    sntp_set_timezone(0);
    sntp_init();

    std::time_t secs{};
    if (WiFi.status() == WL_CONNECTED) {
        secs = _queryServer();
    }

    delay(500);
    if (secs < 1546265623) {
        secs = loadTime() + 30;
        DBG_PRINTF("TimeKeeper: Failed to connect to NTP, load time: %lld\n", secs);
    }
    reference_system_time_ = millis();
    reference_epoc_ = secs;
    last_saved_ = reference_epoc_;
}


std::time_t TimeKeeperClass::getTimeSeconds() // get Epoch time
{
    unsigned long diff = millis() - reference_system_time_;

    if (diff > resync_time) {
        if (WiFi.status() == WL_CONNECTED) {
            if (const unsigned long newtime = sntp_get_current_timestamp(); newtime) {
                reference_system_time_ = millis();
                reference_epoc_ = newtime;
                diff = 0;
            }
        } else {
            // just add up
            reference_system_time_ = millis();
            reference_epoc_ = reference_epoc_ + diff / 1000;
            diff = 0;
        }
    }
    const std::time_t now = reference_epoc_ + diff / 1000;

    if (now - last_saved_ > time_saving_period) {
        saveTime(now);
        last_saved_ = now;
    }
    return now;
}


const char *TimeKeeperClass::getDateTimeStr()
{
    const std::time_t current = getTimeSeconds();
    const std::tm *t = localtime(&current);

    // 2016-07-01T05:22:33Z
    sprintf(date_time_str_buff_, "%d-%02d-%02dT%02d:%02d:%02dZ", t->tm_year + 1900, t->tm_mon + 1,
            t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    return date_time_str_buff_;
}


void TimeKeeperClass::saveTime(const std::time_t t)
{
    theSettings.timeInformation()->savedTime = t;
    theSettings.save();
}


std::time_t TimeKeeperClass::loadTime() { return theSettings.timeInformation()->savedTime; }


void TimeKeeperClass::setTimezoneOffset(const std::int32_t offset)
{
    theSettings.timeInformation()->timezoneoffset = offset;
}


std::uint32_t TimeKeeperClass::getTimezoneOffset()
{
    return theSettings.timeInformation()->timezoneoffset;
}


bool TimeKeeperClass::isSynchronized() const { return ntp_synced_; }


std::time_t TimeKeeperClass::getLocalTimeSeconds()
{
    return getTimeSeconds() + theSettings.timeInformation()->timezoneoffset;
}
