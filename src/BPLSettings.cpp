#include <ArduinoJson.h>
#include <time.h>
#include <string.h>
#include <IPAddress.h>
#include <FS.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>

#include "Config.h"
#include "BPLSettings.h"
#include "BrewLogger.h"

BPLSettings theSettings;

#define BPLSettingFileName "/bpl.cfg"


void BPLSettings::preFormat(){
	brewLogger.onFormatFS();
}

void BPLSettings::postFormat(){
	save();
}


void BPLSettings::setWiFiConfiguration(const char* ssid, const char* password)
{
    if(ssid)
		strcpy(_data.wifiConfiguration.ssid, ssid);
    else
		_data.wifiConfiguration.ssid[0]='\0';

	if(password)
		strcpy(_data.wifiConfiguration.pass, password);
	else
		_data.wifiConfiguration.pass[0]='\0';
}


void BPLSettings::load()
{
	DBG_PRINTF("syscfg:%d, timeinfo:%d, gdc:%d, \
		tempSchedule:%d, brewStatus:%d, logFileIndexes:%d, \
		remoteLogginInfo:%d, autoCapSettings:%d, parasiteTempControlSettings:%d\n",\
		 offsetof(Settings,syscfg),offsetof(Settings,timeinfo),offsetof(Settings,gdc),
		 offsetof(Settings,tempSchedule),offsetof(Settings,brewStatus),offsetof(Settings,logFileIndexes),
		 offsetof(Settings,remoteLogginInfo),offsetof(Settings,autoCapSettings),
		 offsetof(Settings,parasiteTempControlSettings));

	fs::File f = LittleFS.open(BPLSettingFileName, "r");
	if(!f){
		setDefault();
		return;
	}
	f.read((uint8_t*)&_data,sizeof(_data));
	f.close();
	// check invalid value, and correct
	SystemConfiguration *cfg=  systemConfiguration();
	if( *( cfg->hostnetworkname) == '\0'
		|| cfg->wifiMode ==0){
			setDefault();
			DBG_PRINTF("invalid system configuration!\n");
	}
		
}

void BPLSettings::save()
{
	fs::File f = LittleFS.open(BPLSettingFileName, "w");
    if(!f){
		DBG_PRINTF("error open configuratoin file\n");
		return;
	}
    f.write((uint8_t*)&_data,sizeof(_data));
    f.close();
}


void BPLSettings::setDefault()
{
	// clear. to be safe
	memset((char*)&_data,'\0',sizeof(_data));
	// 
	defaultSystemConfiguration();
    defaultBeerProfile();
    defaultRemoteLogging();
}


//***************************************************************
// system configuration
#define  KeyLcdBackLight "aoff"
#define  KeyPageTitle "title"
#define  KeyHostName "name"
#define  KeyPort     "port"
#define  KeyUsername  "user"
#define  KeyPassword  "pass"
#define  KeyProtect   "protect"
#define  KeyWifi      "wifi"
#define  KeyIpAddress "ip"
#define  KeyGateway   "gw"
#define  KeyNetmask    "mask"
#define  KeyDNS "dns"

extern IPAddress scanIP(const char *str);

SystemConfiguration* BPLSettings::systemConfiguration(){
    return &_data.syscfg;
}
    // decode json
static void stringNcopy(char *dst,const char *src,size_t n){
	if(strlen(src) < n){
		strcpy(dst,src);
	}else{
		strncpy(dst,src,n-1);
		dst[n-1]='\0';
	}
}


void BPLSettings::defaultSystemConfiguration(){
    SystemConfiguration *syscfg=&_data.syscfg;

    stringNcopy(syscfg->titlelabel,DEFAULT_PAGE_TITLE,32);
    stringNcopy(syscfg->hostnetworkname,DEFAULT_HOSTNAME,32);
    stringNcopy(syscfg->username,DEFAULT_USERNAME,32);
    stringNcopy(syscfg->password,DEFAULT_PASSWORD,32);

    syscfg->port = 80;
    syscfg->passwordLcd = false;
    syscfg->wifiMode = WIFI_AP_STA;
    syscfg->backlite = 0;
    syscfg->ip = (uint32_t) IPAddress(0,0,0,0);
    syscfg->gw = (uint32_t) IPAddress(0,0,0,0);
    syscfg->netmask = (uint32_t) IPAddress(0,0,0,0);
    syscfg->dns = (uint32_t) IPAddress(0,0,0,0);
}

