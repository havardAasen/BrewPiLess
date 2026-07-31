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
 *
 * You should have received a copy of the GNU General Public License
 * along with BrewPi.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "FilterFixed.h"
#include <cstdlib>
#include <TemperatureFormats.h>

FixedFilter::FixedFilter()
{
    setCoefficients(20);
}

void FixedFilter::init(const temperature val){
    xv[0] = val;
    xv[0] = tempRegularToPrecise(xv[0]); // 16 extra bits are used in the filter for the fraction part

    xv[1] = xv[0];
    xv[2] = xv[0];

    yv[0] = xv[0];
    yv[1] = xv[0];
    yv[2] = xv[0];
}

void FixedFilter::setCoefficients(const uint8_t bValue)
{
    a = bValue * 2 + 4;
    b = bValue;
}

temperature FixedFilter::add(const temperature val){
	const temperature_precise returnVal = addDoublePrecision(tempRegularToPrecise(val));
	return tempPreciseToRegular(returnVal);
}

temperature_precise FixedFilter::addDoublePrecision(const temperature_precise val){
	xv[2] = xv[1];
	xv[1] = xv[0];
	xv[0] = val;

	yv[2] = yv[1];
	yv[1] = yv[0];

	/* Implementation that prevents overflow as much as possible by order of operations: */
	yv[0] = ((yv[1] - yv[2]) + yv[1]) // expected value + 1*
	- (yv[1]>>b) + (yv[2]>>b) + // expected value +0*
	+ (xv[0]>>a) + (xv[1]>>(a-1)) + (xv[2]>>a) // expected value +(1>>(a-2))
	- (yv[2]>>(a-2)); // expected value -(1>>(a-2))

	return yv[0];
}

temperature FixedFilter::readInput() const
{
    return static_cast<temperature>(xv[0] >> 16);
}

temperature FixedFilter::readOutput() const
{
    return static_cast<temperature>(yv[0] >> 16);
}

temperature_precise FixedFilter::readOutputDoublePrecision() const
{
    return yv[0];
}

temperature_precise FixedFilter::readPrevOutputDoublePrecision() const
{
    return yv[1];
}

temperature FixedFilter::detectPosPeak() const
{
	if(yv[0] < yv[1] && yv[1] >= yv[2]){
		return tempPreciseToRegular(yv[1]);
	}

    return INVALID_TEMP;
}

temperature FixedFilter::detectNegPeak() const
{
	if(yv[0] > yv[1] && yv[1] <= yv[2]){
		return tempPreciseToRegular(yv[1]);
	}

    return INVALID_TEMP;
}
