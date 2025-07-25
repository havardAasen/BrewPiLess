#ifndef MQTT_REMOTE_CONTROL_H
#define MQTT_REMOTE_CONTROL_H

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include <AsyncMqttClient.h>
#include <BPLSettings.h>

#include "Config.h"
#include "TempControl.h"

#define MaxSettingLength 31

#define MaximumMqttConnectNumber 5
#define ReconnectTimer 10000
#define ReconnectTimerLong 600000

#define CapStateOn 1
#define CapStateOff 0
#define CapStateUnknown 2

#define PtcRemoteControlRange 3

#define DefaultLogginQoS 1

class MqttRemoteControl{
protected:
    AsyncMqttClient _client;
    uint32_t _connectTime{};
    uint32_t _lastReportTime{};
    uint16_t _connectAttempt{};
    uint8_t _mode{MqttModeOff};
    uint8_t _reportFormat{};

    uint32_t _reportPeriod{};

    bool _reconnecting{};
    bool _reloadConfig{};
    Mode _lvMode{off};
    char _lvBeerSet[MaxSettingLength+1]{};
    char _lvFridgeSet[MaxSettingLength+1]{};

    char* _serverAddress{};
    uint16_t _serverPort{};
    char* _username{};
    char* _password{};

    char* _modePath{};
    char* _beerSetPath{};
    char* _fridgeSetPath{};
    
    char* _reportBasePath{};
    
    #if EanbleParasiteTempControl
    char* _ptcPath{};
    #endif

    #if Auto_CAP
    char* _capPath;
    #endif

    void _onConnect();
    void _onDisconnect();
    void _onMessage(char* topic, uint8_t* payload, size_t len);
    void _onPublish(uint16_t packetId);
    void _onModeChange(const char* payload, std::size_t len);

    void _onSettingTempChange(bool isBeerSet,char* payload, size_t len);

    void _runModeCommand();

#if EanbleParasiteTempControl
    void _onPtcChange(char* payload,size_t len);
#endif

#if Auto_CAP
    void _onCapChange(char* payload,size_t len);
#endif
    void _loadConfig();

    void _reportData();
    uint16_t _publish(const char* key,float value,int precision);
    uint16_t _publish(const char* key,char value);
    uint16_t _lastPacketId{};
    bool     _publishing{};
public:
    MqttRemoteControl();
    bool begin();
    void reset();

    bool disconnect();

    bool loop();
};

extern MqttRemoteControl mqttRemoteControl;

#endif