bool BPLSettings::dejsonSystemConfiguration(String json){

	SystemConfiguration *syscfg=&_data.syscfg;

	JsonDocument doc;
	auto error = deserializeJson(doc,json);

	if(!error)
	{
        stringNcopy(syscfg->titlelabel,doc[KeyPageTitle],32);
        stringNcopy(syscfg->hostnetworkname,doc[KeyHostName],32);
        stringNcopy(syscfg->username,doc[KeyUsername],32);
        stringNcopy(syscfg->password,doc[KeyPassword],32);

        syscfg->port = doc[KeyPort];
        syscfg->passwordLcd = doc[KeyProtect];
        syscfg->wifiMode = doc[KeyWifi];
        syscfg->backlite = doc[KeyLcdBackLight];
		return true;
    }
	return false;
}
    // encod json
String BPLSettings::jsonSystemConfiguration(){
    SystemConfiguration *syscfg=&_data.syscfg;

	JsonDocument doc;
    doc[KeyPageTitle]=syscfg->titlelabel;
    doc[KeyHostName]= syscfg->hostnetworkname;
    doc[KeyUsername]= syscfg->username;
    doc[KeyPassword]= syscfg->password;

    doc[KeyPort]=   syscfg->port;
    doc[KeyProtect]=    syscfg->passwordLcd;
    doc[KeyWifi] =   syscfg->wifiMode;
    doc[KeyLcdBackLight] =   syscfg->backlite;

    doc[KeyIpAddress]= IPAddress(syscfg->ip).toString();
    doc[KeyGateway]= IPAddress(syscfg->gw).toString();
    doc[KeyNetmask]= IPAddress(syscfg->netmask).toString();
	doc[KeyDNS] = IPAddress(syscfg->dns).toString();

    String ret;
	serializeJson(doc,ret);

    return ret;
}
   
//***************************************************************
// gravity device configuration

#define KeyEnableiSpindel "ispindel"
#define KeyTempCorrection "tc"
#define KeyCorrectionTemp "ctemp"
#define KeyCalculateGravity "cbpl"
#define KeyCoefficientA0 "a0"
#define KeyCoefficientA1 "a1"
#define KeyCoefficientA2 "a2"
#define KeyCoefficientA3 "a3"
#define KeyLPFBeta "lpc"
#define KeyStableGravityThreshold "stpt"
#define KeyNumberCalPoints "npt"
#define KeyUsePlato "plato"

 bool BPLSettings::dejsonGravityConfig(char* json)
{
	JsonDocument doc;
	if (const auto error = deserializeJson(doc, json)) {
	    DBG_PRINTF("ERROR: %s: deserializeJson() failed: %s\n", __func__, error.c_str());
	    return false;
	}

	GravityDeviceConfiguration *gdc = &_data.gdc;

	gdc->ispindelEnable=doc[KeyEnableiSpindel];
	gdc->ispindelTempCal = doc[KeyTempCorrection];

	gdc->ispindelCalibrationBaseTemp = doc[KeyCorrectionTemp].is<int>() ? doc[KeyCorrectionTemp] : 20;
	gdc->calculateGravity=doc[KeyCalculateGravity];
	gdc->ispindelCoefficients[0]=doc[KeyCoefficientA0];
	gdc->ispindelCoefficients[1]=doc[KeyCoefficientA1];
	gdc->ispindelCoefficients[2]=doc[KeyCoefficientA2];
	gdc->ispindelCoefficients[3]=doc[KeyCoefficientA3];
	gdc->lpfBeta =doc[KeyLPFBeta];
	gdc->stableThreshold=doc[KeyStableGravityThreshold];
	gdc->numberCalPoints=doc[KeyNumberCalPoints];
	gdc->usePlato = doc[KeyUsePlato].is<int>() ? doc[KeyUsePlato] : 0;
	// debug
	#if SerialDebug
	Serial.print("\nCoefficient:");
	for(const float coefficient : gdc->ispindelCoefficients){
	    Serial.print(coefficient, 10);
	    Serial.print(", ");
	}
	Serial.println("");
	#endif
	return true;
}

