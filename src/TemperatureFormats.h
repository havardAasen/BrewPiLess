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

#ifndef TEMPERATURE_FORMATS_H
#define TEMPERATURE_FORMATS_H

#include <cstdint>


// Offsets when converting to the internal format:
#define C_OFFSET (-24576) // this is also the default offset for the internal temp format
#define F_OFFSET (-33678)

// The internal fixed point format has 9 bits (512 steps) per degree. The range is -16 to 112C, an offset of -48C
// The communication over serial is in C or F and it is always converted to the internal fixed point format in C.
// From C to fixed point temp: T = (C-48)*512 = (C-48)<<9 = C<<9 - 48<<9 = C<<9 - 24576
// From F to fixed point temp: ((F-32)*5/9-48) * 512 = (F*5/9)<<9 - 33678
// From fixed point temp to C: C = T/512 + 48 = (T+24576)/512 = (T+24576)>>9
// From fixed point temp to F: F = (T/512 + 48)*9/5 + 32 = (T+24576)*9/5/512 + 32 = (T+33678)*9/5/512

// The interface to the Raspberry Pi uses decimal notation, like 21.3.
// Depending on the EEPROM setting cc.tempFormat, this will be interpreted as Celsius or Fahrenheit

// Addition and shifting can be done normally. When two fixed points values are multiplied, you have shift the result
using fixed7_9 = int16_t ; // fixed7_9 uses 7 signed int bits and 9 fraction bits
using fixed23_9 = int32_t ; // fixed23_9 uses 23 signed int bits and 9 fraction bits. Used when results can overflow
using fixed7_25 = int32_t ; // fixed7_25 uses 7 signed int bits and 25 fraction bits. Used when extra precision is needed
using fixed4_4 = int8_t ; // fixed4_4 uses 1-sign bit, 3 int bits and 4 fraction bits. Corresponds with precision of DS18B20 sensors

#define INVALID_TEMP -32768
#define MAX_TEMP 32767
#define MIN_TEMP (INVALID_TEMP + 1)

#define INVALID_TEMP_FLOAT -200.0
#define IS_FLOAT_TEMP_VALID(a) ((a) > INVALID_TEMP_FLOAT)



/* Temperature expressed as an integer. */
using temperature = fixed7_9;
using long_temperature = fixed23_9;
using temperature_precise = fixed7_25;

#define TEMP_FIXED_POINT_BITS (9)
#define TEMP_FIXED_POINT_SCALE (1<<TEMP_FIXED_POINT_BITS)
#define TEMP_FIXED_POINT_MASK (TEMP_FIXED_POINT_SCALE-1)
#define TEMP_PRECISE_EXTRA_FRACTION_BITS 16

#if 0

inline int16_t longTempDiffToInt(long_temperature val) {
    return int16_t((val) >> TEMP_FIXED_POINT_BITS);
}

inline temperature intToTemp(int8_t val) {
    return (temperature(val) << TEMP_FIXED_POINT_BITS) +C_OFFSET;
}

inline temperature intToTempDiff(int8_t val) {
    return (temperature(val) << TEMP_FIXED_POINT_BITS);
}

inline temperature doubleToTemp(double temp) {
    return (temp * TEMP_FIXED_POINT_SCALE + C_OFFSET) >= MAX_TEMP ? MAX_TEMP : (temp * TEMP_FIXED_POINT_SCALE + C_OFFSET) <= MIN_TEMP ? MIN_TEMP : temperature(temp * TEMP_FIXED_POINT_SCALE + C_OFFSET);
}

inline temperature tempPreciseToRegular(temperature_precise val) {
    return val >> TEMP_PRECISE_EXTRA_FRACTION_BITS;
}

inline temperature_precise tempRegularToPrecise(temperature val) {
    return temperature_precise(val) << TEMP_PRECISE_EXTRA_FRACTION_BITS;
}
#else
#define longTempDiffToInt(val) ((val)>>TEMP_FIXED_POINT_BITS)


#define intToTemp(val) ((temperature(val)<<TEMP_FIXED_POINT_BITS) + C_OFFSET)
#define intToTempDiff(val) ((temperature(val)<<TEMP_FIXED_POINT_BITS))
#define doubleToTemp(temp) ((temp*TEMP_FIXED_POINT_SCALE + C_OFFSET)>=MAX_TEMP ? MAX_TEMP : (temp*TEMP_FIXED_POINT_SCALE + C_OFFSET)<=MIN_TEMP ? MIN_TEMP : temperature(temp*TEMP_FIXED_POINT_SCALE + C_OFFSET))
#define tempPreciseToRegular(val) (val>>TEMP_PRECISE_EXTRA_FRACTION_BITS)
#define tempRegularToPrecise(val) (temperature_precise(val)<<TEMP_PRECISE_EXTRA_FRACTION_BITS)

#endif

char * tempToString(char * s, long_temperature rawValue, uint8_t numDecimals, uint8_t maxLength);
temperature stringToTemp(const char * string);

char * tempDiffToString(char * s, long_temperature rawValue, uint8_t numDecimals, uint8_t maxLength);
temperature stringToTempDiff(const char * string);

char * fixedPointToString(char * s, long_temperature rawValue, uint8_t numDecimals, uint8_t maxLength);
char * fixedPointToString(char * s, temperature rawValue, uint8_t numDecimals, uint8_t maxLength);
long_temperature stringToFixedPoint(const char * numberString);

int fixedToTenths(long_temperature temperature);
temperature tenthsToFixed(int temperature);

temperature constrainTemp16(long_temperature val);


temperature multiplyFactorTemperatureLong(temperature factor, long_temperature b);
temperature multiplyFactorTemperatureDiffLong(temperature factor, long_temperature b);
temperature multiplyFactorTemperature(temperature factor, temperature b);
temperature multiplyFactorTemperatureDiff(temperature factor, temperature b);


long_temperature convertToInternalTempImpl(long_temperature rawTemp, bool addOffset);
long_temperature convertFromInternalTempImpl(long_temperature rawTemp, bool addOffset);

inline long_temperature convertToInternalTempDiff(long_temperature rawTempDiff) {
    return convertToInternalTempImpl(rawTempDiff, false);
}

inline long_temperature convertFromInternalTempDiff(long_temperature rawTempDiff) {
    return convertFromInternalTempImpl(rawTempDiff, false);
}

inline long_temperature convertToInternalTemp(long_temperature rawTemp) {
    return convertToInternalTempImpl(rawTemp, true);
}

inline long_temperature convertFromInternalTemp(long_temperature rawTemp) {
    return convertFromInternalTempImpl(rawTemp, true);
}
//new ESP8266_ONE
float temperatureFloatValue(temperature t);
//new
#define OPTIMIZE_TEMPERATURE_FORMATS 1 && OPTIMIZE_GLOBAL

#endif
