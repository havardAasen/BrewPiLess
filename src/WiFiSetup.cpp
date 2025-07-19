#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <ESP8266mDNS.h>
//needed for library
#include <DNSServer.h>
#include "Config.h"
#include "WiFiSetup.h"

#include <algorithm>
#include <memory>
#include <numeric>
#include <vector>

WiFiSetupClass WiFiSetup;


#if SerialDebug
#define DebugOut(a) DebugPort.print(a)
#define DBG_PRINTF(...) DebugPort.printf(__VA_ARGS__)
#define wifi_info(a)	DBG_PRINTF("%s,SSID:%s IP:%s, gw:%s\n",(a),WiFi.SSID().c_str(),WiFi.localIP().toString().c_str(),WiFi.gatewayIP().toString().c_str())
#else
#define DebugOut(a)
#define DBG_PRINTF(...)
#define wifi_info(a)
#endif

void WiFiSetupClass::staConfig(const IPAddress& ip, const IPAddress& gw, const IPAddress& nm, const IPAddress& dns){
	_ip=ip;
	_gw=gw;
	_nm=nm;
	_dns=dns;
}

void WiFiSetupClass::setMode(WiFiMode mode)
{
	if(mode == _mode)
		return;

	DBG_PRINTF("WiFi: Change mode from: %d to %d\n", _mode, mode);
	_mode = mode;
	_wifiState = WiFiState::mode_change_pending;
}

void WiFiSetupClass::enterBackupApMode()
{
	WiFi.mode(WIFI_AP_STA);
	createNetwork();
}

void WiFiSetupClass::createNetwork(){
	if(strlen(_apPassword)>=8)
		WiFi.softAP(_apName, _apPassword);
	else
		WiFi.softAP(_apName);

	DBG_PRINTF("WiFi: Create network [%s]\n", _apName);
}

void WiFiSetupClass::setupApService()
{
	dnsServer = std::make_unique<DNSServer>();
	dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
	dnsServer->start(dns_port, "*", WiFi.softAPIP());
	delay(500);
}


bool WiFiSetupClass::isApMode(){
	return WiFi.getMode() == WIFI_AP;
}


void WiFiSetupClass::begin(const WiFiMode mode, const char *ssid, const char *passwd,
                           const char *targetSSID, const char *targetPass)
{
	wifi_info("begin:");

	if (targetSSID && targetSSID[0]) {
		if (_targetSSID) free((void *) _targetSSID);
		_targetSSID = strdup(targetSSID);
	}
	if (targetPass && targetPass[0]) {
		if (_targetPass) free((void *) _targetPass);
		_targetPass = strdup(targetPass);
	}

	_mode = mode;
	WiFiMode mode2use = (_mode == WIFI_OFF) ? WIFI_AP_STA : _mode;

	DBG_PRINTF("Saved SSID:\"%s\" targetSSID:%s\n", WiFi.SSID().c_str(),
	           targetSSID? targetSSID:"NULL");
	DBG_PRINTF("AP mode:%d, used;%d autoReconect:%d\n", mode, mode2use,
	           WiFi.getAutoReconnect());
	if ((mode2use == WIFI_STA || mode2use == WIFI_AP_STA)
	    && _targetSSID == nullptr
	    && (WiFi.SSID() == "[Your SSID]" || WiFi.SSID() == "" || WiFi.SSID() == nullptr)) {
		DBG_PRINTF("Invalid SSID!");
		mode2use = WIFI_AP;
	}
	_apName = (ssid == nullptr || *ssid == '\0') ? DEFAULT_HOSTNAME : ssid;
	_apPassword = (passwd != nullptr && *passwd == '\0') ? nullptr : passwd;

	WiFi.setAutoConnect(true);
	WiFi.mode(mode2use);

	if (mode2use == WIFI_AP || mode2use == WIFI_AP_STA) {
		createNetwork();
		setupApService();
	}

	if (mode2use == WIFI_STA || mode2use == WIFI_AP_STA) {
		if (_ip != INADDR_NONE && _ip != IPAddress(0, 0, 0, 0)) {
			DBG_PRINTF("Config IP:%s, _gw:%s, _nm:%s\n", _ip.toString().c_str(),
			           _gw.toString().c_str(), _nm.toString().c_str());
			WiFi.config(_ip, _gw, _nm, _dns);
		} else {
			// the weird printout of "[NO IP]" implies that explicitly specification of DHCP
			// might be necessary.
			DBG_PRINTF("Dynamic IP\n");
			WiFi.config(IPAddress(0, 0, 0, 0), IPAddress(0, 0, 0, 0),
			            IPAddress(0, 0, 0, 0));
		}
		WiFi.setAutoReconnect(true);

		const auto status = targetSSID
				? WiFi.begin(targetSSID, targetPass)
				: WiFi.begin();
		DBG_PRINTF("WiFi.begin() return: %d\n", status);

		_time = millis();
	}
	_wifiState = (mode2use == WIFI_AP)
		             ? WiFiState::disconnected
		             : WiFiState::connection_recovering;
}

