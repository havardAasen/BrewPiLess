#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include "BPLSettings.h"

class DataLogger
{
public:
    DataLogger(){ _loggingInfo= theSettings.remoteLogInfo(); }

    // web interface
	void loop(time_t now);
	void reportNow();

protected:
	void sendData();

	RemoteLoggingInformation *_loggingInfo;

	time_t _lastUpdate{};
};
extern DataLogger dataLogger;

#endif
