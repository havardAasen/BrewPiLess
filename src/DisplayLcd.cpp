/*
 * Copyright 2012-2013 BrewPi/Elco Jacobs.
 *
 * This file is part of BrewPi.
 *
 * BrewPi is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later v7ersion.
 *
 * BrewPi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with BrewPi.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Brewpi.h"
#include "BrewpiStrings.h"
#include <limits>
#include <stdint.h>

#include "Display.h"
#include "DisplayLcd.h"
#include "Menu.h"
#include "TempControl.h"
#include "TemperatureConversion.h"
#include "Pins.h"

#include <TemperatureFormats.h>


uint8_t LcdDisplay::stateOnDisplay;
uint8_t LcdDisplay::flags;

#if BREWPI_OLED128x64_LCD
LcdDriver LcdDisplay::lcd(OLED128x64_LCD_ADDRESS,PIN_SDA,PIN_SCL);
#else // #if BREWPI_OLED128x64_LCD
#if BREWPI_IIC_LCD
LcdDriver LcdDisplay::lcd(IIC_LCD_ADDRESS,20,4);
#else
LcdDriver LcdDisplay::lcd;
#endif
#endif // #if BREWPI_OLED128x64_LCD
// Constant strings used multiple times
constexpr char STR_Beer_[] PROGMEM = "Beer ";
constexpr char STR_Fridge_[] PROGMEM = "Fridge ";
constexpr char STR_Const_[] PROGMEM = "Const.";
constexpr char STR_Cool[] PROGMEM = "Cool";
constexpr char STR_Heat[] PROGMEM = "Heat";
constexpr char STR_ing_for[] PROGMEM = "ing for";
constexpr char STR_Wait_to_[] PROGMEM = "Wait to ";
constexpr char STR__time_left[] PROGMEM = " time left";
constexpr char STR_empty_string[] PROGMEM = "";

void LcdDisplay::init(){
#ifdef BREWPI_IIC_LCD
//	Wire.begin(PIN_SDA,PIN_SCL);
#endif
	stateOnDisplay = 0xFF; // set to unknown state to force update
	flags = LCD_FLAG_ALTERNATE_ROOM;
	lcd.init(); // initialize LCD
	lcd.begin(20, 4);
	lcd.clear();
}


//print all temperatures on the LCD
void LcdDisplay::printAllTemperatures(){
	// alternate between beer and room temp
	if (flags & LCD_FLAG_ALTERNATE_ROOM) {
		bool displayRoom = ((ticks.seconds()&0x08)==0) && !BREWPI_SIMULATE && tempControl.ambientSensor->isConnected();
		if (displayRoom ^ ((flags & LCD_FLAG_DISPLAY_ROOM)!=0)) {	// transition
			flags = displayRoom ? flags | LCD_FLAG_DISPLAY_ROOM : flags & ~LCD_FLAG_DISPLAY_ROOM;
			printStationaryText();
		}
	}

	printBeerTemp();
	printBeerSet();
	printFridgeTemp();
	printFridgeSet();
}

void LcdDisplay::setDisplayFlags(uint8_t newFlags) {
	flags = newFlags;
	printStationaryText();
	printAllTemperatures();
}



void LcdDisplay::printBeerTemp(){
	printTemperatureAt(6, 1, tempControl.getBeerTemp());
}

void LcdDisplay::printBeerSet(){
	temperature beerSet = tempControl.getBeerSetting();
	printTemperatureAt(12, 1, beerSet);
}

void LcdDisplay::printFridgeTemp(){
	printTemperatureAt(6,2, flags & LCD_FLAG_DISPLAY_ROOM ?
		tempControl.ambientSensor->read() :
		tempControl.getFridgeTemp());
}

void LcdDisplay::printFridgeSet(){
	temperature fridgeSet = tempControl.getFridgeSetting();
	if(flags & LCD_FLAG_DISPLAY_ROOM) // beer setting is not active
		fridgeSet = INVALID_TEMP;
	printTemperatureAt(12, 2, fridgeSet);
}

void LcdDisplay::printTemperatureAt(uint8_t x, uint8_t y, temperature temp){
	lcd.setCursor(x,y);
	printTemperature(temp);
}


void LcdDisplay::printTemperature(temperature temp){
	if (temp==INVALID_TEMP) {
		lcd.print_P(PSTR(" --.-"));
		return;
	}
	char tempString[9];
	tempToString(tempString, temp, 1 , 9);
	int8_t spacesToWrite = 5 - (int8_t) strlen(tempString);
	for(int8_t i = 0; i < spacesToWrite ;i++){
		lcd.write(' ');
	}
	lcd.print(tempString);
}

//print the stationary text on the lcd.
void LcdDisplay::printStationaryText(){
	printAt_P(0, 0, PSTR("Mode"));
	printAt_P(0, 1, STR_Beer_);
	printAt_P(0, 2, (flags & LCD_FLAG_DISPLAY_ROOM) ?  PSTR("Room  ") : STR_Fridge_);
	printDegreeUnit(18, 1);
	printDegreeUnit(18, 2);
}

//print degree sign + temp unit
void LcdDisplay::printDegreeUnit(uint8_t x, uint8_t y){
	lcd.setCursor(x,y);
	lcd.write(0b11011111);
	lcd.write(tempControl.cc.tempFormat);
}

void LcdDisplay::printAt_P(uint8_t x, uint8_t y, const char* text){
	lcd.setCursor(x, y);
	lcd.print_P(text);
}


void LcdDisplay::printAt(uint8_t x, uint8_t y, char* text){
	lcd.setCursor(x, y);
	lcd.print(text);
}

// print mode on the right location on the first line, after "Mode   "
void LcdDisplay::printMode(){
	lcd.setCursor(7,0);
	// Factoring prints out of switch has negative effect on code size in this function
	switch(tempControl.getMode()){
		case fridge_constant:
			lcd.print_P(STR_Fridge_);
			lcd.print_P(STR_Const_);
			break;
		case beer_constant:
			lcd.print_P(STR_Beer_);
			lcd.print_P(STR_Const_);
			break;
		case beer_profile:
			lcd.print_P(STR_Beer_);
			lcd.print_P(PSTR("Profile"));
			break;
		case off:
			lcd.print_P(PSTR("Off"));
			break;
		case test:
			lcd.print_P(PSTR("** Testing **"));
			break;
		default:
			lcd.print_P(PSTR("Invalid mode"));
			break;
	}
	lcd.printSpacesToRestOfLine();
}

// print the current state on the last line of the lcd
void LcdDisplay::printState(){
	uint16_t time = std::numeric_limits<std::uint16_t>::max();
	const State state = tempControl.getDisplayState();
	if(state != stateOnDisplay){ //only print static text when state has changed
		stateOnDisplay = state;
		// Reprint state and clear rest of the line
		const char * part1 = STR_empty_string;
		const char * part2 = STR_empty_string;
		switch (state){
			case idle:
				part1 = PSTR("Idl");
				part2 = STR_ing_for;
				break;
			case waiting_to_cool:
				part1 = STR_Wait_to_;
				part2 = STR_Cool;
				break;
			case waiting_to_heat:
				part1 = STR_Wait_to_;
				part2 = STR_Heat;
				break;
			case waiting_for_peak_detect:
				part1 = PSTR("Waiting for peak");
				break;
			case cooling:
				part1 = STR_Cool;
				part2 = STR_ing_for;
				break;
			case heating:
				part1 = STR_Heat;
				part2 = STR_ing_for;
				break;
			case cooling_min_time:
				part1 = STR_Cool;
				part2 = STR__time_left;
				break;
			case heating_min_time:
				part1 = STR_Heat;
				part2 = STR__time_left;
				break;
			case door_open:
				part1 = PSTR("Door open");
				break;
			case state_off:
				part1 = PSTR("Temp. control OFF");
				break;
			default:
				part1 = PSTR("Unknown status!");
				break;
		}
		printAt_P(0, 3, part1);
		lcd.print_P(part2);
		lcd.printSpacesToRestOfLine();
	}
	uint16_t sinceIdleTime = tempControl.timeSinceIdle();
	if(state==idle){
		time = 	min(tempControl.timeSinceCooling(), tempControl.timeSinceHeating());
	}
	else if(state==cooling || state==heating){
		time = sinceIdleTime;
	}
	else if(state==cooling_min_time){
		#if SettableMinimumCoolTime
		time =(tempControl.cc.minCoolTime > sinceIdleTime)? (tempControl.cc.minCoolTime -sinceIdleTime):0;
		#else
		time = MIN_COOL_ON_TIME-sinceIdleTime;
		#endif
	}

	else if(state==heating_min_time){
		#if SettableMinimumCoolTime
		time = (tempControl.cc.minHeatTime > sinceIdleTime)? (tempControl.cc.minHeatTime-sinceIdleTime):0;
		#else
		time = MIN_HEAT_ON_TIME-sinceIdleTime;
		#endif
	}
	else if(state == waiting_to_cool || state == waiting_to_heat){
		time = tempControl.getWaitTime();
	}
	if(time != std::numeric_limits<std::uint16_t>::max()){
		char timeString[10];
#if DISPLAY_TIME_HMS  // 96 bytes more space required.
		unsigned int minutes = time/60;
		unsigned int hours = minutes/60;
		int stringLength = sprintf_P(timeString, PSTR("%dh%02dm%02d"), hours, minutes%60, time%60);
		char * printString = timeString;
		if(!hours){
			printString = &timeString[2];
			stringLength = stringLength-2;
		}
		printAt(20-stringLength, 3, printString);
#else
		int stringLength = sprintf_P(timeString, STR_FMT_U, (unsigned int)time);
		printAt(20-stringLength, 3, timeString);
#endif
	}
}
