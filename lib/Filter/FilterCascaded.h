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


#include "FilterFixed.h"

#include <TemperatureFormats.h>

// Use 3 filter sections. This gives excellent filtering, without adding too much delay.
// For 3 sections the stop band attenuation is 3x the single section attenuation in dB.
// The delay is also tripled.

class CascadedFilter {
public:
    CascadedFilter();

    void init(temperature val);
    void setCoefficients(uint8_t bValue);

    // adds a value and returns the most recent filter output
    temperature add(temperature val);
    temperature_precise addDoublePrecision(temperature_precise val);

    // returns the most recent filter input
    [[nodiscard]] temperature readInput() const;
    [[nodiscard]] temperature readOutput() const;

    [[nodiscard]] temperature_precise readOutputDoublePrecision() const;
    [[nodiscard]] temperature_precise readPrevOutputDoublePrecision() const;

    // detect peaks in last section
    [[nodiscard]] temperature detectPosPeak() const;
    [[nodiscard]] temperature detectNegPeak() const;

private:
    static constexpr uint8_t numberOfSections = 3;
    FixedFilter sections[numberOfSections];
};

#endif
