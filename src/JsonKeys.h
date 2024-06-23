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

#pragma once

#include "Brewpi.h"

#ifdef ARDUINO
#if defined(ESP8266)
// There is no concept of PROGMEM with the ESP8266
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif
#endif

inline constexpr char JSONKEY_mode[] PROGMEM = "mode";
inline constexpr char JSONKEY_beerSetting[] PROGMEM = "beerSet";
inline constexpr char JSONKEY_fridgeSetting[] PROGMEM = "fridgeSet";
inline constexpr char JSONKEY_heatEstimator[] PROGMEM = "heatEst";
inline constexpr char JSONKEY_coolEstimator[] PROGMEM = "coolEst";

// constant;
inline constexpr char JSONKEY_tempFormat[] PROGMEM = "tempFormat";
inline constexpr char JSONKEY_tempSettingMin[] PROGMEM = "tempSetMin";
inline constexpr char JSONKEY_tempSettingMax[] PROGMEM = "tempSetMax";
inline constexpr char JSONKEY_pidMax[] PROGMEM = "pidMax";
inline constexpr char JSONKEY_Kp[] PROGMEM = "Kp";
inline constexpr char JSONKEY_Ki[] PROGMEM = "Ki";
inline constexpr char JSONKEY_Kd[] PROGMEM = "Kd";
inline constexpr char JSONKEY_iMaxError[] PROGMEM = "iMaxErr";
inline constexpr char JSONKEY_idleRangeHigh[] PROGMEM = "idleRangeH";
inline constexpr char JSONKEY_idleRangeLow[] PROGMEM = "idleRangeL";
inline constexpr char JSONKEY_heatingTargetUpper[] PROGMEM = "heatTargetH";
inline constexpr char JSONKEY_heatingTargetLower[] PROGMEM = "heatTargetL";
inline constexpr char JSONKEY_coolingTargetUpper[] PROGMEM = "coolTargetH";
inline constexpr char JSONKEY_coolingTargetLower[] PROGMEM = "coolTargetL";
inline constexpr char JSONKEY_maxHeatTimeForEstimate[] PROGMEM = "maxHeatTimeForEst";
inline constexpr char JSONKEY_maxCoolTimeForEstimate[] PROGMEM = "maxCoolTimeForEst";
inline constexpr char JSONKEY_fridgeFastFilter[] PROGMEM = "fridgeFastFilt";
inline constexpr char JSONKEY_fridgeSlowFilter[] PROGMEM = "fridgeSlowFilt";
inline constexpr char JSONKEY_fridgeSlopeFilter[] PROGMEM = "fridgeSlopeFilt";
inline constexpr char JSONKEY_beerFastFilter[] PROGMEM = "beerFastFilt";
inline constexpr char JSONKEY_beerSlowFilter[] PROGMEM = "beerSlowFilt";
inline constexpr char JSONKEY_beerSlopeFilter[] PROGMEM = "beerSlopeFilt";
inline constexpr char JSONKEY_lightAsHeater[] PROGMEM = "lah";
inline constexpr char JSONKEY_rotaryHalfSteps[] PROGMEM = "hs";
#if SettableMinimumCoolTime 
inline constexpr char JSONKEY_minCoolTime[] PROGMEM = "minCoolTime";
inline constexpr char JSONKEY_minCoolIdleTime[] PROGMEM = "minCoolIdleTime";
inline constexpr char JSONKEY_minHeatTime[] PROGMEM = "minHeatTime";
inline constexpr char JSONKEY_minHeatIdleTime[] PROGMEM = "minHeatIdleTime";
inline constexpr char JSONKEY_mutexDeadTime[] PROGMEM = "deadTime";
#endif
// variable;
inline constexpr char JSONKEY_beerDiff[] PROGMEM = "beerDiff";
inline constexpr char JSONKEY_diffIntegral[] PROGMEM = "diffIntegral";
inline constexpr char JSONKEY_beerSlope[] PROGMEM = "beerSlope";
inline constexpr char JSONKEY_p[] PROGMEM = "p";
inline constexpr char JSONKEY_i[] PROGMEM = "i";
inline constexpr char JSONKEY_d[] PROGMEM = "d";
inline constexpr char JSONKEY_estimatedPeak[] PROGMEM = "estPeak"; // current peak estimate
inline constexpr char JSONKEY_negPeakEstimate[] PROGMEM = "negPeakEst"; // last neg peak estimate before switching to idle
inline constexpr char JSONKEY_posPeakEstimate[] PROGMEM = "posPeakEst";
inline constexpr char JSONKEY_negPeak[] PROGMEM = "negPeak"; // last true neg peak
inline constexpr char JSONKEY_posPeak[] PROGMEM = "posPeak";

inline constexpr char JSONKEY_logType[] PROGMEM = "logType";
inline constexpr char JSONKEY_logID[] PROGMEM = "logID";
