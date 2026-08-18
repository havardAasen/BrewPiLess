#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <ESPmDNS.h>
#endif
#include <ArduinoOTA.h>
#include <FS.h>
#include <LittleFS.h>
#include <literals.h>

//#include <Hash.h>

#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
//{ brewpi
#include <OneWire.h>

#include "Actuator.h"
#include "Ticks.h"
#include "Display.h"
#include "TempControl.h"
#include "PiLink.h"
#include "Menu.h"
#include "Pins.h"
#include "RotaryEncoder.h"
#include "Buzzer.h"
#include "TempSensor.h"
#include "TempSensorMock.h"
#include "TempSensorExternal.h"
#include "Ticks.h"
#include "Sensor.h"
#include "SettingsManager.h"
#include "ESPEepromAccess.h"
#include "EepromFormat.h"
#include "WebHandler/BrewPiDataHandler.h"
#include "WebHandler/ExternalDataHandler.h"
#include "WebHandler/LogHandler.h"
#include "WebHandler/NetworkDataHandler.h"


#if BREWPI_SIMULATE
#include "Simulator.h"
#endif

//}brewpi
#if AUTO_CAP
#include "AutoCapControl.h"
#endif

#include "TimeKeeper.h"

#include "GravityTracker.h"
#include "BrewKeeper.h"
#ifdef ENABLE_LOGGING
#include "DataLogger.h"
#endif

extern "C" {
#include <sntp.h>
}

#include "BPLSettings.h"

#include "ESPUpdateServer.h"
#include "WiFiSetup.h"

#include "BrewPiProxy.h"

#include "BrewLogger.h"

#include "ExternalData.h"

#if SupportMqttRemoteControl
#include "MqttRemoteControl.h"
#endif

#if EanbleParasiteTempControl
#include "ParasiteTempController.h"
#endif

#if SupportPressureTransducer
#include "PressureMonitor.h"
#endif

#define CaptivePortalTimeout 180

/**************************************************************************************/
/* Start of Configuration 															  */
/**************************************************************************************/

#define WS_PATH 		"/ws"

#define POLLING_PATH 	"/getline_p"
#define PUTLINE_PATH	"/putline"


	String getContentType(String filename){
		if(filename.endsWith(asyncsrv::T__htm)) return asyncsrv::T_text_html;
		else if(filename.endsWith(asyncsrv::T__html)) return asyncsrv::T_text_html;
		else if(filename.endsWith(asyncsrv::T__css)) return asyncsrv::T_text_css;
		else if(filename.endsWith(asyncsrv::T__js)) return asyncsrv::T_text_javascript;
		else if(filename.endsWith(asyncsrv::T__png)) return asyncsrv::T_image_png;
		else if(filename.endsWith(asyncsrv::T__gif)) return asyncsrv::T_image_gif;
		else if(filename.endsWith(asyncsrv::T__jpg)) return asyncsrv::T_image_jpeg;
		else if(filename.endsWith(asyncsrv::T__ico)) return asyncsrv::T_image_x_icon;
		else if(filename.endsWith(asyncsrv::T__xml)) return asyncsrv::T_text_xml;
		else if(filename.endsWith(asyncsrv::T__pdf)) return asyncsrv::T_application_pdf;
		else if(filename.endsWith(".zip")) return "application/zip";
		else if(filename.endsWith(asyncsrv::T__gz)) return "application/gzip";
		return asyncsrv::T_text_plain;
	  }

GravityTracker gravityTracker;

AsyncWebServer *webServer;

BrewPiProxy brewPi;
BrewKeeper brewKeeper([](const char* str){ brewPi.putLine(str);});
#ifdef ENABLE_LOGGING
DataLogger dataLogger;
#endif



void initTime(bool apmode)
{
	if(apmode){
		DBG_PRINTF("initTime in ap mode\n");
		TimeKeeper.begin();
	}else{
		DBG_PRINTF("connect to Time server\n");
		TimeKeeper.begin((char*)"time.google.com",(char*)"pool.ntp.org",(char*)"time.windows.com");
	}
}
#if AUTO_CAP
void capStatusReport();
#endif