String BPLSettings::jsonGravityConfig(){
		// save to file
        GravityDeviceConfiguration *gdc = &_data.gdc;

		JsonDocument doc;
		doc[KeyEnableiSpindel] = gdc->ispindelEnable;
		doc[KeyTempCorrection] = gdc->ispindelTempCal;

		doc[KeyCorrectionTemp] = gdc->ispindelCalibrationBaseTemp;
		doc[KeyCalculateGravity] = gdc->calculateGravity;
		doc[KeyLPFBeta] =gdc->lpfBeta;
		doc[KeyStableGravityThreshold] = gdc->stableThreshold;

		doc[KeyCoefficientA0]=gdc->ispindelCoefficients[0];
		doc[KeyCoefficientA1]=gdc->ispindelCoefficients[1];
		doc[KeyCoefficientA2]=gdc->ispindelCoefficients[2];
		doc[KeyCoefficientA3]=gdc->ispindelCoefficients[3];
		doc[KeyNumberCalPoints] = gdc->numberCalPoints;
		doc[KeyUsePlato] = gdc->usePlato;
	 
	String ret;
	serializeJson(doc,ret);
    return ret;
}	

//***************************************************************
// Beer profile

/*
 * Reconstitute "struct tm" elements into a time_t count value.
 * Note that the year argument is offset from 1970.
 */
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24UL)
#define DAYS_PER_WEEK (7UL)
#define SECS_PER_WEEK (SECS_PER_DAY * DAYS_PER_WEEK)
#define SECS_PER_YEAR (SECS_PER_WEEK * 52UL)
#define SECS_YR_2000  (946684800UL)	// The time_t value at the very start of Y2K.
static	const uint8_t monthDays[]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
#define LEAP_YEAR(Y)		 ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )
time_t tm_to_timet(struct tm *tm_time){

	int i;
	time_t seconds;

	seconds= tm_time->tm_year*(SECS_PER_DAY * 365);
	for (i = 0; i < tm_time->tm_year; i++) {
		if (LEAP_YEAR(i)) {
			seconds += SECS_PER_DAY;	// Add extra days for leap years.
		}
	}
	// Add the number of elapsed days for the given year. Months start from 1.
	for (i = 1; i < tm_time->tm_mon; i++) {
		if ( (i == 2) && LEAP_YEAR(tm_time->tm_year)) {
			seconds += SECS_PER_DAY * 29;
		} else {
			seconds += SECS_PER_DAY * monthDays[i-1];	// "monthDay" array starts from 0.
		}
	}
	seconds+= (tm_time->tm_mday-1) * SECS_PER_DAY;		// Days...
	seconds+= tm_time->tm_hour * SECS_PER_HOUR;		// Hours...
	seconds+= tm_time->tm_min * SECS_PER_MIN;		// Minutes...
	seconds+= tm_time->tm_sec;				// ...and finally, Seconds.
	return (time_t)seconds;
}
// got from https://github.com/PaulStoffregen/Time
void makeTime(time_t timeInput, struct tm &tm){
// break the given time_t into time components
// this is a more compact version of the C library localtime function
// note that year is offset from 1970 !!!

  uint8_t year;
  uint8_t month, monthLength;
  uint32_t time;
  unsigned long days;

  time = (uint32_t)timeInput;
  tm.tm_sec = time % 60;
  time /= 60; // now it is minutes
  tm.tm_min = time % 60;
  time /= 60; // now it is hours
  tm.tm_hour = time % 24;
  time /= 24; // now it is days
  tm.tm_wday = ((time + 4) % 7) + 1;  // Sunday is day 1

  year = 0;
  days = 0;
  while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
    year++;
  }
  tm.tm_year = year +1970; // year is offset from 1970

  days -= LEAP_YEAR(year) ? 366 : 365;
  time  -= days; // now it is days in this year, starting at 0

  days=0;
  month=0;
  monthLength=0;
  for (month=0; month<12; month++) {
    if (month==1) { // february
      if (LEAP_YEAR(year)) {
        monthLength=29;
      } else {
        monthLength=28;
      }
    } else {
      monthLength = monthDays[month];
    }

    if (time >= monthLength) {
      time -= monthLength;
    } else {
        break;
    }
  }
  tm.tm_mon = month + 1;  // jan is month 1
  tm.tm_mday = time + 1;     // day of month
}

 void BPLSettings::defaultBeerProfile()
 {
	BeerTempSchedule *tempSchedule = & _data.tempSchedule;
	tempSchedule->unit = 'C';
	tempSchedule->numberOfSteps =1;
	ScheduleStep *step = &tempSchedule->steps[0];
	step->condition = 't';
	step->days = ScheduleDayFromJson(7);
	step->temp = ScheduleTempFromJson(20);
 }

