/*
 * File:   SensorArduinoPin.h
 * Author: mat
 *
 * Created on 22 August 2013, 19:36
 */

#ifndef SENSOR_ARDUINO_PIN_H
#define SENSOR_ARDUINO_PIN_H

#ifdef ARDUINO

#include "Brewpi.h"
#include "FastDigitalPin.h"
#include "Pins.h"

/* A SwitchSensor whose state is provided by a hardware pin.
  By using a template, the compiler can inline and optimize the call to digitalRead to a single instruction.
*/
template<uint8_t pin, bool invert, bool internalPullup>
class DigitalConstantPinSensor : public SwitchSensor
{
	public:
	DigitalConstantPinSensor() {
		fastPinMode(pin, internalPullup ? INPUT_PULLUP : INPUT);
	}

	bool sense() override {
		return fastDigitalRead(pin) ^ invert;
	}
};

class DigitalPinSensor : public SwitchSensor
{
private:
	bool invert;
	uint8_t pin;



public:

	DigitalPinSensor(uint8_t pin, bool invert)
	{
		pinMode(pin, USE_INTERNAL_PULL_UP_RESISTORS ? INPUT_PULLUP : INPUT);
		this->invert = invert;
		this->pin = pin;
	}

	bool sense() override {
		return digitalRead(pin) ^ invert;
	}
};

#endif

#endif
