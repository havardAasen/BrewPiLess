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

#include "TemperatureFormats.h"

#include <algorithm>
#include <cstdio>
#include <cstring>

#ifdef ESP8266
// Appears this isn't defined in the ESP8266 implementation
char *
strchrnul(const char *s, int c_in)
{
	char c = c_in;
	while (*s && (*s != c))
		s++;

	return (char *)s;
}
#endif

//new  ESP8266_ONE
char * fixedPointToString(char * s, temperature rawValue, uint8_t numDecimals, uint8_t maxLength){
	return fixedPointToString(s, long_temperature(rawValue), numDecimals, maxLength);
}

char * fixedPointToString(char * s, long_temperature rawValue, uint8_t numDecimals, uint8_t maxLength){
	s[0] = ' ';
	if(rawValue < 0l){
		s[0] = '-';
		rawValue = -rawValue;
	}

	int intPart = longTempDiffToInt(rawValue); // rawValue is supposed to be without internal offset
	uint16_t fracPart;
	const char* fmt;
	uint16_t scale;
	switch (numDecimals)
	{
		case 1:
			fmt = "%d.%01d";
			scale = 10;
			break;
		case 2:
			fmt = "%d.%02d";
			scale = 100;
			break;
		default:
			fmt = "%d.%03d";
			scale = 1000;
	}
	fracPart = ((rawValue & TEMP_FIXED_POINT_MASK) * scale + TEMP_FIXED_POINT_SCALE/2) >> TEMP_FIXED_POINT_BITS; // add 256 for rounding
	if(fracPart >= scale){
		intPart++;
		fracPart = 0;
	}
	snprintf(&s[1], maxLength-1, fmt,  intPart, fracPart);
	return s;
}

long_temperature stringToFixedPoint(const char * numberString){
	// receive new temperature as null terminated string: "19.20"
	long_temperature intPart = 0;
	long_temperature fracPart = 0;

	char * fractPtr = nullptr; //pointer to the point in the string
	bool negative = 0;
	if(numberString[0] == '-'){
		numberString++;
		negative = true; // by processing the sign here, we don't have to include strtol
	}

	// find the point in the string to split in the integer part and the fraction part
	fractPtr = strchrnul(numberString, '.'); // returns pointer to the point.

	intPart = atol(numberString);
	if(fractPtr != nullptr){
		// decimal point was found
		fractPtr++; // add 1 to pointer to skip point
		int8_t numDecimals = (int8_t) strlen(fractPtr);
		fracPart = atol(fractPtr);
		fracPart = fracPart << TEMP_FIXED_POINT_BITS; // bits for fraction part
		while(numDecimals > 0){
			fracPart = (fracPart + 5) / 10; // divide by 10 rounded
			numDecimals--;
		}
	}
	long_temperature absVal = (intPart << TEMP_FIXED_POINT_BITS) + fracPart;
	return negative ? -absVal:absVal;
}

temperature constrainTemp16(const long_temperature val)
{
    return static_cast<temperature>(std::clamp(val, MIN_TEMP, MAX_TEMP));
}

temperature multiplyFactorTemperatureLong(temperature factor, long_temperature b)
{
	return constrainTemp16(((long_temperature) factor * (b-C_OFFSET))>>TEMP_FIXED_POINT_BITS);
}

temperature multiplyFactorTemperatureDiffLong(temperature factor, long_temperature b)
{
	return constrainTemp16(((long_temperature) factor * b)>>TEMP_FIXED_POINT_BITS);
}


temperature multiplyFactorTemperature(temperature factor, temperature b)
{
	return constrainTemp16(((long_temperature) factor * ((long_temperature) b - C_OFFSET))>>TEMP_FIXED_POINT_BITS);
}

temperature multiplyFactorTemperatureDiff(temperature factor, temperature b)
{
	return constrainTemp16(((long_temperature) factor * (long_temperature) b )>>TEMP_FIXED_POINT_BITS);
}