#if AUTO_CAP
String capControlStatus()
{
	uint8_t mode=autoCapControl.mode();
	bool capped = autoCapControl.isCapOn();
	String 	capstate=String("\"m\":") + String((int)mode) + String(",\"c\":") + String(capped);

	if(mode == AutoCapModeGravity){
		capstate += String(",\"g\":") + String(autoCapControl.targetGravity(),3);
	}else if (mode ==AutoCapModeTime){
		capstate += String(",\"t\":") + String(autoCapControl.targetTime());
	}

#if SupportPressureTransducer
	PressureMonitorSettings* ps=theSettings.pressureMonitorSettings();
	if(ps->mode == PMModeControl){
		capstate += String(",\"pm\":2,\"psi\":") + String(ps->psi);
	}
#endif

	return capstate;
} 
void stringAvailable(const char*);
void capStatusReport()
{
	char buf[128];
	String capstate= capControlStatus();

	sprintf(buf,"A:{\"cap\":{%s}}", capstate.c_str());
	stringAvailable(buf);
}
#endif

void greeting(const std::function<void(const char*)>& sendFunc)
{
	char buf[512];
	// gravity related info., starting from "G"
	//if(externalData.iSpindelEnabled()){
		externalData.sseNotify(buf);
		sendFunc(buf);
	//}

	// misc informatoin, including

	// RSSI && 
	const char *logname= brewLogger.currentLog();
	if(logname == nullptr) logname="";
	SystemConfiguration *syscfg= theSettings.systemConfiguration();
#if AUTO_CAP
	String capstate= capControlStatus();

#if EanbleParasiteTempControl
	
	String ptcstate= parasiteTempController.getSettings();

	sprintf(buf,"A:{\"nn\":\"%s\",\"ver\":\"%s\",\"rssi\":%d,\"tm\":%lld,\"off\":%u,\"log\":\"%s\",\"cap\":{%s},\"ptcs\":%s}"
		,syscfg->titlelabel,BPL_VERSION,WiFi.RSSI(),
		static_cast<std::int64_t>(TimeKeeper.getTimeSeconds()),TimeKeeper.getTimezoneOffset(),
		logname, capstate.c_str(),ptcstate.c_str());


#else
	sprintf(buf,"A:{\"nn\":\"%s\",\"ver\":\"%s\",\"rssi\":%d,\"tm\":%lld,\"off\":%u,\"log\":\"%s\",\"cap\":{%s}}"
		,syscfg->titlelabel,BPL_VERSION,WiFi.RSSI(),
		static_cast<std::int64_t>(TimeKeeper.getTimeSeconds()),TimeKeeper.getTimezoneOffset(),
		logname, capstate.c_str());
#endif
	
#else
	sprintf(buf,"A:{\"nn\":\"%s\",\"ver\":\"%s\",\"rssi\":%d,\"tm\":%lld,\"off\":%u, \"log\":\"%s\"}"
		,syscfg->titlelabel,BPL_VERSION,WiFi.RSSI(),
		static_cast<std::int64_t>(TimeKeeper.getTimeSeconds()),TimeKeeper.getTimezoneOffset(),
		logname);
#endif

	sendFunc(buf);

	// beer profile:
	String profile=String("B:") + theSettings.jsonBeerProfile();
	sendFunc(profile.c_str());
	//network status:

	String nwstatus=String("W:") + WiFiSetup.status();
	sendFunc(nwstatus.c_str());

}

#define GreetingInMainLoop 1

AsyncWebSocket ws(WS_PATH);


#if GreetingInMainLoop
AsyncWebSocketClient * _lastWSclient=nullptr;
void sayHelloWS()
{
	if(! _lastWSclient) return;
	
	greeting([=](const char* msg){
			_lastWSclient->text(msg);
	});
	
	_lastWSclient = nullptr;
}

