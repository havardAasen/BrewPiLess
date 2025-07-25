/*
* Copyright 2016 John Beeler
*
* This is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this file.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ESP_EEPROM_ACCESS_H
#define ESP_EEPROM_ACCESS_H

#ifndef ESP8266
// Generate an error if we have been incorrectly included in an Arduino build
#error Incorrect processor type!
#endif

#include <EEPROM.h>
#include "EepromStructs.h"


//TODO - Clean this up
class ESPEepromAccess
{
public:
	static uint8_t readByte(eptr_t offset) {
		return EEPROM.read(offset);
	}
	static void writeByte(eptr_t offset, uint8_t value) {
		EEPROM.write(offset, value);
	}

	static void readControlSettings(ControlSettings& target, eptr_t offset, uint16_t size) {
		EEPROM.get(offset, target);
	}

	static void readControlConstants(ControlConstants& target, eptr_t offset, uint16_t size) {
		EEPROM.get(offset, target);
	}

	static void readDeviceDefinition(DeviceConfig& target, eptr_t offset, uint16_t size) {
		EEPROM.get(offset, target);
	}

	static void writeControlSettings(eptr_t target, ControlSettings& source, uint16_t size) {
		EEPROM.put(target, source);

		EEPROM.commit();
	}

	static void writeControlConstants(eptr_t target, ControlConstants& source, uint16_t size) {
		EEPROM.put(target, source);

		EEPROM.commit();
	}

	static void writeDeviceDefinition(eptr_t target, const DeviceConfig& source, uint16_t size) {
		EEPROM.put(target, source);

		EEPROM.commit();
	}

	static void commit() {
		EEPROM.commit();
	}
};

#endif