bool WiFiSetupClass::connect(char const *ssid, const char *passwd, const IPAddress& ip,
                             const IPAddress& gw, const IPAddress& nm, const IPAddress& dns){
	DBG_PRINTF("Connect to %s, ip=%s\n",ssid, ip.toString().c_str());

	if(_targetSSID) free((void*)_targetSSID);
	_targetSSID=strdup(ssid);
	if(_targetPass) free((void*)_targetPass);
	_targetPass=(passwd)? strdup(passwd):nullptr;

	_ip=ip;
	_gw=gw;
	_nm=nm;
	_dns=dns;

	_wifiState = WiFiState::change_connect_pending;
	return true;
}

bool WiFiSetupClass::disconnect(){
	DBG_PRINTF("Disconnect Request\n");
	_wifiState = WiFiState::disconnect_pending;
	return true;
}

bool WiFiSetupClass::isConnected(){
	return WiFi.status() == WL_CONNECTED;
}

void WiFiSetupClass::onConnected(){
	if(_eventHandler){
		_eventHandler(status().c_str());
	}
}

String WiFiSetupClass::status(){
	String ret;
	ret  = String("{\"md\":") + String(_mode) + String(",\"con\":") + String((WiFi.status() == WL_CONNECTED)? 1:0);

	if(_mode != WIFI_AP){
		ret += String(",\"ssid\":\"") + WiFi.SSID() 
			 + String("\",\"ip\":\"") + WiFi.localIP().toString()
			 + String("\",\"gw\":\"") + WiFi.gatewayIP().toString()
			 + String("\",\"nm\":\"") + WiFi.subnetMask().toString() + String("\"");
	}

	ret += String("}");
	return ret;
}

