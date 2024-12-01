/*
 * Copyright 2013 BrewPi/Elco Jacobs.
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

#include "Brewpi.h"
#include <stddef.h>

#include "EepromManager.h"
#include "TempControl.h"
#include "EepromFormat.h"
#include "PiLink.h"

EepromManager eepromManager;
EepromAccess eepromAccess;


bool EepromManager::hasSettings()
{
	uint8_t version = EepromAccess::readByte(pointerOffset(version));
	return (version==EEPROM_FORMAT_VERSION);
}

void EepromManager::resetEeprom()
{
	for (uint16_t offset=0; offset<EepromFormat::MAX_EEPROM_SIZE; offset++)
		EepromAccess::writeByte(offset, 0xFF);

	EepromAccess::commit();
}


void EepromManager::initializeEeprom()
{
	tempControl.loadDefaultConstants();
	tempControl.loadDefaultSettings();
	tempControl.storeConstantsAndSettings();

	// set the version flag - so that storeDevice will work
	EepromAccess::writeByte(0, EEPROM_FORMAT_VERSION);
	EepromAccess::commit();

	// set state to startup
	tempControl.init();
}


bool EepromManager::applySettings()
{
	if (!hasSettings())
		return false;

	logDebug("Applying settings");

	storeTempConstantsAndSettings();

	logDebug("Applied settings");

	applyDevices();

	return true;
}

void EepromManager::applyDevices()
{
    DeviceConfig deviceConfig;
    for (uint8_t index = 0; fetchDevice(deviceConfig, index); index++)
    {
        if (DeviceManager::isDeviceValid(deviceConfig, deviceConfig, index))
            DeviceManager::installDevice(deviceConfig);
        else {
            clear((uint8_t*)&deviceConfig, sizeof(deviceConfig));
            storeDevice(deviceConfig, index);
        }
    }
}

void EepromManager::storeTempConstantsAndSettings()
{
	eptr_t pv = pointerOffset(chambers);
	tempControl.storeConstants(pv+offsetof(ChamberBlock, chamberSettings.cc));

	storeTempSettings();
}

void EepromManager::storeTempSettings()
{
	eptr_t pv = pointerOffset(chambers);
	// for now assume just one beer.
	tempControl.storeSettings(pv+offsetof(ChamberBlock, beer[0].cs));
}

bool EepromManager::fetchDevice(DeviceConfig& config, uint8_t deviceIndex)
{
	bool ok = (hasSettings() && deviceIndex<EepromFormat::MAX_DEVICES);
	if (ok)
		EepromAccess::readDeviceDefinition(config, pointerOffset(devices)+sizeof(DeviceConfig)*deviceIndex, sizeof(DeviceConfig));
	return ok;
}

bool EepromManager::storeDevice(const DeviceConfig& config, uint8_t deviceIndex)
{
	bool ok = (hasSettings() && deviceIndex<EepromFormat::MAX_DEVICES);
	if (ok)
		EepromAccess::writeDeviceDefinition(pointerOffset(devices)+sizeof(DeviceConfig)*deviceIndex, config, sizeof(DeviceConfig));
	return ok;
}

void fill(int8_t* p, uint8_t size) {
	while (size-->0) *p++ = -1;
}
void clear(uint8_t* p, uint8_t size) {
	while (size-->0) *p++ = 0;
}
