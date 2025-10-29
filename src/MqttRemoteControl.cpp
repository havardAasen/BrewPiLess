#include "MqttRemoteControl.h"
#include "BrewKeeper.h"
#include "BPLSettings.h"
#include "DataLogger.h"
#include "LogFormatter.h"
#include "TemperatureFormats.h"
#include "BrewPiProxy.h"
#include "ExternalData.h"
#if SupportPressureTransducer
#include "PressureMonitor.h"
#endif


extern BrewPiProxy brewpi;

#if SupportMqttRemoteControl

#if EanbleParasiteTempControl
#include "ParasiteTempController.h"
#endif

#if Auto_CAP
#include "AutoCapContro.h"
#endif


MqttRemoteControl mqttRemoteControl;

MqttRemoteControl::MqttRemoteControl(){
    _client.onConnect([this](bool){
        this->_onConnect();
    });
    _client.onDisconnect([this](AsyncMqttClientDisconnectReason reason){
        DBG_PRINTF("\n***MQTT:disc:%d\n", static_cast<int>(reason));
        this->_onDisconnect();
    });
    _client.onMessage([this](char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total){
        this->_onMessage(topic,(uint8_t*)payload,len);
    });

    _client.onPublish([this](uint16_t pid){
        this->_onPublish(pid);
    });
}
#define BUFFERSIZE 512

void MqttRemoteControl::_onPublish(uint16_t pid){
    if(_publishing){
        if(pid == _lastPacketId){ // finish last packet
            if(_mode == MqttModeLogging) // logging only
                // problematic _client.disconnect();
                _reconnecting = true;
                //DBG_PRINTF("disconnect on last packet id:%d\n",pid);
        }
    }
}

uint16_t MqttRemoteControl::_publish(const char* key,float value,int precision){
    DBG_PRINTF("Publish %s\n",key);

    // somwhow need to be optimized
    char topic[256];
    int baselength=strlen(_reportBasePath);
    strncpy(topic,_reportBasePath,baselength);
    topic[baselength]='/';
    strcpy(topic + baselength +1, key);

    char data[64];
    int len= sprintf(data, "%.*f", precision, value);
    return _client.publish(topic,DefaultLogginQoS,true,data,len);
}

uint16_t MqttRemoteControl::_publish(const char* key,char value){
    DBG_PRINTF("Publish %s\n",key);

    char topic[256];
    int len=strlen(_reportBasePath);
    strncpy(topic,_reportBasePath,len);
    topic[len]='/';
    strcpy(topic + len +1, key);

    char data[4];
    data[0]=value;
    data[1]='\0';
    return _client.publish(topic,DefaultLogginQoS,true,data,1);
}

void MqttRemoteControl::_reportData(){
    _lastReportTime = millis();

    char data[BUFFERSIZE];

    uint16_t packetID=0;
    _publishing = false; // avoid race codition

    if(_reportFormat == MqttReportJson){

        const size_t len = nonNullJson(data,BUFFERSIZE);
        packetID=_client.publish(_reportBasePath,DefaultLogginQoS,true,data,len);
        DBG_PRINTF("Publish Json:%s\n",data);
    }else if(_reportFormat == MqttReportIndividual){
	    State state;
	    Mode mode;
	    float beerSet,fridgeSet;
	    float beerTemp,fridgeTemp,roomTemp;

	    brewPi.getAllStatus(state,mode,& beerTemp,& beerSet,& fridgeTemp,& fridgeSet,& roomTemp);

	    if(IS_FLOAT_TEMP_VALID(beerTemp)) _publish(KeyBeerTemp, beerTemp,1);
	    if(IS_FLOAT_TEMP_VALID(beerSet)) _publish(KeyBeerSet, beerSet,1);
	    if(IS_FLOAT_TEMP_VALID(fridgeTemp)) _publish(KeyFridgeTemp,fridgeTemp,1);
	    if(IS_FLOAT_TEMP_VALID(fridgeSet)) _publish(KeyFridgeSet, fridgeSet,1);
	    if(IS_FLOAT_TEMP_VALID(roomTemp)) _publish(KeyRoomTemp, roomTemp,1);

        packetID=_publish(KeyMode,(char)mode);
    	#if SupportPressureTransducer
	        if(PressureMonitor.isCurrentPsiValid()) packetID=_publish(KeyPressure,PressureMonitor.currentPsi(),1);
	    #endif
    	float sg=externalData.gravity();
	    if(IsGravityValid(sg)){
		    _publish(KeyGravity, sg,5);
		    packetID=_publish(KeyPlato, externalData.plato(),1);
	    }

    	// iSpindel data
	    float vol=externalData.deviceVoltage();
	    if(IsVoltageValid(vol)){
		    _publish(KeyVoltage, vol,2);
		    float at=externalData.auxTemp();
		    if(IS_FLOAT_TEMP_VALID(at)) _publish(KeyAuxTemp, at,1);
		    float tilt=externalData.tiltValue();
		    packetID=_publish(KeyTilt,tilt,2);
	    }
    }
    _lastPacketId = packetID;
    _publishing=true;
     DBG_PRINTF("Mqtt last packet ID: %d\n", packetID);
}