#endif

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{
	if(type == WS_EVT_CONNECT){
    	DBG_PRINTF("ws[%s][%u] connect\n", server->url(), client->id());
    	//client->printf("Hello Client %u :)", client->id());
		#if GreetingInMainLoop
		_lastWSclient = client;
		#else
		greeting([=](const char* msg){
			client->text(msg);
		});
		#endif
  	} else if(type == WS_EVT_DISCONNECT){
    	DBG_PRINTF("ws[%s] disconnect: %u\n", server->url(), client->id());
  	} else if(type == WS_EVT_ERROR){
    	DBG_PRINTF("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
  	} else if(type == WS_EVT_PONG){
    	DBG_PRINTF("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
  	} else if(type == WS_EVT_DATA){
    	AwsFrameInfo * info = (AwsFrameInfo*)arg;
//    	String msg = "";
    	if(info->final && info->index == 0 && info->len == len){
      		//the whole message is in a single frame and we got all of it's data
//      		DBG_PRINTF("ws[%u] message[%lu]:", client->id(), info->len);

	        for(size_t i=0; i < info->len; i++) {
        	  //msg += (char) data[i];
        	  brewPi.write(data[i]);
        	}
//		    DBG_PRINTF("%s\n",msg.c_str());

		} else {
      		//message is comprised of multiple frames or the frame is split into multiple packets
/*      		if(info->index == 0){
        		if(info->num == 0)
        		DBG_PRINTF("ws[%u] frame[%u] start[%u]\n", client->id(), info->num, info->len);
      		}*/

//      		DBG_PRINTF("ws[%u] frame [%lu - %lu]: ", client->id(), info->num, info->index, info->index + len);

	        for(size_t i=0; i < info->len; i++) {
    	    	//msg += (char) data[i];
    	    	brewPi.write(data[i]);
        	}

      		//DBG_PRINTF("%s\n",msg.c_str());

			if((info->index + len) == info->len){
//				DBG_PRINTF("ws[%u] frame[%u] end[%lu]\n", client->id(), info->num, info->len);
//        		if(info->final){
//        			DBG_PRINTF("ws[%s][%u] %s-message end\n",  client->id());
//        		}
      		}
      	}
    }
}

void stringAvailable(const char *str)
{
	//DBG_PRINTF("BroadCast:%s\n",str);
	ws.textAll(str,strlen(str));
}

void reportRssi()
{
	char buf[256];

	Mode mode;
	State state;
	char unit;
	float beerSet, beerTemp, fridgeTemp, fridgeSet, roomTemp;
	float min,max;
	char statusLine[21];
	brewPi.getTemperatureSetting(&unit,&min,&max);
	brewPi.getAllStatus(state, mode, &beerTemp, &beerSet, &fridgeTemp, &fridgeSet, &roomTemp);
	display.getLine(3,statusLine);

#if EanbleParasiteTempControl
	char ptcmode=parasiteTempController.getMode();
	
	#if SupportPressureTransducer
		int pmmode=PressureMonitor.mode();
		int psi = (int) PressureMonitor.currentPsi();
		
		sprintf(buf,"A:{\"rssi\":%d,\"ptc\":\"%c\",\"pt\":%u,\"ptctp\":%d,\"ptclo\":%d,\"ptcup\":%d,\"st\":%d,\"md\":\"%c\",\"bt\":%d,\"bs\":%d,\"ft\":%d,\"fs\":%d,\"rt\":%d,\"sl\":\"%s\",\"tu\":\"%c\",\"pm\":%d,\"psi\":%d}",
				WiFi.RSSI(),ptcmode,parasiteTempController.getTimeElapsed(),
				parasiteTempController.getTemp(),parasiteTempController.getLowerBound(),parasiteTempController.getUpperBound(),
			state,
			mode,
			(int)(beerTemp*100),
			(int)(beerSet*100),
			(int)(fridgeTemp*100),
			(int)(fridgeSet*100),
			(int)(roomTemp*100),
			statusLine,
			unit,
			pmmode,
			psi
			);

	#else
	sprintf(buf,"A:{\"rssi\":%d,\"ptc\":\"%c\",\"pt\":%u,\"ptctp\":%d,\"ptclo\":%d,\"ptcup\":%d,\"st\":%d,\"md\":\"%c\",\"bt\":%d,\"bs\":%d,\"ft\":%d,\"fs\":%d,\"rt\":%d,\"sl\":\"%s\",\"tu\":\"%c\"}",
			WiFi.RSSI(),ptcmode,parasiteTempController.getTimeElapsed(),
			parasiteTempController.getTemp(),parasiteTempController.getLowerBound(),parasiteTempController.getUpperBound(),
		state,
		mode,
		(int)(beerTemp*100),
		(int)(beerSet*100),
		(int)(fridgeTemp*100),
		(int)(fridgeSet*100),
		(int)(roomTemp*100),
		statusLine,
		unit

			);
	#endif
	stringAvailable(buf);
#else
	sprintf(buf,"A:{\"rssi\":%d,\"st\":%d,\"md\":\"%c\",\"bt\":%d,\"bs\":%d,\"ft\":%d,\"fs\":%d,\"rt\":%d,\"sl\":\"%s\",\"tu\":\"%c\"}",
		WiFi.RSSI(),
		state,
		mode,
		(int)(beerTemp*100),
		(int)(beerSet*100),
		(int)(fridgeTemp*100),
		(int)(fridgeSet*100),
		(int)(roomTemp*100),
		statusLine,
		unit
		);
	stringAvailable(buf);
#endif
}

#if GreetingInMainLoop
void sayHello()
{
	sayHelloWS();
}
#endif 


bpl::webHandler::BrewPiDataHandler brewPiWebHandler;
bpl::webHandler::LogHandler logHandler;
bpl::webHandler::ExternalDataHandler externalDataHandler;
bpl::webHandler::NetworkDataHandler networkConfig;

void wiFiEvent(const char* msg){
	char *buff=(char*)malloc(strlen(msg) +3);
	sprintf(buff,"W:%s",msg);
	stringAvailable(buff);
	free(buff);
}
//{brewpi


// global class objects static and defined in class cpp and h files

// instantiate and configure the sensors, actuators and controllers we want to use


/* Configure the counter and delay timer. The actual type of these will vary depending upon the environment.
* They are non-virtual to keep code size minimal, so typedefs and preprocessing are used to select the actual compile-time type used. */
TicksImpl ticks = TicksImpl(TICKS_IMPL_CONFIG);
DelayImpl wait = DelayImpl(DELAY_IMPL_CONFIG);

DisplayType realDisplay;
DisplayType DISPLAY_REF display = realDisplay;

ValueActuator alarm;

void handleReset()
{
#if defined(ESP8266)
	// The asm volatile method doesn't work on ESP8266. Instead, use ESP.restart
	EspClass::restart();
#else
	// resetting using the watchdog timer (which is a full reset of all registers)
	// might not be compatible with old Arduino bootloaders. jumping to 0 is safer.
	asm volatile ("  jmp 0");
#endif
}


void brewpi_setup()
{
	bpl::EspEepromAccess::begin(MAX_EEPROM_SIZE_LIMIT);

#if BREWPI_BUZZER
	buzzer.init();
	buzzer.beep(2, 500);
#endif

	logDebug("started");
	tempControl.init();
	SettingsManager::loadSettings();

#if BREWPI_SIMULATE
	simulator.step();
	// initialize the filters with the assigned initial temp value
	tempControl.beerSensor->init();
	tempControl.fridgeSensor->init();
#endif
#ifdef EARLY_DISPLAY
	display.clear();
#else
	display.init();
#endif
	display.printStationaryText();
	display.printState();

	RotaryEncoder::init();

	logDebug("init complete");
}

void brewpiLoop()
{
	static unsigned long lastUpdate = 0;

	if (ticks.millis() - lastUpdate >= (1000)) { //update settings every second
		lastUpdate = ticks.millis();

#if BREWPI_BUZZER
		buzzer.setActive(alarm.isActive() && !buzzer.isActive());
#endif

		tempControl.updateTemperatures();
		tempControl.detectPeaks();
		tempControl.updatePID();
		const State oldState = tempControl.getState();
		tempControl.updateState();
		if (oldState != tempControl.getState()) {
			PiLink::printTemperatures(); // add a data point at every state transition
		}
		tempControl.updateOutputs();

#if BREWPI_MENU
		if (RotaryEncoder::pushed()) {
			RotaryEncoder::resetPushed();
			display.updateBacklight();
			Menu::pickSettingToChange();
		}
#endif

		// update the lcd for the chamber being displayed
		display.printState();
		display.printAllTemperatures();
		display.printMode();
		display.updateBacklight();
	}

	PiLink::receive();

}

enum class SystemState {
    operating,
    restartPending,
    waitRestart
};

#define TIME_RESTART_TIMEOUT 3000

bool _disconnectBeforeRestart;
static unsigned long _time;
auto _systemState{SystemState::operating};
void requestRestart(bool disc)
{
	_disconnectBeforeRestart=disc;
	_systemState =SystemState::restartPending;
}


#ifdef EMIWorkaround
uint32_t _lcdReinitTime;
#define LCDReInitPeriod (10*60*1000)
#endif


void setup(void){

	#if SerialDebug == true
  	DebugPort.begin(115200);
  	DBG_PRINTF("\nSetup()\n");
  	DebugPort.setDebugOutput(true);
  	#endif

	//0.Initialize file system
	//start SPI Filesystem
  	if(!LittleFS.begin()){
  		// TO DO: what to do?
  		DBG_PRINTF("LittleFS.being() failed!\n");
  	}else{
  		DBG_PRINTF("LittleFS.being() Success.\n");
  	}


#ifdef EARLY_DISPLAY
	DBG_PRINTF("Init LCD...\n");
	display.init();
	display.printAt_P(1,0,PSTR("Initialize WiFi"));
	display.updateBacklight();
	DBG_PRINTF("LCD Initialized..\n");
#endif


	// try open configuration
	theSettings.load();

	SystemConfiguration *syscfg=theSettings.systemConfiguration();
	
	display.setAutoOffPeriod(syscfg->backlite);
	
	#ifdef ENABLE_LOGGING
//  	dataLogger.loadConfig();
  	#endif


	//1. Start WiFi
	DBG_PRINTF("Starting WiFi...\n");
	WiFiSetup.staConfig(IPAddress(syscfg->ip),IPAddress(syscfg->gw),IPAddress(syscfg->netmask),IPAddress(syscfg->dns));
	WiFiSetup.onEvent(wiFiEvent);

        const auto wifiMode = static_cast<WiFiMode>(syscfg->wifiMode);
        if (strlen(syscfg->hostnetworkname) > 0) {
	        const auto *wifiCon = theSettings.getWifiConfiguration();
        	WiFiSetup.begin(wifiMode, syscfg->hostnetworkname, syscfg->password,
				wifiCon->ssid[0] ? wifiCon->ssid : nullptr,
				wifiCon->pass[0] ? wifiCon->pass : nullptr);
        } else {
        	WiFiSetup.begin(wifiMode,DEFAULT_HOSTNAME,DEFAULT_PASSWORD);
        }

	DBG_PRINTF("WiFi Done!\n");

	initTime(WiFiSetup.isApMode());

	if (!MDNS.begin(syscfg->hostnetworkname)) {
			DBG_PRINTF("Error setting mDNS responder\n");
	}else{
		MDNS.addService("http", "tcp", 80);
	}

	// TODO: SSDP responder


	//3. setup Web Server
	webServer=new AsyncWebServer(syscfg->port);
	// start WEB update pages.
#if (DEVELOPMENT_OTA == true) || (DEVELOPMENT_FILEMANAGER == true)
	ESPUpdateServer_setup(syscfg->username,syscfg->password);
#endif

	ws.onEvent(onWsEvent);
	webServer->addHandler(&ws);


	webServer->addHandler(&brewPiWebHandler);

	webServer->addHandler(&logHandler);

	webServer->addHandler(&externalDataHandler);

	webServer->addHandler(&networkConfig);
	//3.1.2 LittleFS is part of the serving pages
	//server.serveStatic("/", LittleFS, "/","public, max-age=259200"); // 3 days


	webServer->on("/fs",[](AsyncWebServerRequest *request){
		FSInfo fs_info;
		LittleFS.info(fs_info);
		request->send(200,"","totalBytes:" +String(fs_info.totalBytes) +
		" usedBytes:" + String(fs_info.usedBytes)+" blockSize:" + String(fs_info.blockSize)
		+" pageSize:" + String(fs_info.pageSize)
		+" freesketch:" + String(EspClass::getFreeSketchSpace())
		+" heap:"+String(EspClass::getFreeHeap()));
		//testSPIFFS();
	});

	// 404 NOT found.
  	//called when the url is not defined here
	webServer->onNotFound([](AsyncWebServerRequest *request){
		request->send(404);
	});

	//4. start Web server
	webServer->begin();
	DBG_PRINTF("HTTP server started\n");


	// 5. try to connnect Arduino
	brewpi_setup();
  	brewPi.begin(stringAvailable);
	//make sure externalData  is initialized.
	if(brewLogger.begin()){
		// resume, update calibrating information to external data
		externalData.setCalibrating(brewLogger.isCalibrating());
		DBG_PRINTF("Start BrweNCal log:%d\n",brewLogger.isCalibrating());
	}
	
	brewKeeper.begin();

	#if AUTO_CAP
	//Note: necessary to call after brewpi_setup() so that device has been installed.
	autoCapControl.begin();
	#endif

#if EanbleParasiteTempControl
	parasiteTempController.init();
#endif


#ifdef STATUS_LINE
	// brewpi_setup will "clear" the screen.
	IPAddress ip =(WiFiSetup.isApMode())? WiFi.softAPIP():WiFi.localIP();
	char buf[21];
	sprintf(buf,"IP:%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
	display.printStatus(buf);
	_displayTime = TimeKeeper.getTimeSeconds() + 20;
#endif
#ifdef EMIWorkaround
	_lcdReinitTime = millis();
#endif

#if SupportMqttRemoteControl
	//mqtt
	mqttRemoteControl.begin();
#endif
}

uint32_t _rssiReportTime;
#define RssiReportPeriod 5

void loop(void){
//{brewpi
#if BREWPI_SIMULATE
	simulateLoop();
#else
	brewpiLoop();
#endif
//}brewpi
	MDNS.update();
#if EanbleParasiteTempControl
	parasiteTempController.run();
#endif

#if (DEVELOPMENT_OTA == true) || (DEVELOPMENT_FILEMANAGER == true)
	ESPUpdateServer_loop();
#endif
	time_t now=TimeKeeper.getTimeSeconds();

#ifdef EMIWorkAround
	if( (millis() - _lcdReinitTime) > LCDReInitPeriod){
		_lcdReinitTime=millis();
		display.fresh();
	}
#endif

	if( (now - _rssiReportTime) > RssiReportPeriod){
		_rssiReportTime =now;
		reportRssi();
	}

  	brewKeeper.keep(now);

  	brewPi.loop();

 	brewLogger.loop();

#if SupportMqttRemoteControl
	mqttRemoteControl.loop();
#endif

 	#ifdef ENABLE_LOGGING

 	dataLogger.loop(now);
 	#endif
	
	#if AUTO_CAP
	if(autoCapControl.autoCapOn(now,externalData.gravity(true))){
		capStatusReport();
	}
	#endif
	
	#if SupportPressureTransducer
	PressureMonitor.loop();
	#endif

	#if GreetingInMainLoop
	sayHello();
	#endif

    switch (_systemState) {
        case SystemState::operating:
            WiFiSetup.stayConnected();
            break;
        case SystemState::restartPending:
            _time = millis();
            _systemState = SystemState::waitRestart;
            break;
        case SystemState::waitRestart:
            if ((millis() - _time) > TIME_RESTART_TIMEOUT) {
                if (_disconnectBeforeRestart) {
                    WiFi.disconnect();
                    WiFiSetup.setAutoReconnect(false);
                    delay(1000);
                }
                EspClass::restart();
            }
    }
}
