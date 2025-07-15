/*
 * Copyright 2013 Matthew McGowan
 * Copyright 2013 BrewPi/Elco Jacobs.
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

#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

#include "Brewpi.h"

#include "Actuator.h"
#include "Sensor.h"
#include "ITempSensor.h"
#include "TempSensor.h"
#include "OneWireDevices.h"
#include "Pins.h"
#include "EepromStructs.h"


/**
 * A user has freedom to connect various devices to the arduino, either via extending the oneWire bus, or by assigning to specific pins, e.g. actuators, switch sensors.
 * Rather than make this compile-time, the configuration is stored at runtime.
 * Also, the availability of various sensors will change. E.g. it's possible to have a fridge constant mode without a beer sensor.
 *
 * Since the data has to be persisted to EEPROM, references to the actual uses of the devices have to be encoded.  This is the function of the deviceID.
 */

class DeviceConfig;

using device_slot = int8_t;
inline bool isDefinedSlot(device_slot s) { return s>=0; }
inline constexpr device_slot MAX_DEVICE_SLOT = 16;		// exclusive
inline constexpr device_slot INVALID_SLOT = -1;


/** Describes where the device is most closely associated. */
enum class DeviceOwner {
	none=0,
	chamber=1,
	beer=2
};

enum class DeviceType {
	none = 0,
	tempSensor = 1,		///< OneWire temperature sensor
	switchSensor = 2,	///< direct pin and onewire are supported
	switchActuator = 3	///< both direct pin and onewire are supported
};


inline bool isAssignable(DeviceType type, DeviceHardware hardware)
{
	return (hardware==DeviceHardware::pin && (type == DeviceType::switchActuator || type == DeviceType::switchSensor))
#if BREWPI_DS2413
	|| (hardware==DEVICE_HARDWARE_ONEWIRE_2413 && (type==DEVICETYPE_SWITCH_ACTUATOR || (DS2413_SUPPORT_SENSE && type==DEVICETYPE_SWITCH_SENSOR)))
#endif
#if BREWPI_EXTERNAL_SENSOR
	|| (hardware == DeviceHardware::externalSensor && type == DeviceType::tempSensor)
#endif
	|| (hardware == DeviceHardware::onewireTemp && type == DeviceType::tempSensor)
	|| (hardware == DeviceHardware::none && type == DeviceType::none);
}

inline bool isOneWire(DeviceHardware hardware) {
	return
#if BREWPI_DS2413
	hardware==DEVICE_HARDWARE_ONEWIRE_2413 ||
#endif
	hardware == DeviceHardware::onewireTemp;
}

inline bool isDigitalPin(DeviceHardware hardware) {
	return hardware == DeviceHardware::pin;
}

extern DeviceType deviceType(DeviceFunction id);

#if BREWPI_EXTERNAL_SENSOR
inline bool isExternalSensor(DeviceHardware hardware) {
	return hardware == DeviceHardware::externalSensor;
}
#endif

/**
 * Determines where this devices belongs.
 */
inline DeviceOwner deviceOwner(DeviceFunction id) {
	return id==0 ? DeviceOwner::none : id>=DEVICE_BEER_FIRST ? DeviceOwner::beer : DeviceOwner::chamber;
}


typedef void (*EnumDevicesCallback)(DeviceConfig*, void* pv);

struct EnumerateHardware
{
	int8_t hardware;	///< restrict the types of devices requested
	int8_t pin;		///< pin to search
	int8_t values;		///< fetch values for the devices.
	int8_t unused;		///< 0 don't care about unused state, 1 unused only.
	int8_t function;	///< restrict to devices that can be used with this function
};

struct DeviceOutput
{
	device_slot	slot;
	char value[10];
};

struct DeviceDisplay {
	int8_t id{-1};		///< -1 for all devices, >=0 for specific device
	int8_t value{-1};	///< set value
	int8_t write{-1};	///< write value
	int8_t empty{}; 	///< show unused devices when id==-1, default is 0
};

void HandleDeviceDisplay(const char* key, const char* value, void* pv);

/**
 * Reads or writes a value to a device.
 */
