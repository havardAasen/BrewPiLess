#ifndef TEMPERATURE_CONVERSION_H
#define TEMPERATURE_CONVERSION_H

#include <TemperatureFormats.h>

char * tempToString(char * s, long_temperature rawValue, uint8_t numDecimals, uint8_t maxLength);
temperature stringToTemp(const char * string);

char * tempDiffToString(char * s, long_temperature rawValue, uint8_t numDecimals, uint8_t maxLength);
temperature stringToTempDiff(const char * string);

int fixedToTenths(long_temperature temperature);
temperature tenthsToFixed(int temperature);

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

#endif
