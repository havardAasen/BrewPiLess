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

#include <EEPROM.h>
#include "EepromStructs.h"
#include "EepromTypes.h"


namespace bpl::EspEepromAccess
{
	inline void begin(size_t size) {
		EEPROM.begin(size);
	}

	inline uint8_t readByte(eptr_t offset) {
		return EEPROM.read(offset);
	}

	inline void writeByte(eptr_t offset, uint8_t value) {
		EEPROM.write(offset, value);
	}

	inline void readControlSettings(ControlSettings& target, eptr_t offset) {
		EEPROM.get(offset, target);
	}

	inline void readControlConstants(ControlConstants& target, eptr_t offset) {
		EEPROM.get(offset, target);
	}

	inline void readDeviceDefinition(DeviceConfig& target, eptr_t offset) {
		EEPROM.get(offset, target);
	}

	inline void writeControlSettings(eptr_t target, ControlSettings& source) {
		EEPROM.put(target, source);

		EEPROM.commit();
	}

	inline void writeControlConstants(eptr_t target, ControlConstants& source) {
		EEPROM.put(target, source);

		EEPROM.commit();
	}

	inline void writeDeviceDefinition(eptr_t target, const DeviceConfig& source) {
		EEPROM.put(target, source);

		EEPROM.commit();
	}

	inline void commit() {
		EEPROM.commit();
	}
};

#endif
