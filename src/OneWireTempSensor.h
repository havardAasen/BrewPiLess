/*
 * Copyright 2012-2013 BrewPi/Elco Jacobs.
 * Copyright 2013 Matthew McGowan
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

#ifndef ONEWIRE_TEMP_SENSOR_H
#define ONEWIRE_TEMP_SENSOR_H

#include "Brewpi.h"
#include "TempSensor.h"
#include "FastDigitalPin.h"
#include "Ticks.h"

#include <DallasTemperature.h>

class OneWire;

#define ONEWIRE_TEMP_SENSOR_PRECISION (4)

class OneWireTempSensor : public ITempSensor {
public:
	/**
	 * @brief Constructs a new OneWire temperature sensor.
	 * @param bus The onewire bus this sensor is on.
	 * @param address The onewire address for this sensor. If all bytes are 0 in the address,
	 *                the first temp sensor on the bus is used.
	 * @param calibrationOffset A temperature value that is added to all readings. This can be
	 *                          used to calibrate the sensor.
	 */
	OneWireTempSensor(OneWire* bus, DeviceAddress address, fixed4_4 calibrationOffset);

	~OneWireTempSensor() override;

	[[nodiscard]] bool isConnected() const override{
		return connected_;
	}

	/**
	 * @brief Initializes the temperature sensor.
	 *
	 * This method is called when the sensor is first created and also any time the sensor reports
	 * it's disconnected. If the result is @ref TEMP_SENSOR_DISCONNECTED then subsequent calls to
	 * read() will also return @ref TEMP_SENSOR_DISCONNECTED. Clients should attempt to
	 * re-initialize the sensor by calling init() again.
	 */
	bool init() override;
	[[nodiscard]] temperature read() override;

	private:

	void setConnected(bool connected);
	bool requestConversion();
	void waitForConversion()
	{
		wait.millis(750);
	}

	/**
	* @brief Read the temperature and constrain it to a specific range.
	* @return Temperature in the specified range, or @ref TEMP_SENSOR_DISCONNECTED if the sensor
	*         is not connected.
	*/
	temperature readAndConstrainTemp();

	OneWire * oneWire;
	DallasTemperature * sensor{};
	DeviceAddress sensorAddress{};

	fixed4_4 calibrationOffset;
	bool connected_{true};

};

#endif
