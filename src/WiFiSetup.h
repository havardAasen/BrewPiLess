#ifndef WiFiSetup_H
#define WiFiSetup_H

#include <DNSServer.h>


#define TIME_WAIT_TO_CONNECT 20000
#define TIME_RECONNECT_TIMEOUT 20000
#define    DNS_PORT  53

class WiFiSetupClass
{
private:
	enum class WiFiState {
		connected,
		mode_change_pending,
		connecting,
		disconnected,
		disconnect_pending,
		change_connect_pending,
		connection_recovering,
		connection_recovering_pending,
		unknown,
	};

	enum class WiFiScan {
		none,
		pending,
		scanning,
	};

public:
	WiFiSetupClass():_wifiState(WiFiState::connected),_wifiScan(WiFiScan::none),_apMode(false),_switchToAp(true),_autoReconnect(true),
		 _maxReconnect(5),_eventHandler(NULL),_targetSSID(NULL),_targetPass(NULL),_ip(INADDR_NONE),_gw(INADDR_NONE),_nm(INADDR_NONE){}

	void begin(WiFiMode mode, char const *ssid,const char *passwd=NULL);
	void setMode(WiFiMode mode);
	void staConfig(IPAddress ip=0,IPAddress gw=0, IPAddress nm=0, IPAddress dns=0);

	void onEvent(std::function<void(const char*)> handler){ _eventHandler = handler;}

	bool stayConnected(void);
	bool isApMode(void) {return _apMode;}

	void setMaxReconnect(unsigned int reconnect){_maxReconnect=reconnect;}
	void setSwitchToApWhenDisconnected(bool toAp){  _switchToAp= toAp; }
	void setAutoReconnect(bool reconnect){ _autoReconnect=reconnect; }

	String scanWifi(void);
	bool requestScanWifi(void);
	bool connect(char const *ssid,const char *passwd=NULL,IPAddress ip=0,IPAddress gw=0, IPAddress nm=0,IPAddress dns=0);
	bool disconnect(void);

	bool isConnected(void);
	String status(void);
private:
	WiFiMode _mode;
	WiFiState _wifiState;
	WiFiScan _wifiScan;
	bool _apMode;
	bool _switchToAp;
	bool _autoReconnect;

	unsigned int _maxReconnect;
	unsigned int _reconnect;

	unsigned long _time;
	std::function<void(const char*)> _eventHandler;
	
	std::unique_ptr<DNSServer>        dnsServer;

	const char *_apName;
	const char *_apPassword;

	const char *_targetSSID;
	const char *_targetPass;
	IPAddress _ip;
	IPAddress _gw;
	IPAddress _nm;
	IPAddress _dns;

	void setupApService(void);
	void enterBackupApMode();
	void onConnected();
	void createNetwork();
};

extern WiFiSetupClass WiFiSetup;
#endif