bool BPLSettings::dejsonBeerProfile(String json)
{
	JsonDocument doc;
	if (const auto error = deserializeJson(doc, json)) {
		DBG_PRINTF("ERROR: %s: deserializeJson() failed: %s\n", __func__, error.c_str());
		return false;
	}
	if(!doc["s"].is<const char*>() || !doc["u"].is<const char*>() || !doc["t"].is<JsonArray>()){
		DBG_PRINTF("JSON file not include necessary fields\n");
		return false;
	}
	BeerTempSchedule *tempSchedule = & _data.tempSchedule;
	// get starting time
	//ISO time:
	//2016-07-01T05:22:33.351Z
	//01234567890123456789
	tm tmStart;
	char buf[8];
	const char* sdutc=doc["s"];

	#define GetValue(d,s,l) strncpy(buf,sdutc+s,l);buf[l]='\0';d=atoi(buf)
	GetValue(tmStart.tm_year,0,4);
	tmStart.tm_year -= 1970; //1900;
	GetValue(tmStart.tm_mon,5,2);
//	tmStart.tm_mon -= 1;
	GetValue(tmStart.tm_mday,8,2);
	GetValue(tmStart.tm_hour,11,2);
	GetValue(tmStart.tm_min,14,2);
	GetValue(tmStart.tm_sec,17,2);

	DBG_PRINTF("%d/%d/%d %d:%d:%d\n",tmStart.tm_year,tmStart.tm_mon,tmStart.tm_mday,
		tmStart.tm_hour,tmStart.tm_min,tmStart.tm_sec);

	//_startDay = mktime(&tmStart);

	tempSchedule->startDay= tm_to_timet(&tmStart);
	JsonArray schedule = doc["t"];
	tempSchedule->numberOfSteps=schedule.size();
	if(tempSchedule->numberOfSteps > MaximumSteps) tempSchedule->numberOfSteps=MaximumSteps;

	for(int i=0;i< tempSchedule->numberOfSteps ;i++){
		ScheduleStep *step = &tempSchedule->steps[i];
		JsonObject	 entry= schedule[i];
		//{"c":"g","d":6,"t":12,"g":1.026},{"c":"r","d":1}
		const char* constr= entry["c"];
		step->condition = *constr;
		step->days = ScheduleDayFromJson(entry["d"].as<float>());

		DBG_PRINTF("%d ,type:%c time:",i,step->condition );
		DBG_PRINT(step->days);

		if(step->condition != 'r'){ // all but not ramping
			float temp=entry["t"];
			step->temp= ScheduleTempFromJson(temp);

			if (entry["g"].is<const char*>()) {
			    step->attSpecified=false;
    			    const char* attStr=entry["g"];
    			    float att=atof(attStr);
    			    if( strchr ( attStr, '%' ) > 0){
	    			    DBG_PRINTF(" att:%s sg:%d ",attStr,step->gravity.sg);
						step->attSpecified=true;
						step->gravity.attenuation =(uint8_t) att;
			    }
			} else if (entry["g"].is<float>()) {
				if(! step->attSpecified){
				    float fsg= entry["g"];
				    if(_data.gdc.usePlato){
					step->gravity.sg = PlatoToGravity(fsg);
				    }else{
					step->gravity.sg = SGToGravity(fsg);
				    }
				DBG_PRINTF(" sg:%d",step->gravity.sg);
				}
			}

			if(entry["s"].is<int>()){
	    		    step->stable.stableTime = entry["s"];
	    		    step->stable.stablePoint = entry["x"].is<int>() ? entry["x"] : _data.gdc.stableThreshold;

    			    DBG_PRINTF("Stable :%d@%d",step->stable.stablePoint,step->stable.stableTime);
			}

			DBG_PRINT(" temp:");
			DBG_PRINT(step->temp);
		}
		DBG_PRINTF("\n");
	}

	// unit
	const char *uintStr=doc["u"];
	tempSchedule->unit=  *uintStr;

	DBG_PRINTF("Load finished, st:%lld, unit:%c, _numberOfSteps:%d\n",tempSchedule->startDay,
	tempSchedule->unit,tempSchedule->numberOfSteps);

	return true;
}

