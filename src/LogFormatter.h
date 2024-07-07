#ifndef LOG_FORMATTER_H
#define LOG_FORMATTER_H

#define KeyBeerTemp "beerTemp"
#define KeyBeerSet "beerSet"
#define KeyFridgeTemp "fridgeTemp"
#define KeyFridgeSet "fridgeSet"
#define KeyRoomTemp "roomTemp"
#define KeyGravity "gravity"
#define KeyPlato "plato"
#define KeyAuxTemp "auxTemp"
#define KeyVoltage "voltage"
#define KeyTilt  "tilt"
#define KeyPressure "pressure"
#define KeyMode "mode"


size_t nonNullJson(char *buffer,size_t size);
size_t dataSprintf(char *buffer,const char *format,const char* invalidStr);


#endif