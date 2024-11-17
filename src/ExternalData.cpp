#include <ArduinoJson.h>
#include <cstdio>
#include "ExternalData.h"
#include "common/conversion.h"

ExternalData externalData;


float ExternalData::plato(bool filtered){ 
	float sg= filtered? _filteredGravity:_gravity;
	return _cfg->usePlato ? sg : bpl::specific_gravity_to_brix(sg);
}
float ExternalData::gravity(bool filtered){ 
	float sg= filtered? _filteredGravity:_gravity;
	return _cfg->usePlato ? bpl::brix_to_specific_gravity(sg) : sg;
}

void ExternalData::waitFormula(){
    _cfg->numberCalPoints =0;
}

bool ExternalData::iSpindelEnabled(){
    return _cfg->ispindelEnable;
}

float ExternalData::hydrometerCalibration(){ 
    return _cfg->ispindelCalibrationBaseTemp;
}

void ExternalData::sseNotify(char *buf){

		char strbattery[8];
		int len=sprintf(strbattery, "%.*f", 2, _deviceVoltage);
		strbattery[len]='\0';

		char strgravity[8];
		len=sprintf(strgravity, "%.*f", 3, _gravity);
		strgravity[len]='\0';

		char slowpassfilter[8];
		len=sprintf(slowpassfilter, "%.*f", 2, filter.beta());
		slowpassfilter[len]='\0';

		char strtilt[8];
		len=sprintf(strtilt, "%.*f", 2, _ispindelTilt);
		strtilt[len]='\0';

		char coeff[4][20];
		for(int i=0;i<4;i++){
			len=sprintf(coeff[i], "%.*f", 9, _cfg->ispindelCoefficients[i]);
			coeff[i][len]='\0';	
		}
		char strRssi[32];
		if(_rssiValid){
			len=sprintf(strRssi,",\"rssi\":%d",_rssi);
			strRssi[len]='\0';
		}else{
			strRssi[1]=' ';
			strRssi[0]='\0';
		} 
		const char *spname=(_ispindelName)? _ispindelName:"Unknown";
		sprintf(buf,R"(G:{"name":"%s","battery":%s,"sg":%s,"angle":%s %s,"lu":%lld,"lpf":%s,"stpt":%d,"fpt":%d,"ctemp":%d,"plato":%d})",
					spname, 
					strbattery,
					strgravity,
					strtilt,
					strRssi,
					_lastUpdate,slowpassfilter,_cfg->stableThreshold,
					_cfg->numberCalPoints,
                    _cfg->ispindelCalibrationBaseTemp,
					_cfg->usePlato);
}


void ExternalData::loadConfig(){
    _cfg = theSettings.GravityConfig();
    filter.setBeta(_cfg->lpfBeta);
}


bool ExternalData::processconfig(char* configdata){
   bool ret= theSettings.dejsonGravityConfig(configdata);
   if(ret){
	   theSettings.save();
   }
   return ret;
}

void ExternalData::formula(float coeff[4],uint32_t npt){
    if(_cfg->numberCalPoints == npt){ 
		DBG_PRINTF("formula nochanged\n");
		return;
	}
	_cfg->numberCalPoints =npt;

	for(int i=0;i<4;i++){
		_cfg->ispindelCoefficients[i] = coeff[i];
	}
	 theSettings.save();
}

void ExternalData::setOriginalGravity(float og){
//		_og = og;
		brewLogger.addGravity(og,true);
#if EnableGravitySchedule
		brewKeeper.updateOriginalGravity(og);
#endif
}

void ExternalData::setTilt(float tilt,float temp,time_t now){
	_lastUpdate=now;
	_ispindelTilt=tilt;

	// add tilt anyway
	brewLogger.addTiltAngle(tilt);

	if(_calibrating && _cfg->numberCalPoints ==0){
		DBG_PRINTF("No valid formula!\n");
		return; // don't calculate if formula is not available.
	}
		// calculate plato
	float sg = _cfg->ispindelCoefficients[0]
            +  _cfg->ispindelCoefficients[1] * tilt
            +  _cfg->ispindelCoefficients[2] * tilt * tilt
            +  _cfg->ispindelCoefficients[3] * tilt * tilt * tilt;

	// temp. correction
	if(_cfg->ispindelTempCal){
		if(_cfg->usePlato){
			sg = bpl::specific_gravity_to_brix(temperatureCorrection(
				bpl::brix_to_specific_gravity(sg), bpl::celsius_to_fahrenheit(temp),
				bpl::celsius_to_fahrenheit(
					static_cast<float>(_cfg->ispindelCalibrationBaseTemp))));
		}else
		sg = temperatureCorrection(sg, bpl::celsius_to_fahrenheit(temp),
	                                   bpl::celsius_to_fahrenheit(
		                                   static_cast<float>(_cfg->
			                                   ispindelCalibrationBaseTemp)));
	}
	// update, gravity data calculated
	setGravity(sg,now,!_calibrating); // save only when not calibrating.
}

