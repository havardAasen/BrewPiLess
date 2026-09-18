#include "BrewPiProxy.h"
#include "Display.h"
#include "EepromManager.h"
#include "TempControl.h"

#include <QueueBuffer.h>
#include <TemperatureFormats.h>

#include <functional>

QueueBuffer brewPiRxBuffer(2048);
QueueBuffer brewPiTxBuffer(2048);


void BrewPiProxy::write(char ch)
{
	brewPiRxBuffer.print(ch);
}

void BrewPiProxy::putLine(const char* str)
{
	brewPiRxBuffer.print(str);
	brewPiRxBuffer.print('\n');
}

void BrewPiProxy::begin(const std::function<void(const char*)> &readString)
{
	_readString = readString;
}

void BrewPiProxy::loop()
{
	while(brewPiTxBuffer.available()){
		char ch=brewPiTxBuffer.read();
		 if(ch == '\n'){
			_buff[_readPtr]='\0';
			_readString(_buff);
			_readPtr=0;
		}
		else
		if(ch == 0xB0){
			//for safty
			if(_readPtr < (BUFF_SIZE-2)){
				_buff[_readPtr++]=0xC2;
				_buff[_readPtr++]=0xB0;
			}
		 }
		 else
			 _buff[_readPtr++]=ch;
		if( _readPtr >= BUFF_SIZE) _readPtr=0; // just drop.
	}
}

void BrewPiProxy::getTemperature(float &beerTemp, float &beerSet, float &fridgeTemp, float &fridgeSet)
{
	beerTemp = temperatureFloatValue(tempControl.getBeerTemp());
	beerSet = temperatureFloatValue(tempControl.getBeerSetting());
	fridgeTemp = temperatureFloatValue(tempControl.getFridgeTemp());
	fridgeSet = temperatureFloatValue(tempControl.getFridgeSetting());
}

void BrewPiProxy::getControlParameter(char &unit, Mode &mode, float &beerSet, float &fridgeSet)
{
	unit = tempControl.cc.tempFormat;
	mode = tempControl.cs.mode;
	beerSet = temperatureFloatValue(tempControl.getBeerSetting());
	fridgeSet = temperatureFloatValue(tempControl.getFridgeSetting());

}

void BrewPiProxy::getTemperatureSetting(char &unit, float &minSetTemp, float &maxSetTemp)
{
	unit = tempControl.cc.tempFormat;
	minSetTemp = temperatureFloatValue(tempControl.cc.tempSettingMin);
	maxSetTemp = temperatureFloatValue(tempControl.cc.tempSettingMax);
}

void BrewPiProxy::getLogInfo(char &unit, std::uint8_t &mode, std::uint8_t &state)
{
	unit = tempControl.cc.tempFormat;
	state = (std::uint8_t) tempControl.getState();
	mode = (std::uint8_t) tempControl.getMode();
}

void BrewPiProxy::getAllStatus(State& state, Mode& mode, float &beerTemp, float &beerSet, float &fridgeTemp, float &fridgeSet, float
                               &roomTemp)
{
	beerTemp = temperatureFloatValue(tempControl.getBeerTemp());
	beerSet = temperatureFloatValue(tempControl.getBeerSetting());
	fridgeTemp = temperatureFloatValue(tempControl.getFridgeTemp());
	fridgeSet = temperatureFloatValue(tempControl.getFridgeSetting());
	roomTemp =temperatureFloatValue(tempControl.getRoomTemp());
	state = tempControl.getState();
	mode = tempControl.getMode();
}

bool BrewPiProxy::ambientSensorConnected()
{
	return tempControl.ambientSensor->isConnected();
}