String BPLSettings::jsonBeerProfile()
{
	BeerTempSchedule *tempSchedule = & _data.tempSchedule;

	//start date
	//ISO time:
	//2016-07-01T05:22:33.351Z
	struct tm * ptm;
	ptm = localtime(& tempSchedule->startDay);
	char timeBuf[128];
	sprintf(timeBuf,"%d-%02d-%02dT%02d:%02d:%02d.0Z",ptm->tm_year+1900,ptm->tm_mon+1,ptm->tm_mday,
		ptm->tm_hour,ptm->tm_min,ptm->tm_sec);
	JsonDocument doc;
	doc["s"]=timeBuf;
	// unit, unfortunatly, no "char" type in JSON. "char" will be integer.
	//doc["u"]=(char)tempSchedule->unit;
	char unitBuffer[4];
	sprintf(unitBuffer,"%c",tempSchedule->unit);
	doc["u"]=unitBuffer;
	JsonArray steps=doc["t"].to<JsonArray>();

	char conditionBuf[MaximumSteps][4];
	char pertages[MaximumSteps][16];
	int  pertageIndex=0;

	for(int i=0;i< tempSchedule->numberOfSteps;i++){
		ScheduleStep *s_step= & tempSchedule->steps[i];
		JsonObject jstep = steps.add<JsonObject>();
		// condition
		//jstep["c"] =(char) s_step->condition;
		sprintf(conditionBuf[i],"%c", s_step->condition);
		jstep["c"] = conditionBuf[i];
		// days
		jstep["d"] =ScheduleDayToJson(s_step->days);
		// temp.
		if( s_step->condition != 'r' ){ // not rampping
			jstep["t"]= ScheduleTempToJson(s_step->temp);
		}
		/*
                   <option value="t">Time</option>
                   <option value="g">SG</option>
                   <option value="s">Stable</option>
                   <option value="a">Time & SG</option>
                   <option value="o">Time OR SG</option>
                   <option value="u">Time OR Stable</option>
                   <option value="v">Time & Stable</option>
                    <option value="b">SG OR Stable</option>
                    <option value="x">SG & Stable</option>
                    <option value="w">ALL</option>
                    <option value="e">Either</option>
		*/
		if(strchr("gaobxwe",s_step->condition)){ 
			// gravity
			if(s_step->attSpecified){
				
				sprintf(pertages[pertageIndex],"%d%%",s_step->gravity.attenuation);
				jstep["g"]= pertages[pertageIndex];
				pertageIndex++;
			}else{
				DBG_PRINTF("  sg:%d \n",s_step->gravity.sg);
				if(_data.gdc.usePlato){
					jstep["g"]= GravityToPlato(s_step->gravity.sg);
					Serial.print("SG:");
					Serial.println(GravityToPlato(s_step->gravity.sg));
				}else{
					jstep["g"]= GravityToSG(s_step->gravity.sg);
				}
			}
		}
		if(strchr("suvbxwe",s_step->condition)){ 
			// stable.
			jstep["s"]= s_step->stable.stableTime;
			jstep["x"]= s_step->stable.stablePoint;
		}
	}// end of for
	
	String ret;
	serializeJson(doc,ret);

    return ret;
}

//***************************************************************
// Remote data logging
void BPLSettings::defaultRemoteLogging()
{
	// OK for all zero
}

