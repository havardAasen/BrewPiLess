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


/**
 * Defines global config for the brewpi project. This file is included in every file in the project to ensure conditional
 * compilation directives are recognized.
 *
 * To customize the build, users may add settings to Config.h, or define symbols in the project.
 */

#ifndef BREWPI_H
#define BREWPI_H

// Most pins are only conditionally defined here, allowing definitions to be provided in Config.h for
// local overrides
#define BREWPI_SHIELD_DIY   0
#define BREWPI_SHIELD_REV_A	1
#define BREWPI_SHIELD_REV_C	2

#define BREWPI_BOARD_UNKNOWN '?'
#define BREWPI_BOARD_ESP8266 'e'

#include <Arduino.h>

#include "Config.h"

#endif
