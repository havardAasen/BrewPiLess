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

#ifndef TEMP_SENSOR_DISCONNECTED_H
#define TEMP_SENSOR_DISCONNECTED_H

#include "ITempSensor.h"
#include "TempSensor.h"

class DisconnectedTempSensor : public ITempSensor {

public:
	[[nodiscard]] bool isConnected() const override { return false; }

	bool init() override {
		return read()!=TEMP_SENSOR_DISCONNECTED;
	}

	[[nodiscard]] temperature read() override {
		return TEMP_SENSOR_DISCONNECTED;
	}

};

#endif