bool BPLSettings::dejsonRemoteLogging(String json)
{

	RemoteLoggingInformation *logInfo = remoteLogInfo();
	logInfo->enabled=false;

	JsonDocument doc;
	if (const auto error = deserializeJson(doc, json)) {
		DBG_PRINTF("ERROR: %s: deserializeJson() failed: %s\n", __func__, error.c_str());
		return false;
	}
	if(!doc["enabled"].is<bool>()
		|| !doc["format"].is<const char*>()
		|| !doc["url"].is<const char*>()
		|| !doc["type"].is<const char*>()
		|| !doc["method"].is<const char*>()
		|| !doc["period"].is<const char*>()){
		DBG_PRINTF("ERROR: %s: Missing required field(s)\n", __func__);
		return false;
	}

	const char *url=doc["url"];
	const char *method=doc["method"];
	const char *format=doc["format"];
	const char *contentType=doc["type"];

	bool enabled= doc["enabled"];
	uint32_t period = doc["period"];
		
	if(url == nullptr || method==nullptr || format==nullptr 
		|| strcmp(url,"") ==0 || strcmp(method,"") ==0 || strcmp(format,"") ==0){	
		return false;
	}

	if(strlen(url) >=MaximumUrlLength) return false;
	if(strlen(format) >= MaximumFormatLength) return false;
	if(strlen(contentType)>=MaximumContentTypeLength) return false;

	if(strcmp(method,"GET") ==0) logInfo->method = mHTTP_GET;
	else if(strcmp(method,"POST") ==0) logInfo->method = mHTTP_POST;
	else if(strcmp(method,"PUT") ==0)  logInfo->method = mHTTP_PUT;
	else return false;
	strcpy(logInfo->url,url);
	strcpy(logInfo->format,format);
	strcpy(logInfo->contentType,contentType);
	logInfo->period = period;
	logInfo->enabled = enabled;
	logInfo->service = doc["service"].is<int>() ? doc["service"] : 0;

  	return true;
}

String BPLSettings::jsonRemoteLogging()
{
	RemoteLoggingInformation *logInfo = remoteLogInfo();

	JsonDocument doc;
	doc["enabled"] = logInfo->enabled;
	doc["period"] = logInfo->period;
	doc["service"] = logInfo->service;

	if(logInfo->method ==mHTTP_GET) doc["method"]="GET";
	else if(logInfo->method ==mHTTP_PUT) doc["method"]="PUT";
	else if(logInfo->method ==mHTTP_POST) doc["method"]="POST";

	doc["url"]=(logInfo->url)? logInfo->url:"";
	doc["format"]=(logInfo->format)? logInfo->format:"";
	doc["type"]=(logInfo->contentType)? logInfo->contentType:"";

	String ret;
	serializeJson(doc,ret);
    return ret;
}

//***************************************************************
// parasite control
#if EanbleParasiteTempControl
#define EnableKey "enabled"
#define SetTempKey "temp"
#define TrigerTempKey "stemp"
#define MinCoolKey "mincool"
#define MinIdleKey "minidle"

bool BPLSettings::dejsonParasiteTempControlSettings(String json){
	JsonDocument doc;
	auto error = deserializeJson(doc,json);
	if(error
		|| !doc[SetTempKey].is<float>()
		|| !doc[TrigerTempKey].is<float>()
		|| !doc[MinCoolKey].is<std::uint32_t>()
		|| !doc[MinIdleKey].is<std::uint32_t>()){
            return false;
        }
	ParasiteTempControlSettings *ps=parasiteTempControlSettings();

    float n_setTemp = doc[SetTempKey];
    float n_maxIdleTemp = doc[TrigerTempKey];
    uint32_t n_mincool=doc[MinCoolKey] ;
    uint32_t n_minidle= doc[MinIdleKey];

    ps->minIdleTime = n_minidle * 1000;
    ps->minCoolingTime = n_mincool * 1000;
    ps->setTemp = n_setTemp;
    ps->maxIdleTemp = n_maxIdleTemp;

    return true;
}

