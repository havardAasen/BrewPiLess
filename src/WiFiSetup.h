#ifndef WiFiSetup_H
#define WiFiSetup_H

#include <DNSServer.h>


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
	WiFiSetupClass():_wifiState(WiFiState::connected),_wifiScan(WiFiScan::none),_switchToAp(true),_autoReconnect(true),
		 _maxReconnect(5),_eventHandler(nullptr),_targetSSID(nullptr),_targetPass(nullptr),_ip(INADDR_NONE),_gw(INADDR_NONE),_nm(INADDR_NONE){}

	void begin(WiFiMode mode, const char *ssid, const char *passwd = nullptr, const char *targetSSID = nullptr,const char *targetPass = nullptr);
	void setMode(WiFiMode mode);
	void staConfig(const IPAddress& ip=0, const IPAddress& gw=0, const IPAddress& nm=0, const IPAddress& dns=0);

	void onEvent(std::function<void(const char*)> handler){ _eventHandler = handler;}

	bool stayConnected();
	bool isApMode();

	void setMaxReconnect(unsigned int reconnect){_maxReconnect=reconnect;}
	void setSwitchToApWhenDisconnected(bool toAp){  _switchToAp= toAp; }
	void setAutoReconnect(bool reconnect){ _autoReconnect=reconnect; }

	[[nodiscard]] String scanWifi() const;
	bool requestScanWifi();
	bool connect(char const *ssid, const char *passwd=nullptr, const IPAddress& ip=0,
                     const IPAddress& gw=0, const IPAddress& nm=0, const IPAddress& dns=0);
	bool disconnect();

	bool isConnected();
	String status();
private:
	WiFiMode _mode;
	WiFiState _wifiState;
	WiFiScan _wifiScan;
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

	static constexpr int dns_port = 53;
	static constexpr int time_for_recovering_network = 8000;
	static constexpr int wait_time_to_recover_network = 60000;

	void setupApService();
	void enterBackupApMode();
	void onConnected();
	void createNetwork();
};

extern WiFiSetupClass WiFiSetup;
#endif