void ExternalData::setGravity(float sg, time_t now,bool log){
        // copy these two for reporting to web interface
    float old_sg=_gravity;
	
    DBG_PRINTF("setGravity:%d, saved:%d\n",(int)(sg*10000.0),log);
    // verfiy sg, even invalid value will be reported to web interface
	//if(!IsGravityInValidRange(sg)) return;
	_gravity = sg;
	_lastUpdate=now;

	if(!IsGravityValid(old_sg)) filter.setInitial(sg);
#if EnableGravitySchedule
    float _filteredGravity=filter.addData(sg);
		// use filter data as input to tracker and beer profile.
	brewKeeper.updateGravity(_filteredGravity);
	if(_cfg->usePlato)
		gravityTracker.add(Plato2TrackingGravity(_filteredGravity),now);
	else
		gravityTracker.add(SG2TrackingGravity(_filteredGravity),now);
#endif
	// don't save it if it is cal&brew
	if(log) brewLogger.addGravity(sg,false);
}


void ExternalData::setAuxTemperatureCelsius(float temp){
	char unit;
	float max,min;

    brewPi.getTemperatureSetting(&unit,&min,&max);
	
    if(unit == 'C'){
		_auxTemp= temp;
	}else{
		_auxTemp = bpl::celsius_to_fahrenheit(temp);
	}
	
    brewLogger.addAuxTemp(_auxTemp);

	#if BREWPI_EXTERNAL_SENSOR
	if(WirelessTempSensor::theWirelessTempSensor){
		WirelessTempSensor::theWirelessTempSensor->setTemp(temp);
	}
	#endif
}


float  ExternalData::temperatureCorrection(float sg, float t, float c){

	return sg*((1.00130346-0.000134722124*t+0.00000204052596*t*t -0.00000000232820948*t*t*t)/
	    (1.00130346-0.000134722124*c+0.00000204052596*c*c-0.00000000232820948*c*c*c));
}

bool ExternalData::processGravityReport(char data[], size_t length, bool authenticated,
                                        uint8_t &error)
{
    JsonDocument doc;
    auto jsonerror = deserializeJson(doc, data, length);
    if (jsonerror || !doc.containsKey("name")) {
        DBG_PRINTF("Invalid JSON\n");
        error = ErrorJSONFormat;
        return false;
    }

    String name = doc["name"];
    // web interface
    if (name.equals("webjs")) {
        if (!authenticated) {
            error = ErrorAuthenticateNeeded;
            return false;
        }

        if (!doc.containsKey("gravity")) {
            DBG_PRINTF("No gravity\n");
            error = ErrorMissingField;
            return false;
        }
        float gravity = doc["gravity"];

        // if(!IsGravityInValidRange(gravity)) return true;
        if (doc.containsKey("plato")) {
            if (doc["plato"] && !_cfg->usePlato) {
                gravity = bpl::brix_to_specific_gravity(gravity);
            } else if (!doc["plato"] && _cfg->usePlato) {
                gravity = bpl::specific_gravity_to_brix(gravity);
            }
        }

        if (doc.containsKey("og")) {
            setOriginalGravity(gravity);
        } else {
            // gravity data from user
            setGravity(gravity, TimeKeeper.getTimeSeconds());
        }
    } else if (name.startsWith("iSpindel")) {
        //{"name": "iSpindel01", "id": "XXXXX-XXXXXX", "temperature": 20.5, "angle": 89.5,
        //"gravityP": 13.6, "battery": 3.87}
        DBG_PRINTF("%s\n", name.c_str());

        if (!_ispindelName) {
            _ispindelName = (char *) malloc(name.length() + 1);
            if (_ispindelName)
                strcpy(_ispindelName, name.c_str());
        }

        if (!doc.containsKey("temperature")) {
            DBG_PRINTF("iSpindel report no temperature!\n");
            return false;
        }

        float itemp = doc["temperature"];
        float tempC = itemp;
        if (doc.containsKey("temp_units")) {
            const char *TU = doc["temp_units"];
            if (*TU == 'F')
                tempC = bpl::fahrenheit_to_celsius(itemp);
            else if (*TU == 'K')
                tempC = itemp - 273.15;
        }

        setAuxTemperatureCelsius(tempC);

        // Serial.print("temperature:");
        // Serial.println(itemp);

        if (!doc.containsKey("angle")) {
            DBG_PRINTF("iSpindel report no angle!\n");
            return false;
        }

        setTilt(doc["angle"], itemp, TimeKeeper.getTimeSeconds());

        if (doc.containsKey("battery"))
            setDeviceVoltage(doc["battery"]);

        if (doc.containsKey("RSSI"))
            setDeviceRssi(doc["RSSI"]);

        // setPlato(doc["gravityP"],TimeKeeper.getTimeSeconds());
        if (doc.containsKey("gravity") && !_cfg->calculateGravity && !_calibrating) {
            // gravity information directly from iSpindel
            float sgreading = doc["gravity"];
            setGravity(sgreading, TimeKeeper.getTimeSeconds());
        }
    } else {
        error = ErrorUnknownSource;
        return false;
    }
    return true;
}

