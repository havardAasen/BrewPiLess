/*
 * Copyright 2012-2013 BrewPi/Elco Jacobs.
 *
 * This file is part of BrewPi.
 *
 * BrewPi is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * BrewPi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with BrewPi.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Brewpi.h"
#include "ITempSensor.h"
#include "Pins.h"
#include <algorithm>
#include <climits>

#include "TemperatureFormats.h"
#include "TempControl.h"
#include "PiLink.h"
#include "TempSensor.h"
#include "Ticks.h"
#include "TempSensorMock.h"
#include "EepromManager.h"
#include "TempSensorDisconnected.h"
#include "RotaryEncoder.h"

TempControl tempControl;

#if TEMP_CONTROL_STATIC

extern ValueSensor<bool> defaultSensor;
extern ValueActuator defaultActuator;
extern DisconnectedTempSensor defaultTempSensor;

// These sensors are switched out to implement multi-chamber.
TempSensor* TempControl::beerSensor;
TempSensor* TempControl::fridgeSensor;
ITempSensor* TempControl::ambientSensor = &defaultTempSensor;


Actuator* TempControl::heater = &defaultActuator;
Actuator* TempControl::cooler = &defaultActuator;
Actuator* TempControl::light = &defaultActuator;
Actuator* TempControl::fan = &defaultActuator;

ValueActuator cameraLightState;
AutoOffActuator TempControl::cameraLight(600, &cameraLightState);	// timeout 10 min
Sensor<bool>* TempControl::door = &defaultSensor;

// Control parameters
ControlConstants TempControl::cc;
ControlSettings TempControl::cs;
ControlVariables TempControl::cv;

	// State variables
State TempControl::state;
bool TempControl::doPosPeakDetect;
bool TempControl::doNegPeakDetect;
bool TempControl::doorOpen;

	// keep track of beer setting stored in EEPROM
temperature TempControl::storedBeerSetting;

	// Timers
uint16_t TempControl::lastIdleTime;
uint16_t TempControl::lastHeatTime;
uint16_t TempControl::lastCoolTime;
uint16_t TempControl::waitTime;
#endif

void TempControl::init(){
	state=idle;
	cs.mode = off;

	cameraLight.setActive(false);

	// this is for cases where the device manager hasn't configured beer/fridge sensor.
	if (beerSensor==nullptr) {
		beerSensor = new TempSensor(&defaultTempSensor);
		beerSensor->init();
	}

	if (fridgeSensor==nullptr) {
		fridgeSensor = new TempSensor(&defaultTempSensor);
		fridgeSensor->init();
	}

	updateTemperatures();
	reset();

	// Do not allow heating/cooling directly after reset.
	// A failing script + CRON + Arduino uno (which resets on serial connect) could damage the compressor
	// For test purposes, set these to -3600 to eliminate waiting after reset
	lastHeatTime = 0;
	lastCoolTime = 0;
}

void TempControl::reset(){
	doPosPeakDetect=false;
	doNegPeakDetect=false;
}

void updateSensor(TempSensor* sensor) {
	sensor->update();
	if(!sensor->isConnected()) {
		sensor->init();
	}
}

void TempControl::updateTemperatures(){

	updateSensor(beerSensor);
	updateSensor(fridgeSensor);

	// Read ambient sensor to keep the value up to date. If no sensor is connected, this does nothing.
	// This prevents a delay in serial response because the value is not up to date.
	if(ambientSensor->read() == TEMP_SENSOR_DISCONNECTED){
		ambientSensor->init(); // try to reconnect a disconnected, but installed sensor
	}
}

void TempControl::updatePID(){
	static unsigned char integralUpdateCounter = 0;
	if(tempControl.modeIsBeer()){
		if(cs.beerSetting == INVALID_TEMP){
			// beer setting is not updated yet
			// set fridge to unknown too
			cs.fridgeSetting = INVALID_TEMP;
			return;
		}

		// fridge setting is calculated with PID algorithm. Beer temperature error is input to PID
		cv.beerDiff =  cs.beerSetting - beerSensor->readSlowFiltered();
		cv.beerSlope = beerSensor->readSlope();
		temperature fridgeFastFiltered = fridgeSensor->readFastFiltered();

		if(integralUpdateCounter++ == 60){
			integralUpdateCounter = 0;

			temperature integratorUpdate = cv.beerDiff;

			// Only update integrator in IDLE, because thats when the fridge temp has reached the fridge setting.
			// If the beer temp is still not correct, the fridge setting is too low/high and integrator action is needed.
			if(state != idle){
				integratorUpdate = 0;
			}
			else if(abs(integratorUpdate) < cc.iMaxError){
				// difference is smaller than iMaxError
				// check additional conditions to see if integrator should be active to prevent windup
				bool updateSign = (integratorUpdate > 0); // 1 = positive, 0 = negative
				bool integratorSign = (cv.diffIntegral > 0);

				if(updateSign == integratorSign){
					// beerDiff and integrator have same sign. Integrator would be increased.

					// If actuator is already at max increasing actuator will only cause integrator windup.
					integratorUpdate = (cs.fridgeSetting >= cc.tempSettingMax) ? 0 : integratorUpdate;
					integratorUpdate = (cs.fridgeSetting <= cc.tempSettingMin) ? 0 : integratorUpdate;
					integratorUpdate = ((cs.fridgeSetting - cs.beerSetting) >= cc.pidMax) ? 0 : integratorUpdate;
					integratorUpdate = ((cs.beerSetting - cs.fridgeSetting) >= cc.pidMax) ? 0 : integratorUpdate;

					// cooling and fridge temp is more than 2 degrees from setting, actuator is saturated.
					integratorUpdate = (!updateSign && (fridgeFastFiltered > (cs.fridgeSetting +1024))) ? 0 : integratorUpdate;

					// heating and fridge temp is more than 2 degrees from setting, actuator is saturated.
					integratorUpdate = (updateSign && (fridgeFastFiltered < (cs.fridgeSetting -1024))) ? 0 : integratorUpdate;
				}
				else{
					// integrator action is decreased. Decrease faster than increase.
					integratorUpdate = integratorUpdate*2;
				}
			}
			else{
				// decrease integral by 1/8 when far from the end value to reset the integrator
				integratorUpdate = -(cv.diffIntegral >> 3);
			}
			cv.diffIntegral = cv.diffIntegral + integratorUpdate;
		}

		// calculate PID parts. Use long_temperature to prevent overflow
		cv.p = multiplyFactorTemperatureDiff(cc.Kp, cv.beerDiff);
		cv.i = multiplyFactorTemperatureDiffLong(cc.Ki, cv.diffIntegral);
		cv.d = multiplyFactorTemperatureDiff(cc.Kd, cv.beerSlope);
		long_temperature newFridgeSetting = cs.beerSetting;
		newFridgeSetting += cv.p;
		newFridgeSetting += cv.i;
		newFridgeSetting += cv.d;

		// constrain to tempSettingMin or beerSetting - pidMAx, whichever is lower.
		temperature lowerBound = (cs.beerSetting <= cc.tempSettingMin + cc.pidMax) ? cc.tempSettingMin : cs.beerSetting - cc.pidMax;
		// constrain to tempSettingMax or beerSetting + pidMAx, whichever is higher.
		temperature upperBound = (cs.beerSetting >= cc.tempSettingMax - cc.pidMax) ? cc.tempSettingMax : cs.beerSetting + cc.pidMax;

		cs.fridgeSetting = std::clamp(constrainTemp16(newFridgeSetting), lowerBound, upperBound);
	}
	else if(cs.mode == fridge_constant){
		// FridgeTemperature is set manually, use INVALID_TEMP to indicate beer temp is not active
		cs.beerSetting = INVALID_TEMP;
	}
}

void TempControl::updateState(){
	//update state
	bool stayIdle = false;
	bool newDoorOpen = door->sense();

	if(newDoorOpen!=doorOpen) {
		doorOpen = newDoorOpen;
#ifdef ESP8266  // ESP8266 Doesn't support %S
		String annotation = "";
		annotation += "Fridge door ";
		annotation += doorOpen ? "opened" : "closed";
		piLink.printTemperaturesJSON(nullptr, annotation.c_str());
#else
		piLink.printFridgeAnnotation(PSTR("Fridge door %S"), doorOpen ? PSTR("opened") : PSTR("closed"));
#endif
	}

	if(cs.mode == off){
		state = state_off;
		stayIdle = true;
	}
	// stay idle when one of the required sensors is disconnected, or the fridge setting is INVALID_TEMP
	if( cs.fridgeSetting == INVALID_TEMP ||
		!fridgeSensor->isConnected() ||
		(!beerSensor->isConnected() && tempControl.modeIsBeer())){
		state = idle;
		stayIdle = true;
	}

	uint16_t sinceIdle = timeSinceIdle();
	uint16_t sinceCooling = timeSinceCooling();
	uint16_t sinceHeating = timeSinceHeating();
	temperature fridgeFast = fridgeSensor->readFastFiltered();
	temperature beerFast = beerSensor->readFastFiltered();
	ticks_seconds_t secs = ticks.seconds();
	switch(state)
	{
		case idle:
		case state_off:
		case waiting_to_cool:
		case waiting_to_heat:
		case waiting_for_peak_detect:
		{
			lastIdleTime=secs;
			// set waitTime to zero. It will be set to the maximum required waitTime below when wait is in effect.
			if(stayIdle){
				break;
			}
			resetWaitTime();
			if(fridgeFast > (cs.fridgeSetting+cc.idleRangeHigh) ){  // fridge temperature is too high
				#if SettableMinimumCoolTime
				tempControl.updateWaitTime(cc.mutexDeadTime, sinceHeating);
				#else
				tempControl.updateWaitTime(MIN_SWITCH_TIME, sinceHeating);
				#endif
				if(cs.mode==fridge_constant){
					tempControl.updateWaitTime(MIN_COOL_OFF_TIME_FRIDGE_CONSTANT, sinceCooling);
				}
				else{
					if(beerFast < (cs.beerSetting + 16) ){ // If beer is already under target, stay/go to idle. 1/2 sensor bit idle zone
						state = idle; // beer is already colder than setting, stay in or go to idle
						break;
					}
					#if SettableMinimumCoolTime
					tempControl.updateWaitTime(cc.minCoolIdleTime, sinceCooling);
					#else
					tempControl.updateWaitTime(MIN_COOL_OFF_TIME, sinceCooling);
					#endif
				}
				if(tempControl.cooler != &defaultActuator){
					if(getWaitTime() > 0){
						state = waiting_to_cool;
					}
					else{
						state = cooling;
					}
				}
			}
			else if(fridgeFast < (cs.fridgeSetting+cc.idleRangeLow)){  // fridge temperature is too low
				#if SettableMinimumCoolTime
				tempControl.updateWaitTime(cc.mutexDeadTime, sinceCooling);
				tempControl.updateWaitTime(cc.minHeatIdleTime, sinceHeating);
				#else
				tempControl.updateWaitTime(MIN_SWITCH_TIME, sinceCooling);
				tempControl.updateWaitTime(MIN_HEAT_OFF_TIME, sinceHeating);
				#endif
				if(cs.mode!=fridge_constant){
					if(beerFast > (cs.beerSetting - 16)){ // If beer is already over target, stay/go to idle. 1/2 sensor bit idle zone
						state = idle;  // beer is already warmer than setting, stay in or go to idle
						break;
					}
				}
				if(tempControl.heater != &defaultActuator || (cc.lightAsHeater && (tempControl.light != &defaultActuator))){
					if(getWaitTime() > 0){
						state = waiting_to_heat;
					}
					else{
						state = heating;
					}
				}
			}
			else{
				state = idle; // within IDLE range, always go to IDLE
				break;
			}
			if(state == heating || state == cooling){
				if(doNegPeakDetect == true || doPosPeakDetect == true){
					// If peak detect is not finished, but the fridge wants to switch to heat/cool
					// Wait for peak detection and display 'Await peak detect' on display
					state = waiting_for_peak_detect;
					break;
				}
			}
		}
		break;
		case cooling:
		case cooling_min_time:
		{
			doNegPeakDetect=true;
			lastCoolTime = secs;
			updateEstimatedPeak(cc.maxCoolTimeForEstimate, cs.coolEstimator, sinceIdle);
			state = cooling; // set to cooling here, so the display of COOLING/COOLING_MIN_TIME is correct

			// stop cooling when estimated fridge temp peak lands on target or if beer is already too cold (1/2 sensor bit idle zone)
			if(cv.estimatedPeak <= cs.fridgeSetting || (cs.mode != fridge_constant && beerFast < (cs.beerSetting - 16))){
				#if SettableMinimumCoolTime
				if(sinceIdle > cc.minCoolTime){
				#else
				if(sinceIdle > MIN_COOL_ON_TIME){
				#endif
					cv.negPeakEstimate = cv.estimatedPeak; // remember estimated peak when I switch to IDLE, to adjust estimator later
					state=idle;
					break;
				}
				else{
					state = cooling_min_time;
					break;
				}
			}
		}
		break;
		case heating:
		case heating_min_time:
		{
			doPosPeakDetect=true;
			lastHeatTime=secs;
			updateEstimatedPeak(cc.maxHeatTimeForEstimate, cs.heatEstimator, sinceIdle);
			state = heating; // reset to heating here, so the display of HEATING/HEATING_MIN_TIME is correct

			// stop heating when estimated fridge temp peak lands on target or if beer is already too warm (1/2 sensor bit idle zone)
			if(cv.estimatedPeak >= cs.fridgeSetting || (cs.mode != fridge_constant && beerFast > (cs.beerSetting + 16))){
				#if SettableMinimumCoolTime
				if(sinceIdle > cc.minHeatTime){
				#else
				if(sinceIdle > MIN_HEAT_ON_TIME){
				#endif
					cv.posPeakEstimate=cv.estimatedPeak; // remember estimated peak when I switch to IDLE, to adjust estimator later
					state=idle;
					break;
				}
				else{
					state = heating_min_time;
					break;
				}
			}
		}
		break;
                default:;
        }
}

void TempControl::updateEstimatedPeak(uint16_t timeLimit, temperature estimator, uint16_t sinceIdle)
{
	uint16_t activeTime = min(timeLimit, sinceIdle); // heat or cool time in seconds
	temperature estimatedOvershoot = ((long_temperature) estimator * activeTime)/3600; // overshoot estimator is in overshoot per hour
	if(stateIsCooling()){
		estimatedOvershoot = -estimatedOvershoot; // when cooling subtract overshoot from fridge temperature
	}
	cv.estimatedPeak = fridgeSensor->readFastFiltered() + estimatedOvershoot;
}

void TempControl::updateOutputs() {
	if (cs.mode==test)
		return;

	cameraLight.update();
	bool heating = stateIsHeating();
	bool cooling = stateIsCooling();
	cooler->setActive(cooling);
	heater->setActive(!cc.lightAsHeater && heating);
	light->setActive(isDoorOpen() || (cc.lightAsHeater && heating) || cameraLightState.isActive());
	fan->setActive(heating || cooling);
}


void TempControl::detectPeaks(){
	//detect peaks in fridge temperature to tune overshoot estimators
	LOG_ID_TYPE detected = 0;
	temperature peak, estimate, error, oldEstimator, newEstimator;

	if(doPosPeakDetect && !stateIsHeating()){
		peak = fridgeSensor->detectPosPeak();
		estimate = cv.posPeakEstimate;
		error = peak-estimate;
		oldEstimator = cs.heatEstimator;
		if(peak != INVALID_TEMP){
			// positive peak detected
			if(error > cc.heatingTargetUpper){
				// Peak temperature was higher than the estimate.
				// Overshoot was higher than expected
				// Increase estimator to increase the estimated overshoot
				increaseEstimator(&(cs.heatEstimator), error);
			}
			if(error < cc.heatingTargetLower){
				// Peak temperature was lower than the estimate.
				// Overshoot was lower than expected
				// Decrease estimator to decrease the estimated overshoot
				decreaseEstimator(&(cs.heatEstimator), error);
			}
			detected = INFO_POSITIVE_PEAK;
		}
		else if(timeSinceHeating() > HEAT_PEAK_DETECT_TIME){
			if(fridgeSensor->readFastFiltered() < (cv.posPeakEstimate+cc.heatingTargetLower)){
				// Idle period almost reaches maximum allowed time for peak detection
				// This is the heat, then drift up too slow (but in the right direction).
				// estimator is too high
				peak=fridgeSensor->readFastFiltered();
				decreaseEstimator(&(cs.heatEstimator), error);
				detected = INFO_POSITIVE_DRIFT;
			}
			else{
				// maximum time for peak estimation reached
				doPosPeakDetect = false;
			}
		}
		if(detected){
			newEstimator = cs.heatEstimator;
			cv.posPeak = peak;
			doPosPeakDetect = false;
		}
	}
	else if(doNegPeakDetect && !stateIsCooling()){
		peak = fridgeSensor->detectNegPeak();
		estimate = cv.negPeakEstimate;
		error = peak-estimate;
		oldEstimator = cs.coolEstimator;
		if(peak != INVALID_TEMP){
			// negative peak detected
			if(error < cc.coolingTargetLower){
				// Peak temperature was lower than the estimate.
				// Overshoot was higher than expected
				// Increase estimator to increase the estimated overshoot
				increaseEstimator(&(cs.coolEstimator), error);
			}
			if(error > cc.coolingTargetUpper){
				// Peak temperature was higher than the estimate.
				// Overshoot was lower than expected
				// Decrease estimator to decrease the estimated overshoot
				decreaseEstimator(&(cs.coolEstimator), error);

			}
			detected = INFO_NEGATIVE_PEAK;
		}
		else if(timeSinceCooling() > COOL_PEAK_DETECT_TIME){
			if(fridgeSensor->readFastFiltered() > (cv.negPeakEstimate+cc.coolingTargetUpper)){
				// Idle period almost reaches maximum allowed time for peak detection
				// This is the cooling, then drift down too slow (but in the right direction).
				// estimator is too high
				peak = fridgeSensor->readFastFiltered();
				decreaseEstimator(&(cs.coolEstimator), error);
				detected = INFO_NEGATIVE_DRIFT;
			}
			else{
				// maximum time for peak estimation reached
				doNegPeakDetect=false;
			}
		}
		if(detected){
			newEstimator = cs.coolEstimator;
			cv.negPeak = peak;
			doNegPeakDetect=false;
		}
	}
	if(detected){
		// send out log message for type of peak detected
		logInfoTempTempFixedFixed(detected, peak, estimate, oldEstimator, newEstimator);
	}
}

// Increase estimator at least 20%, max 50%s
void TempControl::increaseEstimator(temperature * estimator, temperature error){
	temperature factor = 614 + std::clamp(abs(error)>>5, 0, 154); // 1.2 + 3.1% of error, limit between 1.2 and 1.5
	*estimator = multiplyFactorTemperatureDiff(factor, *estimator);
	if(*estimator < 25){
		*estimator = intToTempDiff(5)/100; // make estimator at least 0.05
	}
	EepromManager::storeTempSettings();
}

// Decrease estimator at least 16.7% (1/1.2), max 33.3% (1/1.5)
void TempControl::decreaseEstimator(temperature * estimator, const temperature error){
	const temperature factor = 426 - std::clamp(abs(error)>>5, 0, 85); // 0.833 - 3.1% of error, limit between 0.667 and 0.833
	*estimator = multiplyFactorTemperatureDiff(factor, *estimator);
	EepromManager::storeTempSettings();
}

uint16_t TempControl::timeSinceCooling(){
	return ticks.timeSince(lastCoolTime);
}

uint16_t TempControl::timeSinceHeating(){
	return ticks.timeSince(lastHeatTime);
}

uint16_t TempControl::timeSinceIdle(){
	return ticks.timeSince(lastIdleTime);
}

void TempControl::loadDefaultSettings(){
#if BREWPI_EMULATE
	setMode(beer_constant);
#else
	setMode(off);
#endif
	cs.beerSetting = intToTemp(20);
	cs.fridgeSetting = intToTemp(20);
	cs.heatEstimator = intToTempDiff(2)/10; // 0.2
	cs.coolEstimator=intToTempDiff(5);
}

void TempControl::storeConstants(eptr_t offset){
	EepromAccess::writeControlConstants(offset,  cc, sizeof(ControlConstants));
}

void TempControl::loadConstants(eptr_t offset){
	EepromAccess::readControlConstants(cc, offset, sizeof(ControlConstants));
	initFilters();
}

// write new settings to EEPROM to be able to reload them after a reset
// The update functions only write to EEPROM if the value has changed
void TempControl::storeSettings(eptr_t offset){
	EepromAccess::writeControlSettings(offset, cs, sizeof(ControlSettings));
	storedBeerSetting = cs.beerSetting;
}

void TempControl::loadSettings(eptr_t offset){
	EepromAccess::readControlSettings(cs, offset, sizeof(ControlSettings));
	logDebug("loaded settings");
	storedBeerSetting = cs.beerSetting;
	setMode(cs.mode, true);		// force the mode update
}

void TempControl::loadDefaultConstants(){
	memcpy_P((void*) &tempControl.cc, (void*) &tempControl.ccDefaults, sizeof(ControlConstants));
	initFilters();
}

void TempControl::initFilters()
{
	fridgeSensor->setFastFilterCoefficients(cc.fridgeFastFilter);
	fridgeSensor->setSlowFilterCoefficients(cc.fridgeSlowFilter);
	fridgeSensor->setSlopeFilterCoefficients(cc.fridgeSlopeFilter);
	beerSensor->setFastFilterCoefficients(cc.beerFastFilter);
	beerSensor->setSlowFilterCoefficients(cc.beerSlowFilter);
	beerSensor->setSlopeFilterCoefficients(cc.beerSlopeFilter);
}

void TempControl::setMode(Mode newMode, bool force){
	logDebug("TempControl::setMode from %c to %c", cs.mode, newMode);

	if(newMode != cs.mode || state == waiting_to_heat || state == waiting_to_cool || state == waiting_for_peak_detect){
		state = idle;
		force = true;
	}
	if (force) {
		cs.mode = newMode;
		if(newMode == off){
			cs.beerSetting = INVALID_TEMP;
			cs.fridgeSetting = INVALID_TEMP;
		}
		EepromManager::storeTempSettings();
	}
}

temperature TempControl::getBeerTemp(){
	if(beerSensor->isConnected()){
		return beerSensor->readFastFiltered();
	}

	return INVALID_TEMP;
}

temperature TempControl::getBeerSetting(){
	return cs.beerSetting;
}

temperature TempControl::getFridgeTemp(){
	if(fridgeSensor->isConnected()){
		return fridgeSensor->readFastFiltered();
	}

	return INVALID_TEMP;
}

temperature TempControl::getFridgeSetting(){
	return cs.fridgeSetting;
}

void TempControl::setBeerTemp(temperature newTemp){
	temperature oldBeerSetting = cs.beerSetting;
	cs.beerSetting= newTemp;
	if(abs(oldBeerSetting - newTemp) > intToTempDiff(1)/2){ // more than half degree C difference with old setting
		reset(); // reset controller
	}
	updatePID();
	updateState();
	if(cs.mode != beer_profile || abs(storedBeerSetting - newTemp) > intToTempDiff(1)/4){
		// more than 1/4 degree C difference with EEPROM
		// Do not store settings every time in profile mode, because EEPROM has limited number of write cycles.
		// A temperature ramp would cause a lot of writes
		// If Raspberry Pi is connected, it will update the settings anyway. This is just a safety feature.
		EepromManager::storeTempSettings();
	}
}

void TempControl::setFridgeTemp(temperature newTemp){
	cs.fridgeSetting = newTemp;
	reset(); // reset peak detection and PID
	updatePID();
	updateState();
	EepromManager::storeTempSettings();
}

bool TempControl::stateIsCooling(){
	return (state==cooling || state==cooling_min_time);
}
bool TempControl::stateIsHeating(){
	return (state==heating || state==heating_min_time);
}

const ControlConstants TempControl::ccDefaults PROGMEM =
{
	// Do Not change the order of these initializations!
	/* tempFormat */ 'C',
	/* tempSettingMin */ intToTemp(1),	// +1 deg Celsius
	/* tempSettingMax */ intToTemp(30),	// +30 deg Celsius

	// control defines, also in fixed point format (7 int bits, 9 frac bits), so multiplied by 2^9=512
	/* Kp	*/ intToTempDiff(5),	// +5
	/* Ki	*/ intToTempDiff(1)/4, // +0.25
	/* Kd	*/ intToTempDiff(-3)/2,	// -1.5
	/* iMaxError */ intToTempDiff(5)/10,  // 0.5 deg

	// Stay Idle when fridge temperature is in this range
	/* idleRangeHigh */ intToTempDiff(1),	// +1 deg Celsius
	/* idleRangeLow */ intToTempDiff(-1),	// -1 deg Celsius

	// when peak falls between these limits, its good.
	/* heatingTargetUpper */ intToTempDiff(3)/10,	// +0.3 deg Celsius
	/* heatingTargetLower */ intToTempDiff(-2)/10,	// -0.2 deg Celsius
	/* coolingTargetUpper */ intToTempDiff(2)/10,	// +0.2 deg Celsius
	/* coolingTargetLower */ intToTempDiff(-3)/10,	// -0.3 deg Celsius

	// maximum history to take into account, in seconds
	/* maxHeatTimeForEstimate */ 600,
	/* maxCoolTimeForEstimate */ 1200,

	// Set filter coefficients. This is the b value. See FilterFixed.h for delay times.
	// The delay time is 3.33 * 2^b * number of cascades
	/* fridgeFastFilter */ 1u,
	/* fridgeSlowFilter */ 4u,
	/* fridgeSlopeFilter */ 3u,
	/* beerFastFilter */ 3u,
	/* beerSlowFilter */ 4u,
	/* beerSlopeFilter */ 4u,

	/* lightAsHeater */ 0,
	/* rotaryHalfSteps */ 0,

	/* pidMax */ intToTempDiff(10),	// +/- 10 deg Celsius
#if SettableMinimumCoolTime
    /* minCoolTime */ 180,
    /* minCoolIdleTime */ 300,
    /* minHeatTime */  180,
    /* minHeatIdleTime */ 300,
    /* mutexDeadTime */ 600
#endif
};
