/*
 * Copyright 2012-2013 BrewPi/Elco Jacobs.
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

#ifndef FILTER_CASCADED_H
#define FILTER_CASCADED_H

#include "Brewpi.h"

#include "FilterFixed.h"

#include <TemperatureFormats.h>

// Use 3 filter sections. This gives excellent filtering, without adding too much delay.
// For 3 sections the stop band attenuation is 3x the single section attenuation in dB.
// The delay is also tripled.
#define NUM_SECTIONS 3

class CascadedFilter{
	public:
	// CascadedFilter implements a filter that consists of multiple second order secions.
	FixedFilter sections[NUM_SECTIONS];

	public:
	CascadedFilter();
	void init(temperature val);
	void setCoefficients(uint8_t bValue);
	temperature add(temperature val); // adds a value and returns the most recent filter output
	temperature_precise addDoublePrecision(temperature_precise val);
	temperature readInput(); // returns the most recent filter input

	temperature readOutput(){
		return sections[NUM_SECTIONS-1].readOutput(); // return output of last section
	}
	temperature_precise readOutputDoublePrecision();
	temperature_precise readPrevOutputDoublePrecision();

	temperature detectPosPeak(){
		return sections[NUM_SECTIONS-1].detectPosPeak(); // detect peaks in last section
	}
	temperature detectNegPeak(){
		return sections[NUM_SECTIONS-1].detectNegPeak(); // detect peaks in last section
	}
};

#endif
