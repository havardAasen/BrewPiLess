#include <Arduino.h>
#include <pgmspace.h>
#include "Config.h"

#ifndef NULL
#define NULL 0
#endif
#define Language english
#define STRINGIFY(str)  #str
#define PASTER(lo,file)   STRINGIFY(lo ## file )
#define EVALUATOR(l,x)  PASTER(l,x)

#define STRINGIFY(str)  #str
#define PASTER(lo,file)   STRINGIFY(lo ## file )
#define EVALUATOR(l,x)  PASTER(l,x)


#define IndexHtmFile EVALUATOR(WebPageLanguage,_index_htm.h)
#define ControlHtmFile EVALUATOR(WebPageLanguage,_control_htm.h)
#define SetupHtmFile EVALUATOR(WebPageLanguage,_setup_htm.h)
#define LogHtmFile EVALUATOR(WebPageLanguage,_log_htm.h)
#define GravityHtmFile EVALUATOR(WebPageLanguage,_gdc_htm.h)
#define ConfigHtmFile EVALUATOR(WebPageLanguage,_config_htm.h)
#define PressureHtmFile EVALUATOR(WebPageLanguage,_pressure_htm.h)
#define BundleJs EVALUATOR(WebPageLanguage,_bundle_js.h)



#if NoEmbeddedFile == true

const uint8_t* getEmbeddedFile(const char* filename,bool &gzip, unsigned int &size){
	return nullptr;
}

#else

struct EmbeddedFileMapEntry{
	const char *filename;
	const uint8_t *content;
    unsigned int size;
	bool  gzipped;
};

#include "testcmd_htm.h"
constexpr char file_testcmd_htm [] PROGMEM="/testcmd.htm";


constexpr char file_lcd_htm [] PROGMEM="/lcd";

#include "lcd_htm.h"

#include IndexHtmFile
#include ControlHtmFile
#include SetupHtmFile
#include LogHtmFile
#include GravityHtmFile
#include ConfigHtmFile
#include PressureHtmFile
#include BundleJs

constexpr char file_index_htm [] PROGMEM="/index.htm";
constexpr char file_bundle_js [] PROGMEM="/bundle.js";
constexpr char file_control_htm [] PROGMEM="/control.htm";
constexpr char file_setup_htm [] PROGMEM="/setup.htm";
constexpr char file_logconfig [] PROGMEM="/logging.htm";
constexpr char file_gravitydevice [] PROGMEM="/gravity.htm";
constexpr char file_config [] PROGMEM="/config.htm";
constexpr char file_pressure [] PROGMEM="/pressure.htm";

EmbeddedFileMapEntry fileMaps[]={
{file_index_htm,data_index_htm_gz,sizeof(data_index_htm_gz),true},
{file_bundle_js,data_bundle_js,sizeof(data_bundle_js),true},
{file_control_htm,control_htm_gz,sizeof(control_htm_gz),true},
{file_setup_htm,setup_htm_gz,sizeof(setup_htm_gz),true},
{file_logconfig,logging_htm_gz,sizeof(logging_htm_gz),true},
{file_gravitydevice,gravity_htm_gz,sizeof(gravity_htm_gz),true},
{file_config,config_htm_gz,sizeof(config_htm_gz),true},
{file_pressure,pressure_htm_gz,sizeof(pressure_htm_gz),true},
{file_testcmd_htm,testcmd_htm_gz,sizeof(testcmd_htm_gz),true},
{file_lcd_htm,lcd_htm_gz,sizeof(lcd_htm_gz),true}
};


const uint8_t* getEmbeddedFile(const char* filename,bool &gzip, unsigned int &size)
{
	for (auto& entry : fileMaps) {
		if (strcmp_P(filename, entry.filename) == 0) {
			gzip = entry.gzipped;
			size = entry.size;
			return entry.content;
		}
	}
	return nullptr;
}
#endif
