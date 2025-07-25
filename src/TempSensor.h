/*
 * Copyright 2012-2013 BrewPi/Elco Jacobs.
 * Copyright 2013 Matthew McGowan.
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

#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H

#include "Brewpi.h"
#include "FilterCascaded.h"
#include "ITempSensor.h"
#include <stdlib.h>

#define TEMP_SENSOR_DISCONNECTED INVALID_TEMP

#ifndef TEMP_SENSOR_CASCADED_FILTER
#define TEMP_SENSOR_CASCADED_FILTER 1
#endif

#if TEMP_SENSOR_CASCADED_FILTER
typedef CascadedFilter TempSensorFilter;
#else
typedef FixedFilter TempSensorFilter;
#endif


class TempSensor {
	public:
	explicit TempSensor(ITempSensor* sensor =nullptr)  {
		setSensor(sensor);
	 }

	 void setSensor(ITempSensor* sensor) {
		 _sensor = sensor;
		 failedReadCount = -1;
	 }
	#if FridgeSensorFallBack
	void setBackupSensor(ITempSensor* sensor) {
		 _backupSensor = sensor;
	 }
	#endif

	void init();

	[[nodiscard]] bool isConnected() const;

	void update();

	temperature readFastFiltered();

	temperature readSlowFiltered(){
		return slowFilter.readOutput(); //return most recent unfiltered value
	}

	temperature readSlope();

	temperature detectPosPeak();

	temperature detectNegPeak();

	void setFastFilterCoefficients(uint8_t b);

	void setSlowFilterCoefficients(uint8_t b);

	void setSlopeFilterCoefficients(uint8_t b);

	ITempSensor& sensor();

	private:
	ITempSensor* _sensor{};
	TempSensorFilter fastFilter;
	TempSensorFilter slowFilter;
	TempSensorFilter slopeFilter;
	unsigned char updateCounter{255}; // first update for slope filter after (255-4s)
	temperature_precise prevOutputForSlope{};
	#if FridgeSensorFallBack
	ITempSensor* _backupSensor{};
	bool _useBackupSensor{};
	#endif
	// An indication of how stale the data is in the filters. Each time a read fails, this value is incremented.
	// It's used to reset the filters after a large enough disconnect delay, and on the first init.
	int8_t failedReadCount{-1};		// -1 for uninitialized, >=0 afterwards.

	friend class DeviceManager;
};

#endif
