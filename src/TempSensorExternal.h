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

#ifndef TEMP_SENSOR_EXTERNAL_H
#define TEMP_SENSOR_EXTERNAL_H

#include "Brewpi.h"
#include "ITempSensor.h"
#include "TempSensor.h"

/**
 * A temp sensor whose value is not read from the device, but set in code.
 * This is used by the simulator.
 */
class ExternalTempSensor : public ITempSensor
{
	public:
	explicit ExternalTempSensor(bool connected=false)
	{
		setConnected(connected);
	}

	void setConnected(bool connected)
	{
		this->_connected = connected;
	}

	[[nodiscard]] bool isConnected() const override { return _connected; }

	bool init() override {
		return read()!=TEMP_SENSOR_DISCONNECTED;
	}

	[[nodiscard]] temperature read() override {
		if (!isConnected())
			return TEMP_SENSOR_DISCONNECTED;
		return _temperature;
	}

	void setValue(temperature newTemp) {
		_temperature = newTemp;
	}

	private:
	temperature _temperature{};
	bool _connected{};
};

#endif