bool MqttRemoteControl::loop(){
    if(_reconnecting){
        DBG_PRINTF("MQTT:reconnecting..\n");
        if(_client.connected()){
            _client.disconnect();
        }
        // load
        if(_reloadConfig){
            _loadConfig();
            _reloadConfig = false;
        }
        // reconnect aagin in next loop, if necessary
        _reconnecting =false;
    }

    if(_mode == MqttModeOff) return false;

    if(_mode == MqttModeLogging){
        //logging only, not necessary to keep connected
        if( millis() - _lastReportTime  < _reportPeriod) return false;
    }
    // Control and/or logging mode
    if(! _client.connected()){
        // reconnect
        uint32_t now=millis();

        if(( (_connectAttempt < MaximumMqttConnectNumber) && (now - _connectTime > ReconnectTimer))
            || (now - _connectTime > ReconnectTimerLong)
            ){
            DBG_PRINTF("MQTT:reconnect..\n");

            _connectTime = now;
            _client.connect();
        }
    }else{
        // connected
        if(_mode == MqttModeBothControlLoggging || _mode== MqttModeLogging){
            if( millis() - _lastReportTime  > _reportPeriod){
                _reportData();
            }
        }
    }
    return true;
}

void MqttRemoteControl::_runModeCommand(){
    brewKeeper.setModeFromRemote(_lvMode);
}