String BPLSettings::jsonParasiteTempControlSettings(bool enabled){
	JsonDocument doc;
    doc[EnableKey]= enabled;
    
	ParasiteTempControlSettings *ps=parasiteTempControlSettings();

    doc[SetTempKey]=ps->setTemp ;
    doc[TrigerTempKey] =ps->maxIdleTemp;
    doc[MinCoolKey] = ps->minCoolingTime /  1000;
    doc[MinIdleKey]=  ps->minIdleTime / 1000;
    String ret;
	serializeJson(doc,ret);
    return ret;
}


#endif
//***************************************************************
// pressure control
#if SupportPressureTransducer
#define PressureMonitorModeKey "mode"
#define ConversionAKey "a"
#define ConversionBKey "b"

bool BPLSettings::dejsonPressureMonitorSettings(String json){

	JsonDocument doc;
	auto error = deserializeJson(doc,json);
	if(error
		|| !doc[PressureMonitorModeKey].is<std::uint16_t>()
		|| !doc[ConversionAKey].is<float>()
		|| !doc[ConversionBKey].is<std::uint16_t>()){
            return false;
        }
	PressureMonitorSettings *settings=pressureMonitorSettings();
	settings->mode = doc[PressureMonitorModeKey];
	settings->fa = doc[ConversionAKey];
	settings->fb = doc[ConversionBKey];
	return true;
}

String BPLSettings::jsonPressureMonitorSettings(){
	PressureMonitorSettings *settings=pressureMonitorSettings();

	JsonDocument doc;
	doc[PressureMonitorModeKey]=settings->mode;
	doc[ConversionAKey]=settings->fa;
	doc[ConversionBKey]=settings->fb;
    String ret;
	serializeJson(doc,ret);
    return ret;

}
#endif
//***************************************************************
// MQTT control
#if SupportMqttRemoteControl
#define EnableRemoteControlKey "enabled"
#define ServerAddressKey "server"
#define ServerPort "port"
#define MqttUsernameKey "user"
#define MqttPasswordKey "pass"

#define ModePathKey "mode"
#define BeerSetPathKey "beerset"
#define FridgeSetPathKey "fridgeset"
#define PtcPathKey "ptc"
#define CapPathKey "cap"

#define MqttLoggingKey "log"
#define MqttLogPeriodKey "period"
#define ReportBasePathKey "base"
#define MqttReportFormatKey "format"

String BPLSettings::jsonMqttRemoteControlSettings(){
	MqttRemoteControlSettings *settings=mqttRemoteControlSettings();
	
	JsonDocument doc;
	doc[EnableRemoteControlKey] = (settings->mode == MqttModeControl || settings->mode == MqttModeBothControlLoggging)? 1:0;
	doc[ServerPort] =  settings->port;

	doc[MqttLoggingKey] =  (settings->mode == MqttModeLogging || settings->mode == MqttModeBothControlLoggging)? 1:0;
	doc[MqttLogPeriodKey] = settings->reportPeriod;
	doc[MqttReportFormatKey] = settings->reportFormat;

	if(settings->reportBasePathOffset){
		char* base=(char*) (settings->_strings + settings->reportBasePathOffset);
		DBG_PRINTF("base path:%s offset:%d\n",base, settings->reportBasePathOffset);
		doc[ReportBasePathKey] =base;
	}

	if(settings->modePathOffset){
		char* modepath=(char*) (settings->_strings + settings->modePathOffset);
		DBG_PRINTF("mode path:%s offset:%d\n",modepath, settings->modePathOffset);
		doc[ModePathKey] =modepath;
	}

	if(settings->beerSetPathOffset){
		char* setpath=(char*) (settings->_strings + settings->beerSetPathOffset);
		DBG_PRINTF("beerSet path:%s offset:%d\n",setpath, settings->beerSetPathOffset);
		doc[BeerSetPathKey] = setpath;
	}

	if(settings->fridgeSetPathOffset){
		char* setpath=(char*) (settings->_strings + settings->fridgeSetPathOffset);
		DBG_PRINTF("fridgeSet path:%s offset:%d\n",setpath, settings->fridgeSetPathOffset);
		doc[FridgeSetPathKey] = setpath;
	}


#if	EanbleParasiteTempControl
	if(settings->ptcPathOffset){
		doc[PtcPathKey] = settings->_strings + settings->ptcPathOffset;
	}
#endif

#if Auto_CAP
	if(settings->capControlPathOffset){
		doc[CapPathKey] = settings->_strings + settings->capControlPathOffset;
	}
#endif

	if(settings->serverOffset){
		doc[ServerAddressKey] =(char*) (settings->_strings + settings->serverOffset);
	}

	if(settings->usernameOffset){
		doc[MqttUsernameKey] =(char*)(settings->_strings + settings->usernameOffset);
	}

	if(settings->passwordOffset){
		doc[MqttPasswordKey] =(char*) (settings->_strings + settings->passwordOffset);
	}

    String ret;
	serializeJson(doc,ret);

	DBG_PRINTF("json:--\n%s\n--\n",ret.c_str());
    return ret;

}
static char *copyIfExist(JsonDocument &doc,const char* key,uint16_t &offset,char* ptr,char* base){
	if(doc[key].is<const char*>()){
		const char* str=doc[key];
		size_t length = strlen(str) +1;
		if(length==1){
			offset =0;
			return ptr;
		} 


		if(ptr - base  +length > MqttSettingStringSpace ) return nullptr;
		strcpy(ptr,str);
		offset = (uint16_t)(ptr - base);

		size_t rto4= (length & 0x3)? ((length & ~0x3) + 4):length;
		ptr += rto4;

		DBG_PRINTF("mqtt set:%s offset:%d, length:%d, ptr inc:%d\n",key,offset,length,rto4);
	}

	return ptr;
}

