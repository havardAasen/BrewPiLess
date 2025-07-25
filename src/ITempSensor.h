/*
 * Copyright 2012-2013 BrewPi/Elco Jacobs.
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

#ifndef ITEMPSENSOR_H
#define ITEMPSENSOR_H

#include "TemperatureFormats.h"


#define TEMP_SENSOR_DISCONNECTED INVALID_TEMP


class ITempSensor
{
public:
    virtual ~ITempSensor() = default;

    [[nodiscard]] virtual bool isConnected() const = 0;

    /** Attempt to (re-)initialize the sensor. */
    virtual bool init() = 0;

    /** Fetch a new reading from the sensor. */
    [[nodiscard]] virtual temperature read() = 0;
};

#endif
