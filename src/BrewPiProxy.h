#ifndef BREW_PI_PROXY_H
#define BREW_PI_PROXY_H

#include <Arduino.h>
#include "Config.h"

#define BUFF_SIZE 1024

#define LCD_CMD 'l'


enum Mode : uint8_t;

class BrewPiProxy{
public:
	void begin(void (*readString)(const char*));

	void loop();
	void write(char ch);

	void putLine(const char* str);

	char* getLastLine(){return _lastLineBuff;}

	void getTemperature(float *pBeerTemp,float *pBeerSet,float *pFridgeTemp, float *pFridgeSet);
	void getTemperatureSetting(char *pUnit,float *pMinSetTemp,float *pMaxSetTemp);
	void getControlParameter(char *pUnit,Mode *pMode,float *pBeerSet, float *pFridgeSet);
	void getLogInfo(char *pUnit,uint8_t *pMode,uint8_t *pState);
	void getAllStatus(uint8_t *pState,uint8_t *pMode,float *pBeerTemp,float *pBeerSet,float *pFridgeTemp, float *pFridgeSet, float *pRoomTemp);

	bool ambientSensorConnected();

protected:
	char _unit{'C'};
	char _lastLineBuff[BUFF_SIZE]{};
	int  _lastLineLength{};
	char _buff[BUFF_SIZE]{};
	int   _readPtr{};

	void (*_readString)(const char*){};
};
extern BrewPiProxy brewPi;
#endif
