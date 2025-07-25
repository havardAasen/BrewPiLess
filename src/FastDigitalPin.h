/*
 * Copyright 2013 BrewPi/Elco Jacobs.
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

#ifndef FAST_DIGITAL_PIN_H
#define FAST_DIGITAL_PIN_H

#ifndef FAST_DIGITAL_PIN
#define FAST_DIGITAL_PIN 1
#endif

// compiler optimization required in order to resolve pin numbers to compile time constants.
#define USE_FAST_DIGITAL_PIN FAST_DIGITAL_PIN && __OPTIMIZE__

#if USE_FAST_DIGITAL_PIN
	#include "DigitalPin.h"
#else
	#define fastPinMode pinMode
	#define fastDigitalWrite digitalWrite
	#define fastDigitalRead digitalRead
#endif

#endif
