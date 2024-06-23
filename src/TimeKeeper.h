#ifndef TimeKeeper_H
#define TimeKeeper_H

class TimeKeeperClass
{
public:
	void begin(char* server1,char* server2,char* server3);
	void begin();

	void updateTime();

	time_t getTimeSeconds(); // get Epoch time
	time_t getLocalTimeSeconds();
	
	const char *getDateTimeStr();

	void setCurrentTime(time_t current);
	void setTimezoneOffset(int32_t offset);
	int32_t getTimezoneOffset();
	[[nodiscard]] bool isSynchronized() const { return _ntpSynced; }
private:
	time_t _referenceEpoc{};
	time_t _referenceSystemTime{};
	bool _ntpSynced{};

	time_t _lastSaved{};
	void saveTime(time_t t);
	time_t loadTime();

	time_t _queryServer();
};

extern TimeKeeperClass TimeKeeper;

#endif