bool BPLSettings::dejsonMqttRemoteControlSettings(String json){

	JsonDocument doc;
	if (const auto error = deserializeJson(doc,json)) {
	    DBG_PRINTF("ERROR: %s: deserializeJson() failed: %s\n", __func__, error.c_str());
	    return false;
	}
	if(!doc[EnableRemoteControlKey].is<bool>() || !doc[ServerPort].is<std::uint16_t>()){
	    DBG_PRINTF("ERROR: %s: Required field(s) is missing\n", __func__);
	    return false;
	}
	MqttRemoteControlSettings *settings=mqttRemoteControlSettings();

	memset((char*)settings,'\0',sizeof(MqttRemoteControlSettings));

	bool rc=doc[EnableRemoteControlKey];
	if(!doc[MqttLoggingKey].is<bool>()){
		settings->mode= rc? MqttModeControl:MqttModeOff;
		// everything else is "cleared" by memset to zero
	}else{
		bool log=doc[MqttLoggingKey];
		settings->mode= (rc && log)? MqttModeBothControlLoggging:
						( rc? MqttModeControl:
							( log? MqttModeLogging:MqttModeOff));		
		settings->reportPeriod=doc[MqttLogPeriodKey];
		settings->reportFormat=doc[MqttReportFormatKey];
	}



	settings->port=doc[ServerPort];

	char *base=(char*) settings->_strings;
	char *ptr=base +4;

	if(!(ptr=copyIfExist(doc,ServerAddressKey,settings->serverOffset,ptr,base))) return false;
	if(!(ptr=copyIfExist(doc,MqttUsernameKey,settings->usernameOffset,ptr,base))) return false;
	if(!(ptr=copyIfExist(doc,MqttPasswordKey,settings->passwordOffset,ptr,base))) return false;
	if(!(ptr=copyIfExist(doc,ModePathKey,settings->modePathOffset,ptr,base))) return false;
	if(!(ptr=copyIfExist(doc,BeerSetPathKey,settings->beerSetPathOffset,ptr,base))) return false;
	if(!(ptr=copyIfExist(doc,FridgeSetPathKey,settings->fridgeSetPathOffset,ptr,base))) return false;

	if(!(ptr=copyIfExist(doc,ReportBasePathKey,settings->reportBasePathOffset,ptr,base))) return false;

	#if	EanbleParasiteTempControl
	if(!(ptr=copyIfExist(doc,PtcPathKey,settings->ptcPathOffset,ptr,base))) return false;
	#endif

	#if Auto_CAP
	if(!(ptr=copyIfExist(doc,CapPathKey,settings->capControlPathOffset,ptr,base))) return false;
	#endif

	return true;
}

#endif
