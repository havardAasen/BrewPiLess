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

#include "Brewpi.h"
#include "OneWireTempSensor.h"
#include "OneWire.h"
#include "OneWireDevices.h"
#include "PiLink.h"
#include "Ticks.h"
#include "TemperatureFormats.h"

#include <algorithm>
#include <DallasTemperature.h>

OneWireTempSensor::OneWireTempSensor(OneWire* bus, DeviceAddress address, const fixed4_4 calibrationOffset)
: oneWire(bus)
, calibrationOffset(calibrationOffset)
{
	memcpy(sensorAddress, address, sizeof(DeviceAddress));
};

OneWireTempSensor::~OneWireTempSensor(){
	delete sensor;
};

bool OneWireTempSensor::init()
{
	if (sensor==nullptr) {
		sensor = new(std::nothrow) DallasTemperature(oneWire);
		if (sensor==nullptr) {
			char addressString[17];
			printBytes(sensorAddress, 8, addressString);

			logErrorString(ERROR_SRAM_SENSOR, addressString);
			return false;
		}
	}

	logDebug("init onewire sensor");
	bool success = false;
	// This quickly tests if the sensor is connected and initializes the reset detection.
	// During the main TempControl loop, we don't want to spend many seconds
	// scanning each sensor since this brings things to a halt.
	if (sensor && requestConversion()) {
		logDebug("init onewire sensor - wait for conversion");
		waitForConversion();
		const temperature temp = readAndConstrainTemp();
		DEBUG_ONLY(logInfoIntStringTemp(INFO_TEMP_SENSOR_INITIALIZED, oneWirePin, addressString, temp));
		success = temp!=TEMP_SENSOR_DISCONNECTED && requestConversion();
	}
	setConnected(success);
	logDebug("init onewire sensor complete %d", success);
	return success;
}

bool OneWireTempSensor::requestConversion()
{
	const bool connected = sensor->requestTemperaturesByAddress(sensorAddress);
	setConnected(connected);
	return connected;
}

void OneWireTempSensor::setConnected(bool connected) {
	if (connected_==connected)
		return; // state is stays the same

	char addressString[17];
	printBytes(sensorAddress, 8, addressString);
	connected_ = connected;
	if(connected){
		//logInfoIntString(INFO_TEMP_SENSOR_CONNECTED, 0, addressString);
		logInfoIntString(INFO_TEMP_SENSOR_CONNECTED, oneWirePin, addressString);
	}
	else{
		//logWarningIntString(WARNING_TEMP_SENSOR_DISCONNECTED, 0, addressString);
		logWarningIntString(WARNING_TEMP_SENSOR_DISCONNECTED, oneWirePin, addressString);
	}
}

temperature OneWireTempSensor::read(){

	if (!connected_)
		return TEMP_SENSOR_DISCONNECTED;

	temperature temp = readAndConstrainTemp();
	requestConversion();
	return temp;
}

temperature OneWireTempSensor::readAndConstrainTemp()
{
	const long_temperature long_temp = sensor->getTemp(sensorAddress);
	if(long_temp == DEVICE_DISCONNECTED_RAW){
		setConnected(false);
		return TEMP_SENSOR_DISCONNECTED;
	}

	const auto temp = static_cast<temperature>(long_temp>>3);
	constexpr std::uint8_t shift = TEMP_FIXED_POINT_BITS-ONEWIRE_TEMP_SENSOR_PRECISION; // difference in precision between DS18B20 format and temperature adt
	constexpr std::int16_t lower = MIN_TEMP>>shift; // -1024
	constexpr std::int16_t upper = MAX_TEMP>>shift; // 1023
	return static_cast<temperature>(std::clamp(static_cast<temperature>(temp+calibrationOffset+(C_OFFSET>>shift)), lower, upper)<<shift);
}