bool WiFiSetupClass::stayConnected()
{
	if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA)
		dnsServer->processNextRequest();

	if (_wifiState == WiFiState::change_connect_pending) {
		DBG_PRINTF("Change Connect\n");
		WiFi.disconnect();
		WiFiMode mode = WiFi.getMode();
		if (mode == WIFI_AP) {
			WiFi.mode(WIFI_AP_STA);
			MDNS.notifyAPChange();
		}

		if (_ip != INADDR_NONE) {
			WiFi.config(_ip, _gw, _nm, _dns);
		}
		if (_targetSSID)
			WiFi.begin(_targetSSID, _targetPass);
		else
			WiFi.begin();
		_reconnect = 0;
		_wifiState = WiFiState::connection_recovering;

		wifi_info("**try:");
		_time = millis();
	} else if (_wifiState == WiFiState::disconnect_pending) {
		WiFi.disconnect();
		DBG_PRINTF("Enter AP Mode\n");
		_wifiState = WiFiState::disconnected;
		return true;
	} else if (_wifiState == WiFiState::mode_change_pending) {
		WiFiMode mode = WiFi.getMode();

		if (mode == WIFI_AP_STA) {
			if (_mode == WIFI_AP) {
				DBG_PRINTF("Change from AP_STA to AP\n");
				WiFi.disconnect();
				_wifiState = WiFiState::disconnected;
			} else if (_mode == WIFI_STA) {
			}
			WiFi.mode(_mode);
			MDNS.notifyAPChange();
		} else if (mode == WIFI_STA) {
			if (_mode == WIFI_AP_STA) {
				WiFi.mode(_mode);
				MDNS.notifyAPChange();

				createNetwork();
				setupApService();
			} else if (_mode == WIFI_AP) {
				//WiFi.disconnect();
				_wifiState = WiFiState::disconnected;
				WiFi.mode(_mode);
				MDNS.notifyAPChange();
			}
		} else if (mode == WIFI_AP) {
			if (_mode == WIFI_AP_STA) {
				WiFi.mode(_mode);
				MDNS.notifyAPChange();
			}
			WiFi.begin();
			_wifiState = WiFiState::connection_recovering;
			_time = millis();

			if (_mode == WIFI_STA) {
				if (WiFi.SSID() == nullptr || WiFi.SSID() == "") {
					WiFi.mode(WIFI_AP_STA);
					MDNS.notifyAPChange();
				}
				// just keep WIFI_AP_Mode in case Network isn't specified
			}
		}
	} else if (WiFi.status() != WL_CONNECTED) {
		if (_wifiState == WiFiState::connected) {
			wifi_info("**disc:");
			if (_mode != WIFI_AP) {
				onConnected();
				DBG_PRINTF("Lost Network.WiFi.status()= %d\n", WiFi.status());
				_wifiState = WiFiState::connection_recovering;
				if (_targetSSID) WiFi.begin(_targetSSID, _targetPass);
				else WiFi.begin();
				WiFi.setAutoReconnect(true);
				_time = millis();
			}
		} else if (_wifiState == WiFiState::connection_recovering) {
			// if sta mode, turn on AP mode
			if (millis() - _time > time_for_recovering_network) {
				DBG_PRINTF("Stop recovering\n");
				// enter AP mode, or the underlying WiFi stack would keep searching and block
				//  connections to AP mode.

				WiFi.setAutoReconnect(false);

				if (_mode == WIFI_STA && WiFi.getMode() == WIFI_STA) {
					enterBackupApMode();
				}

				_time = millis();
				_wifiState = WiFiState::disconnected;
			} // millis() - _time > TimeForRecoveringNetwork
		} else if (_wifiState == WiFiState::disconnected) {
			if (_mode == WIFI_AP) {
				// Can't "return" here, if it returns here, the "canning networking will never run."
				//  // don't try to restore network, since there is none to be rediscover
			} else {
				// in AP_STA or STA mode
				if (millis() - _time > wait_time_to_recover_network) {
					DBG_PRINTF("Start recovering, wifi.status=%d\n",
					           WiFi.status());
					DBG_PRINTF("retry SSID:%s\n",
					           _targetSSID? _targetSSID:"NULL");
					if (_targetSSID) WiFi.begin(_targetSSID, _targetPass);
					else WiFi.begin();

					WiFi.setAutoReconnect(true);

					_wifiState = WiFiState::connection_recovering;
					_time = millis();
				}
			}
		}
	} else { // connected
		if (_mode == WIFI_AP) {
			DBG_PRINTF("Connected in AP_mode\n");
		} else {
			const WiFiState oldState = _wifiState;
			_wifiState = WiFiState::connected;
			_reconnect = 0;
			if (oldState != _wifiState) {
				if (WiFi.getMode() != _mode) {
					DBG_PRINTF("Change to mode: %d from %d\n", _mode,
					           WiFi.getMode());
					WiFi.mode(_mode);
					MDNS.notifyAPChange();
				}

				wifi_info("WiFi Connected:");
				onConnected();
			}
		}
	} // end of connected

	if(_wifiScan == WiFiScan::pending){
		String nets=scanWifi();
		_wifiScan = WiFiScan::none;
		if(_eventHandler) _eventHandler(nets.c_str());
	}

	return false;
}

bool WiFiSetupClass::requestScanWifi() {
	if(_wifiScan == WiFiScan::none){
		_wifiScan = WiFiScan::pending;
		return true;
	}
	return false;
}

String WiFiSetupClass::scanWifi() const {
	
	String rst="{\"list\":[";
	
	DBG_PRINTF("Scan Networks...\n");
	const std::uint8_t available_networks = WiFi.scanNetworks();
    DBG_PRINTF("Scan done\n");
    if (available_networks == 0) {
    	DBG_PRINTF("No networks found\n");
		rst += "]}";
		return rst;
    }

	std::vector<std::uint8_t> networks(available_networks);
	std::iota(networks.begin(), networks.end(), 0);
	std::sort(networks.begin(), networks.end(), [](const std::uint8_t a, const std::uint8_t b)
			  { return WiFi.SSID(a) > WiFi.SSID(b); });
	const auto end_unique = std::unique(networks.begin(), networks.end(),
	                                    [](const std::uint8_t a, const std::uint8_t b) {
		                                    return WiFi.SSID(a) == WiFi.SSID(b);
	                                    });
	networks.erase(end_unique, networks.end());
	std::sort(networks.begin(), networks.end(), [](const std::uint8_t a, const std::uint8_t b)
			  { return WiFi.RSSI(a) > WiFi.RSSI(b); });

	bool comma = false;
	for (const auto network : networks) {
		DBG_PRINTF("SSID: %s, RSSI: %d\n", WiFi.SSID(network).c_str(), WiFi.RSSI(network));
		String item = String("{\"ssid\":\"") + WiFi.SSID(network) +
					  String("\",\"rssi\":") + WiFi.RSSI(network) +
					  String(",\"enc\":") + String((WiFi.encryptionType(network) != ENC_TYPE_NONE) ? "1" : "0") + String("}");
		if (comma)
			rst += ",";
		else
			comma = true;
		rst += item;
	}

	rst += "]}";
	DBG_PRINTF("scan result:%s\n",rst.c_str());
	return rst;
}