void MqttRemoteControl::_loadConfig()
{
    MqttRemoteControlSettings *settings=theSettings.mqttRemoteControlSettings();

    _mode = settings->mode;
    
    if(_mode == MqttModeOff) return;
    _serverPort = settings->port;

    _serverAddress=settings->serverOffset? (char*)settings->_strings + settings->serverOffset:nullptr;

    _username = settings->usernameOffset? (char*)settings->_strings + settings->usernameOffset:nullptr;
    _password = settings->passwordOffset? (char*)settings->_strings + settings->passwordOffset:nullptr;


    #if SerialDebug
        DBG_PRINTF("MQTT load config, mode:%d\n",_mode);
        if(_serverAddress) DBG_PRINTF("server:%s\n",_serverAddress);
        if(_username) DBG_PRINTF("username:%s\n",_username);
        if(_password) DBG_PRINTF("_password:%s\n",_password);
    #endif

    if(_mode == MqttModeLogging || _mode == MqttModeBothControlLoggging){

        DBG_PRINTF("_reportPeriod:%d\n",settings->reportPeriod);
        DBG_PRINTF("_reportFormat:%d\n",settings->reportFormat);
        DBG_PRINTF("reportBasePathOffset:%d\n",settings->reportBasePathOffset);
        
        _reportPeriod = settings->reportPeriod * 1000;
        _reportFormat = settings->reportFormat;
        _reportBasePath =settings->reportBasePathOffset ? (char*)settings->_strings + settings->reportBasePathOffset:nullptr;

        if(_reportPeriod ==0 || _reportBasePath == nullptr){
            DBG_PRINTF("ERROR: %s: Invalid period, %d or invalid base path\n", __func__, _reportPeriod);
            _mode = (_mode == MqttModeBothControlLoggging)? MqttModeControl:MqttModeOff;
        }
    }

    if(_mode == MqttModeControl || _mode == MqttModeBothControlLoggging){

        _modePath = settings->modePathOffset? (char*)settings->_strings + settings->modePathOffset:nullptr;
        _beerSetPath = settings->beerSetPathOffset? (char*)settings->_strings + settings->beerSetPathOffset:nullptr;
        _fridgeSetPath = settings->fridgeSetPathOffset? (char*)settings->_strings + settings->fridgeSetPathOffset:nullptr;
        
#if EanbleParasiteTempControl
        _ptcPath = settings->ptcPathOffset? (char*)settings->_strings + settings->ptcPathOffset:nullptr;
#endif

#if Auto_CAP
        _capPath = settings->capControlPathOffset? (char*)settings->_strings + settings->capControlPathOffset:nullptr;
#endif


        #if SerialDebug
        if(_modePath) DBG_PRINTF("_modePath:%s\n",_modePath);
        if(_beerSetPath) DBG_PRINTF("_setTempPath:%s\n",_beerSetPath);
        if(_fridgeSetPath) DBG_PRINTF("_setTempPath:%s\n",_fridgeSetPath);

        #if EanbleParasiteTempControl

        if(_ptcPath) DBG_PRINTF("_ptcPath:%s\n",_ptcPath);
        #endif

        #if Auto_CAP
        if(_capPath) DBG_PRINTF("_capPath:%s\n",_capPath);
        #endif        
        #endif
    }

    _client.setServer(_serverAddress, _serverPort);
    _client.setCredentials(_username,_password);

}

bool MqttRemoteControl::begin()
{
    _loadConfig();
    _connectAttempt=0;
    _reloadConfig=false;
    _reconnecting=false;
    return false;
}

void MqttRemoteControl::reset()
{
    _connectAttempt=0;
    _reconnecting = true;
    _reloadConfig=true;
}


void MqttRemoteControl::_onConnect(){
    _connectAttempt =0;
    DBG_PRINTF("MQTT:connected..\n");

    if(_mode == MqttModeLogging) return;
    // subscribe
    if(_modePath){
        if(_client.subscribe(_modePath, 1)){
            DBG_PRINTF("MQTT:Subscribing %s\n",_modePath);
        }else{
            DBG_PRINTF("MQTT:Subscribing %s FAILED\n",_modePath);
        }
    }

    if(_beerSetPath){
        if(_client.subscribe(_beerSetPath, 1)){
            DBG_PRINTF("MQTT:Subscribing %s\n",_beerSetPath);
        }else{
            DBG_PRINTF("MQTT:Subscribing %s FAILED\n",_beerSetPath);
        }
    }

    if(_fridgeSetPath){
        if(_client.subscribe(_fridgeSetPath, 1)){
            DBG_PRINTF("MQTT:Subscribing %s\n",_fridgeSetPath);
        }else{
            DBG_PRINTF("MQTT:Subscribing %s FAILED\n",_fridgeSetPath);
        }
    }


    #if EanbleParasiteTempControl
    if(_ptcPath){
        if(_client.subscribe(_ptcPath, 1)){
            DBG_PRINTF("MQTT:Subscribing %s\n",_ptcPath);
        }else{
            DBG_PRINTF("MQTT:Subscribing %s FAILED\n",_ptcPath);
        }
    }
    #endif

    #if Auto_CAP
    if(_capPath){
        if(_client.subscribe(_capPath, 1)){
            DBG_PRINTF("MQTT:Subscribing %s\n",_capPath);
        }else{
            DBG_PRINTF("MQTT:Subscribing %s FAILED\n",_capPath);
        }
    }    
    #endif

}