void UpdateDeviceState(DeviceDisplay& dd, DeviceConfig& dc, char* val);

class OneWire;

class DeviceManager
{
public:

	bool isDefaultTempSensor(ITempSensor* sensor);

	int8_t enumerateActuatorPins(uint8_t offset)
	{
#if BREWPI_ACTUATOR_PINS && defined(ARDUINO)
#if BREWPI_STATIC_CONFIG<=BREWPI_SHIELD_REV_A
		switch (offset) {
			case 0: return heatingPin;
			case 1: return coolingPin;
			default:
				return -1;
		}
#elif BREWPI_STATIC_CONFIG>=BREWPI_SHIELD_REV_C
		switch (offset) {
			case 0: return actuatorPin1;
			case 1: return actuatorPin2;
			case 2: return actuatorPin3;
			case 3: return actuatorPin4;
			default: return -1;
		}
#endif
#endif
		return -1;
	}

	int8_t enumerateSensorPins(uint8_t offset) {
#if BREWPI_SENSOR_PINS && defined(ARDUINO)
		if (offset==0)
			return doorPin;
#endif
		return -1;
	}

	/* Enumerates the 1-wire pins.
	 *
	 */
	int8_t enumOneWirePins(uint8_t offset)
	{
#ifdef ARDUINO
#ifdef oneWirePin
		if (offset == 0)
			return oneWirePin;
#elif defined(beerSensorPin) && defined(fridgeSensorPin)
		if (offset==0)
			return beerSensorPin;
		if (offset==1)
			return fridgeSensorPin;
#endif
#endif
		return -1;
	}

	static void setupUnconfiguredDevices();

	/**
	 * Creates and Installs a device from the given device config.
	 * /return true if a device was installed. false if the config is not complete.
	 */
	static void installDevice(DeviceConfig& config);

	static void uninstallDevice(DeviceConfig& config);

	static void parseDeviceDefinition();
	static void printDevice(device_slot slot, DeviceConfig& config, const char* value);

	/**
	 * Iterate over the defined devices.
	 * Caller first calls with deviceIndex 0. If the return value is true, config is filled out with the
	 * config for the device. The caller can then increment deviceIndex and try again.
	 */
	static bool allDevices(DeviceConfig& config, uint8_t deviceIndex);

	static bool isDeviceValid(DeviceConfig& config, DeviceConfig& original, uint8_t deviceIndex);

	/**
	 * read hardware spec from stream and output matching devices
	 */
	static void enumerateHardware();

	static bool enumDevice(DeviceDisplay& dd, DeviceConfig& dc, uint8_t idx);

	static void listDevices();

private:
	#if BREWPI_EXTERNAL_SENSOR //vito: enumerate device
	static void enumerateExternalDevices(EnumerateHardware& h, EnumDevicesCallback callback, DeviceOutput& output);
	#endif
	static void enumerateOneWireDevices(EnumerateHardware& h, EnumDevicesCallback callback, DeviceOutput& output);
	static void enumeratePinDevices(EnumerateHardware& h, EnumDevicesCallback callback, DeviceOutput& output);
	static void OutputEnumeratedDevices(DeviceConfig* config, void* pv);
	static void handleEnumeratedDevice(DeviceConfig& config, EnumerateHardware& h, EnumDevicesCallback callback, DeviceOutput& out);
	static void readTempSensorValue(DeviceConfig::Hardware hw, char* out);


	static void* createDevice(DeviceConfig& config, DeviceType dc);
	static void* createOneWireGPIO(DeviceConfig& config, DeviceType dt);

	static void beginDeviceOutput() { firstDeviceOutput = true; }

	static OneWire* oneWireBus(uint8_t pin);

#ifdef ARDUINO

// There is no reason to separate the OneWire busses - if we have a single bus, use it.
#ifdef oneWirePin
	static OneWire primaryOneWireBus;
#else
	static OneWire beerSensorBus;
	static OneWire fridgeSensorBus;
#endif

#endif
	static bool firstDeviceOutput;
};


extern DeviceManager deviceManager;

#endif
