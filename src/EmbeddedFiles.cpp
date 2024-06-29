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

#include "bwf_js.h"
const char file_bwf_js [] PROGMEM="/bwf.js";

#include "testcmd_htm.h"
const char file_testcmd_htm [] PROGMEM="/testcmd.htm";

#include "dygraph_js.h"


const char file_lcd_htm [] PROGMEM="/lcd";

#include "lcd_htm.h"

#include IndexHtmFile
#include ControlHtmFile
#include SetupHtmFile
#include LogHtmFile
#include GravityHtmFile
#include ConfigHtmFile
#include PressureHtmFile

const char file_index_htm [] PROGMEM="/index.htm";
const char file_dygraph_js [] PROGMEM="/dygraph-combined.js";
const char file_control_htm [] PROGMEM="/control.htm";
const char file_setup_htm [] PROGMEM="/setup.htm";
const char file_logconfig [] PROGMEM="/logging.htm";
const char file_gravitydevice [] PROGMEM="/gravity.htm";
const char file_config [] PROGMEM="/config.htm";
const char file_pressure [] PROGMEM="/pressure.htm";

EmbeddedFileMapEntry fileMaps[]={
{file_bwf_js,data_bwf_min_js_gz,sizeof(data_bwf_min_js_gz),true},
{file_index_htm,data_index_htm_gz,sizeof(data_index_htm_gz),true},
{file_dygraph_js,dygraph_combined_js_gz,sizeof(dygraph_combined_js_gz),true},
{file_control_htm,control_htm_gz,sizeof(control_htm_gz),true},
{file_setup_htm,setup_htm_gz,sizeof(setup_htm_gz),true},
{file_logconfig,logging_htm_gz,sizeof(logging_htm_gz),true},
{file_gravitydevice,gravity_htm_gz,sizeof(gravity_htm_gz),true},
{file_config,config_htm_gz,sizeof(config_htm_gz),true},
{file_pressure,pressure_htm_gz,sizeof(pressure_htm_gz),true},
{file_testcmd_htm,(const uint8_t *)data_testcmd_htm,0,false},
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