void MqttRemoteControl::_onDisconnect(){
     DBG_PRINTF("\nMQTT:Disconnected.\n");
    _connectAttempt ++;
}


void MqttRemoteControl::_onMessage(char* topic, uint8_t* payload, size_t len) {
    DBG_PRINTF("MQTT:rcv %s\n",topic);

    if(strcmp(topic, _modePath) ==0){
        this->_onModeChange((char*)payload,len);
    }else if(strcmp(topic, _beerSetPath) ==0){
        this->_onSettingTempChange(true,(char*)payload,len);
    }else if(strcmp(topic, _fridgeSetPath) ==0){
        this->_onSettingTempChange(false,(char*)payload,len);
    }
#if EanbleParasiteTempControl
    else if(strcmp(topic, _ptcPath) ==0){
        this->_onPtcChange((char*)payload,len);
    }
#endif 
#if Auto_CAP
    else if(strcmp(topic, _capPath) ==0){
        this->_onCapChange((char*)payload,len);
    }
#endif
}

// Accepting mode and integer.
void MqttRemoteControl::_onModeChange(const char *payload, const std::size_t len)
{
    constexpr std::array<Mode, 4> modeChars{off, fridge_constant, beer_constant, beer_profile};

#if SerialDebug
    DBG_PRINTF("MQTT: Mode path value:");
    for(size_t i=0;i<len;i++)
        DBG_PRINTF("%c",payload[i]);
    DBG_PRINTF("\n");
#endif

    Mode mode;
    if (*payload >= '0' && *payload <= '3') {
        mode = modeChars[*payload - '0'];
    } else {
        if (std::none_of(modeChars.begin(), modeChars.end(),
                         [&](const char c) { return c == *payload; })) {
            DBG_PRINTF("MQTT: Error, unknown mode\n");
            return;
        }
        mode = static_cast<Mode>(*payload);
    }
    DBG_PRINTF("MQTT: Mode: %c\n", mode);
    if (_lvMode != mode) {
        _lvMode = mode;
        _runModeCommand();
    }
}

void MqttRemoteControl::_onSettingTempChange(bool isBeerSet,char* payload, size_t len){
    // assume it's just a simple float string.
    size_t toCopy=len;
    char *settingPtr=isBeerSet? _lvBeerSet:_lvFridgeSet;

    if(toCopy > MaxSettingLength) toCopy=MaxSettingLength;

    if(strncmp(settingPtr,payload,toCopy) !=0){
    
        memcpy(settingPtr,payload,toCopy);
        settingPtr[toCopy]='\0';

        DBG_PRINTF("MQTT:tempset :%s\n",settingPtr);
        if(isBeerSet)
            brewKeeper.setBeerSet(settingPtr);
        else
            brewKeeper.setFridgeSet(settingPtr);
        
        dataLogger.reportNow();
    }else{
        DBG_PRINTF("MQTT:Setting not changed\n");
    }
}

#if EanbleParasiteTempControl
void MqttRemoteControl::_onPtcChange(char* payload, size_t len){
    char buffer[32];
    size_t toCopy=len;
    if(toCopy > 31) toCopy=31;
    memcpy(buffer,payload,toCopy);
    buffer[toCopy]='\0';

    float temp= atof(buffer);
    
    parasiteTempController.setTemperatureRange(temp, temp + PtcRemoteControlRange);
}

#endif


#if Auto_CAP
void MqttRemoteControl::_onCapChange(char* payload,size_t len){
    bool mode;

    if(*payload >='0' && *payload <= '1'){
        mode = *payload != '0';
    }else{
        // char. check if it is valid
        if(strncmp(payload,"ON",2) ==0 || strncmp(payload,"on",2) ==0){
            mode = true;
        }else {
            mode =false;
        }
    }

    autoCapControl.capManualSet(mode);
}
#endif

#endif