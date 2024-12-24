#ifndef BREW_KEEPER_H
#define BREW_KEEPER_H

#include <Arduino.h>
#include "Config.h"
#include "GravityTracker.h"
#include "BPLSettings.h"
// always enabled #if EnableGravitySchedule


enum Mode : uint8_t;

class BrewProfile
{
	BeerTempSchedule *_schedule;
	BrewStatus  *_status;

	char _unit{'U'};
	uint8_t _stableThreshold{1};

	void _tempConvert();

	void _estimateStep(time_t now,Gravity gravity);

	void _toNextStep(unsigned long time);
	bool checkCondition(unsigned long time,Gravity gravity);
	bool _loadProfile(String filename);
	uint32_t currentStepDuration();
	void _saveBrewingStatus();
public:
	BrewProfile() {
		_schedule =  theSettings.beerTempSchedule();
		_status =  theSettings.brewStatus();
	}
	void setUnit(char unit);
	void setOriginalGravityPoint(uint16_t gravity);
	float tempByTimeGravity(time_t time,Gravity gravity);
	void setStableThreshold(uint8_t threshold){ _stableThreshold=threshold; }
	void profileUpdated();
	void setScheduleStartDate(time_t time);
};


class BrewKeeper
{
protected:
	time_t _lastSetTemp{};

	BrewProfile _profile;
	Gravity _lastGravity{INVALID_GRAVITY};

	void (*_write)(const char*);
	void _loadProfile();
public:

	explicit BrewKeeper(void(*puts)(const char*)):_write(puts){}
	void updateGravity(float sg);
	void updateOriginalGravity(float sg);

	void keep(time_t now);

	void setStableThreshold(uint8_t threshold){_profile.setStableThreshold(threshold);}

	void profileUpdated(){ _profile.profileUpdated();}
	void begin(){ _profile.profileUpdated();}

	void setModeFromRemote(Mode mode);
	void setBeerSet(char *tempStr);
	void setFridgeSet(char *tempStr);
};

extern BrewKeeper brewKeeper;
#endif
