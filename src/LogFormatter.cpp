#include <ArduinoJson.h>
#include <cstdio>

#include "LogFormatter.h"
#include "DataLogger.h"
#include "Config.h"
#include "TemperatureFormats.h"
#include "BrewPiProxy.h"
#include "ExternalData.h"
#if SupportPressureTransducer
#include "PressureMonitor.h"
#endif


[[nodiscard]] static constexpr char modeInInteger(const char mode){
	switch (mode) {
		case 'p':
			return '3';
		case 'b':
			return '2';
		case 'f':
			return '1';
		default:
			return '0';
	}
}

size_t printFloat(char* buffer,float value,int precision,bool valid,const char* invalidstr)
{
	if(valid){
		return sprintf(buffer, "%.*f", precision, value);
	}
	strcpy(buffer,invalidstr);
	return strlen(invalidstr);
}

size_t dataSprintf(char *buffer,const char *format,const char* invalid)
{
	uint8_t state, mode;
	float beerSet,fridgeSet;
	float beerTemp,fridgeTemp,roomTemp;

	brewPi.getAllStatus(&state,&mode,& beerTemp,& beerSet,& fridgeTemp,& fridgeSet,& roomTemp);

	size_t d=0;
	for(size_t i= 0; i < strlen(format); i++) {
		if (char ch = format[i]; ch == '%') {
			i++;
			ch=format[i];
			if(ch == '%'){
				buffer[d++]=ch;
			}else if(ch == 'b'){
				d += printFloat(buffer+d,beerTemp,1,IS_FLOAT_TEMP_VALID(beerTemp),invalid);
			}else if(ch == 'B'){
				d += printFloat(buffer+d,beerSet,1,IS_FLOAT_TEMP_VALID(beerSet),invalid);
			}else if(ch == 'f'){
				d += printFloat(buffer+d,fridgeTemp,1,IS_FLOAT_TEMP_VALID(fridgeTemp),invalid);
			}else if(ch == 'F'){
				d += printFloat(buffer+d,fridgeSet,1,IS_FLOAT_TEMP_VALID(fridgeSet),invalid);
			}else if(ch == 'r'){
				d += printFloat(buffer+d,roomTemp,1,IS_FLOAT_TEMP_VALID(roomTemp),invalid);
			}else if(ch == 'g'){
				float sg=externalData.gravity();
				d += printFloat(buffer+d,sg,4,IsGravityValid(sg),invalid);
			}else if(ch == 'p'){
				float sg=externalData.plato();
				d += printFloat(buffer+d,sg,2,IsGravityValid(sg),invalid);
			}
			#if SupportPressureTransducer
			else if(ch == 'P'){
				d += printFloat(buffer+d,PressureMonitor.currentPsi(),1,PressureMonitor.isCurrentPsiValid(),invalid);
			}
			#endif
			else if(ch == 'v'){
				float vol=externalData.deviceVoltage();
				d += printFloat(buffer+d,vol,1,IsVoltageValid(vol),invalid);
			}else if(ch == 'a'){
				float at=externalData.auxTemp();
				d += printFloat(buffer+d,at,1,IS_FLOAT_TEMP_VALID(at),invalid);
			}else if(ch == 't'){
				float tilt=externalData.tiltValue();
				d += printFloat(buffer+d,tilt,2,isTiltAngleValid(tilt),invalid);
			}else if(ch == 'u'){
				d += sprintf(buffer+d, "%lld",  externalData.lastUpdate());
			}else if(ch == 'U'){
				char unit;
				uint8_t unused1,unused2;
				brewPi.getLogInfo(&unit,&unused1,&unused2);
				*(buffer+d)= unit;
				d++;
			}else if(ch == 'm'){
				*(buffer+d)= modeInInteger(mode);
				d++;
			}else if(ch == 'M'){
				*(buffer+d)= mode;
				d++;
			}else{
				// wrong format
				return 0;
			}
		}else{
			buffer[d++]=ch;
		}
	}// for each char

	buffer[d]='\0';
	return d;
}

/*
int _copyName(char *buf,char *name,bool concate)
{
	char *ptr=buf;
	if(name ==nullptr) return 0;
	if(concate){
		*ptr='&';
		ptr++;
	}
	int len=strlen(name);
	strcpy(ptr,name);
	ptr+=len;
	*ptr = '=';
	ptr++;
	return (ptr - buf);
}

int copyTemp(char* buf,char* name,float value, bool concate)
{
	int n;
	if((n = _copyName(buf,name,concate))!=0){
		if(IS_FLOAT_TEMP_VALID(value)){
			n += sprintf(buf + n, "%.*f", 2, value);
		}else{
			strcpy(buf + n,"null");
			n += 4;
		}

	}
	return n;
}
*/

size_t nonNullJson(char* buffer,size_t size)
{
	uint8_t state, mode;
	float beerSet,fridgeSet;
	float beerTemp,fridgeTemp,roomTemp;

	brewPi.getAllStatus(&state,&mode,& beerTemp,& beerSet,& fridgeTemp,& fridgeSet,& roomTemp);

	JsonDocument doc;
	if(IS_FLOAT_TEMP_VALID(beerTemp)) doc[KeyBeerTemp] = beerTemp;
	if(IS_FLOAT_TEMP_VALID(beerSet)) doc[KeyBeerSet] = beerSet;
	if(IS_FLOAT_TEMP_VALID(fridgeTemp)) doc[KeyFridgeTemp] = fridgeTemp;
	if(IS_FLOAT_TEMP_VALID(fridgeSet)) doc[KeyFridgeSet] = fridgeSet;
	if(IS_FLOAT_TEMP_VALID(roomTemp)) doc[KeyRoomTemp] = roomTemp;

	doc[KeyMode] =(int)( modeInInteger(mode) - '0');
	#if SupportPressureTransducer
	if(PressureMonitor.isCurrentPsiValid()) doc[KeyPressure]= PressureMonitor.currentPsi();
	#endif
	if (const float sg = externalData.gravity(); IsGravityValid(sg)) {
		doc[KeyGravity] = sg;
		doc[KeyPlato] = externalData.plato();
	}

	// iSpindel data
	if (const float vol=externalData.deviceVoltage(); IsVoltageValid(vol)) {
		doc[KeyVoltage] = vol;
		if (const float at = externalData.auxTemp(); IS_FLOAT_TEMP_VALID(at)) {
			doc[KeyAuxTemp] = at;
		}
		doc[KeyTilt]=externalData.tiltValue();
	}
	return	serializeJson(doc,buffer,size);
}
