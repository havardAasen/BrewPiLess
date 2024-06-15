#ifndef TimeKeeper_H
#define TimeKeeper_H

class TimeKeeperClass
{
public:
	TimeKeeperClass():_referenceSeconds(0),_referenceSystemTime(0){}
	void begin(char* server1,char* server2,char* server3);
	void begin();

	time_t getTimeSeconds(); // get Epoch time
	time_t getLocalTimeSeconds();
	
	const char *getDateTimeStr();

	void setInternetAccessibility(bool connected){ _online=connected; }
	void setCurrentTime(time_t current);
	void setTimezoneOffset(int32_t offset);
	int32_t getTimezoneOffset();
private:
	time_t _referenceSeconds;
	time_t _referenceSystemTime;
	bool _online;

	time_t _lastSaved;
	void saveTime(time_t t);
	time_t loadTime();
};

extern TimeKeeperClass TimeKeeper;

#endif
