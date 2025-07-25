#ifndef BREW_LOGGER_H
#define BREW_LOGGER_H

#include <FS.h>
#include "BPLSettings.h"
#include "TimeKeeper.h"
#include <array>

#define INVALID_RECOVERY_TIME 0xFF
#define INVALID_TEMPERATURE -250
#define INVALID_GRAVITY -1

#define LOG_PATH "/log"

#define LogBufferSize 1024

// Log tags
#define StartLogTag 0xFF
#define ResumeBrewTag 0xFE

#define PeriodTag 0xF0
#define StateTag 0xF1
#define EventTag 0xF2
#define CorrectionTempTag 0xF3
#define ModeTag 0xF4

#define FillTag 0xF7
#define OriginGravityTag 0xF8
#define CalibrationPointTag 0xF9
#define IgnoredCalPointMaskTag 0xFA


#define INVALID_TEMP_INT 0x7FFF
#define INVALID_GRAVITY_INT 0x7FFF
#define VolatileHeaderSize 28

#define OrderBeerSet 0
#define OrderBeerTemp 1
#define OrderFridgeTemp 2
#define OrderFridgeSet 3
#define OrderRoomTemp 4
#define OrderExtTemp 5
#define OrderGravity 6

#define NumberDataBitMask 7

#undef NumberDataBitMask
#define NumberDataBitMask 8

#define OrderTiltAngle 7
#define TiltEncode(g) (uint16_t)(100.0 * (g) + 0.5)
#define INVALID_TILT_ANGLE 0x7FFF

#define GravityEncode(g) (uint16_t)(10000.0 * (g) + 0.5)
#define GravityDecode(a) (float)(a)/10000.0
#define PlatoEncode(g) (uint16_t)(100.0 * (g) + 0.5)
#define PlatoDecode(a) (float)(a)/100.0

#define HighOctect(a) (uint8_t)((a)>>8) 
#define LowOctect(a) (uint8_t)((a)&0xFF)

class BrewLogger
{

public:
	bool begin();

	String fsinfo();
	const char* currentLog();

	String loggingStatus();
	void rmLog(int index);
	bool isLogging(){ return _recording; }

	bool startSession(const char *filename,bool calibrating);
	void endSession();
	bool resumeSession();

	void loop();
	void logData();

	size_t beginCopyAfter(size_t last);
	size_t read(uint8_t *buffer, size_t maxLen, size_t index);
	void getFilePath(char* buf,int index);
	// read data
	size_t volatileDataOffset();
	size_t volatileDataAvailable(size_t start,size_t offset);
	size_t readVolatileData(uint8_t *buffer, size_t maxLen, size_t index);
	// add data
	void addGravity(float gravity,bool isOg=false);
	void addAuxTemp(float temp);
	void addTiltAngle(float tilt);
	void addCorrectionTemperature(float temp);
	void addTiltInWater(float tilt,float reading);
	bool isCalibrating(){ return _calibrating;}
	void addIgnoredCalPointMask(uint32_t mask);
	//format file system
	void onFormatFS();
private:
	size_t _fsspace{};
	uint32_t  _tempLogPeriod{60000};
	uint32_t _lastTempLog{};
	uint32_t _resumeLastLogTime{};

	bool _recording{};
	bool _calibrating{};

	size_t _logIndex{};
	size_t _savedLength{};
	size_t _lastRead{};
	char _logBuffer[LogBufferSize]{};

	File    _logFile;

	// brewpi specific info
	uint8_t _mode{};
	uint8_t _state{};
	bool _usePlato{};

	std::array<std::uint16_t, 5> _iTempData{INVALID_TEMP_INT};
	uint16_t  _extTemp{INVALID_TEMP_INT};
	uint16_t  _extGravity{INVALID_GRAVITY_INT};
	uint16_t  _extOriginGravity{INVALID_GRAVITY_INT};
	uint16_t  _extTileAngle{INVALID_TILT_ANGLE};

	// for circular buffer
	int _logHead{};
	uint32_t _headTime{};
	uint32_t _startOffset{};
	bool _sendHeader{};
	uint32_t _sendOffset{};
	FileIndexes *_pFileInfo{};

	#define VolatileDataHeaderSize 7
	uint16_t  _headData[VolatileDataHeaderSize]{};

	void resetTempData();
	void checkspace();

	void volatileHeader(char *buf);

	void startLog(bool fahrenheit,bool calibrating);
	void startVolatileLog();
	int freeBufferSpace();
	void dropData();
	int volatileLoggingAlloc(int size);
	int allocByte(byte size);
	void writeBuffer(int idx,uint8_t data);
	void commitData(int idx,int len);
	void addOG(uint16_t og);
	void addMode(uint8_t mode);
	void addState(uint8_t state);
	uint16_t convertTemperature(float temp);
	void addResumeTag();

	void loadIdxFile();
	void saveIdxFile();
};

extern BrewLogger brewLogger;
#endif
