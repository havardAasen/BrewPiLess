#include "TempControl.h"
#include "TemperatureConversion.h"

#include <TemperatureFormats.h>

float temperatureFloatValue(temperature t)
{
    if(t == INVALID_TEMP) return INVALID_TEMP_FLOAT;

    long_temperature rawValue = convertFromInternalTemp(t);

    float sign=1.0;
    if(rawValue < 0l){
        sign=-1.0;
        rawValue = -rawValue;
    }

    int intPart = longTempDiffToInt(rawValue); // rawValue is supposed to be without internal offset
    uint16_t fracPart;
    fracPart = ((rawValue & TEMP_FIXED_POINT_MASK) * 1000 + TEMP_FIXED_POINT_SCALE/2) >> TEMP_FIXED_POINT_BITS; // add 256 for rounding
    return sign *((float)intPart +(float)fracPart/1000.0);
}
//new

// See header file for details about the temp format used.

// result can have maximum length of : sign + 3 digits integer part + point + 3 digits fraction part + '\0' = 9 bytes;
// only 1, 2 or 3 decimals allowed.
// returns pointer to the string
// long_temperature is used to prevent overflow
char * tempToString(char * s, long_temperature rawValue, uint8_t numDecimals, uint8_t maxLength){
    if(rawValue == INVALID_TEMP){
        strcpy(s, "null");
        return s;
    }
    rawValue = convertFromInternalTemp(rawValue);
    return fixedPointToString(s, rawValue, numDecimals, maxLength);
}

char * tempDiffToString(char * s, long_temperature rawValue, uint8_t numDecimals, uint8_t maxLength){
    rawValue = convertFromInternalTempDiff(rawValue);
    return fixedPointToString(s, rawValue, numDecimals, maxLength);
}

temperature stringToTemp(const char * numberString){
    long_temperature rawTemp = stringToFixedPoint(numberString);
    rawTemp = convertToInternalTemp(rawTemp);
    return constrainTemp16(rawTemp);
}

temperature stringToTempDiff(const char * numberString){
    long_temperature rawTempDiff = stringToFixedPoint(numberString);
    rawTempDiff = convertToInternalTempDiff(rawTempDiff);
    return constrainTemp16(rawTempDiff);
}

int fixedToTenths(long_temperature temp){
    temp = convertFromInternalTemp(temp);
    return (int) ((10 * temp + intToTempDiff(5)/10) / intToTempDiff(1)); // return rounded result in tenth of degrees
}

temperature tenthsToFixed(int temp){
    long_temperature fixedPointTemp = convertToInternalTemp(((long_temperature) temp * intToTempDiff(1) + 5) / 10);
    return constrainTemp16(fixedPointTemp);
}

// convertToInternalTemp receives the external temp format in fixed point and converts it to the internal format
// It scales the value for Fahrenheit and adds the offset needed for absolute temperatures. For temperature differences, use no offset.
long_temperature convertToInternalTempImpl(long_temperature rawTemp, bool addOffset){
    if(tempControl.cc.tempFormat == 'F'){ // value received is in F, convert to C
        rawTemp = (rawTemp) * 5 / 9;
        if(addOffset){
            rawTemp += F_OFFSET;
        }
    }
    else{
        if(addOffset){
            rawTemp += C_OFFSET;
        }
    }
    return rawTemp;
}

// convertAndConstrain adds an offset, then scales with *9/5 for Fahrenheit. Use it without the offset argument for temperature differences
long_temperature convertFromInternalTempImpl(long_temperature rawTemp, bool addOffset){
    if(tempControl.cc.tempFormat == 'F'){ // value received is in F, convert to C
        if(addOffset){
            rawTemp -= F_OFFSET;
        }
        rawTemp = rawTemp * 9 / 5;
    }
    else{
        if(addOffset){
            rawTemp -= C_OFFSET;
        }
    }
    return rawTemp;
}
