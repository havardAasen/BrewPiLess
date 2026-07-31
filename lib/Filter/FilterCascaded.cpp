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

#include "FilterFixed.h"
#include "FilterCascaded.h"

#include <cstdlib>
#include <TemperatureFormats.h>

CascadedFilter::CascadedFilter()
{
    setCoefficients(2);
}

void CascadedFilter::init(const temperature val)
{
    for (auto &section: sections) {
        section.init(val);
    }
}

void CascadedFilter::setCoefficients(const uint8_t bValue)
{
    for (auto &section: sections) {
        section.setCoefficients(bValue);
    }
}

temperature CascadedFilter::add(const temperature val)
{
    temperature_precise valDoublePrecision = tempRegularToPrecise(val);
    valDoublePrecision = addDoublePrecision(valDoublePrecision);
    // return output, shifted back to single precision
    return tempPreciseToRegular(valDoublePrecision);
}

temperature_precise CascadedFilter::addDoublePrecision(const temperature_precise val)
{
    temperature_precise input = val;
    // input is input for next section, which is the output of the previous section
    for (auto &section: sections) {
        input = section.addDoublePrecision(input);
    }
    return input;
}

temperature CascadedFilter::readInput() const
{
    return sections[0].readInput();
}

temperature CascadedFilter::readOutput() const
{
    return sections[numberOfSections - 1].readOutput();
}

temperature_precise CascadedFilter::readOutputDoublePrecision() const
{
    return sections[numberOfSections - 1].readOutputDoublePrecision();
}

temperature_precise CascadedFilter::readPrevOutputDoublePrecision() const
{
    return sections[numberOfSections - 1].readPrevOutputDoublePrecision();
}

temperature CascadedFilter::detectPosPeak() const
{
    return sections[numberOfSections - 1].detectPosPeak();
}

temperature CascadedFilter::detectNegPeak() const
{
    return sections[numberOfSections - 1].detectNegPeak();
}
