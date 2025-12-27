#ifndef EXTERNAL_DATA_H
#define EXTERNAL_DATA_H

#include "BrewPiProxy.h"
#include "BrewKeeper.h"
#include "BrewLogger.h"
#include "BPLSettings.h"

#if BREWPI_EXTERNAL_SENSOR
#include "TempSensorWireless.h"
#endif

#define INVALID_VOLTAGE -1
#define INVALID_GRAVITY -1
#ifdef INVALID_TEMP
#undef INVALID_TEMP
#endif
#define INVALID_TEMP  -250

inline bool isTiltAngleValid(const float angle) { return angle > 0; }
#define IsVoltageValid(v) ((v) > 0)
//#define IsGravityValid(g) ((g) > 0)

#define IsGravityInValidRange(g) ((g) > 0.8 && (g) < 1.25)
#define GavityDeviceConfigFilename "/gdconfig"
#define MAX_CONFIGDATA_SIZE 256

#define MimimumDifference 0.0000001

#define ErrorNone 0
#define ErrorAuthenticateNeeded 1
#define ErrorJSONFormat 2
#define ErrorMissingField 3
#define ErrorUnknownSource 4

#define C2F(t) ((t)*1.8+32)

class SimpleFilter
{
	float _y{};
	float _b{0.1};
public:
	void setInitial(float v){ _y=v;}
	void setBeta(float b) { _b = b; }
	float beta(){ return _b; }

	float addData(float x){
		_y = _y + _b * (x - _y);
		return _y;
	}
};

class ExternalData
{
protected:
	float _gravity{INVALID_GRAVITY};
	float _auxTemp{INVALID_TEMP};
	time_t _lastUpdate{};
	float  _deviceVoltage{INVALID_VOLTAGE};
//	float _og;
	SimpleFilter filter;
	char *_ispindelName{};
	float _ispindelTilt{-1};
	bool  _calibrating{};
	float _filteredGravity{INVALID_GRAVITY};
	int16_t _rssi{};
	bool _rssiValid{};


	GravityDeviceConfiguration *_cfg{};

	float temperatureCorrection(float sg, float t, float c);

	void setTilt(float tilt,float temp,time_t now);
	void setGravity(float sg, time_t now,bool log=true);
	void setAuxTemperatureCelsius(float temp);
	void setOriginalGravity(float og);	
public:
	float gravity(bool filtered=false);
	float plato(bool filtered=false);

	// to prevent from calculate gravity when no valid formula available.
	void waitFormula();
	void setCalibrating(bool cal){ _calibrating=cal;}
	//configuration reading
    bool iSpindelEnabled();
	float hydrometerCalibration();

    void sseNotify(char *buf);
	//configuration processs
    bool processconfig(char* configdata);
	void loadConfig();
	//update formula
	void formula(float coeff[4],uint32_t npt);


	// for remote data logger
	float auxTemp(){return _auxTemp; }
//	void setUpdateTime(time_t update){ _lastUpdate=update;}
	time_t lastUpdate(){return _lastUpdate;}
	void setDeviceVoltage(float vol){ _deviceVoltage = vol; }
	void setDeviceRssi(int16_t rssi){_rssi = rssi;  _rssiValid=true;}
	float deviceVoltage(){return _deviceVoltage;}
	float tiltValue(){return _ispindelTilt;}
	void invalidateDeviceVoltage() { _deviceVoltage= INVALID_VOLTAGE; }

	bool processGravityReport(char data[],size_t length, bool authenticated, uint8_t& error);
};

extern ExternalData externalData;

#endif
