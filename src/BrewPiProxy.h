#ifndef BREW_PI_PROXY_H
#define BREW_PI_PROXY_H

#include <cstdint>
#include <functional>

#define BUFF_SIZE 1024

#define LCD_CMD 'l'


enum Mode : std::uint8_t;
enum State : std::uint8_t;

class BrewPiProxy{
public:
	void begin(const std::function<void(const char*)> &readString);

	void loop();
	void write(char ch);

	void putLine(const char* str);

	void getTemperature(float &beerTemp, float &beerSet, float &fridgeTemp, float &fridgeSet);
	void getTemperatureSetting(char &unit, float &minSetTemp, float &maxSetTemp);
	void getControlParameter(char &unit, Mode &mode, float &beerSet, float &fridgeSet);
	void getLogInfo(char &unit, std::uint8_t &mode, std::uint8_t &state);
	void getAllStatus(State &state, Mode &mode, float &beerTemp, float &beerSet, float &fridgeTemp,
	                  float &fridgeSet, float &roomTemp);

	bool ambientSensorConnected();

protected:
	char _unit{'C'};
	int  _lastLineLength{};
	char _buff[BUFF_SIZE]{};
	int   _readPtr{};

	std::function<void(const char*)> _readString;
};
extern BrewPiProxy brewPi;
#endif
