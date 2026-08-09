#include "BrewLogger.h"
#include "Config.h"
#include "ExternalData.h"
#include "LogHandler.h"

#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

extern void stringAvailable(const char *);

namespace {
    namespace Local {
        constexpr char logListPath[] = "/loglist.php";
        constexpr char chartDataPath[] = "/chart.php";
    }
}


bool bpl::webHandler::LogHandler::canHandle(AsyncWebServerRequest *request) const
{
    if (request->url() == Local::chartDataPath || request->url() == Local::logListPath
        /*|| request->url() == IGNORE_MASK_PATH */)
        return true;
    return false;
}


void bpl::webHandler::LogHandler::handleRequest(AsyncWebServerRequest *request)
{
    /*		if( request->url() == IGNORE_MASK_PATH){
                if(request->hasParam("m")){
                    uint32_t mask= request->getParam("m")->value().toInt();
                    brewLogger.addIgnoredCalPointMask(mask);
                    request->send(200,asyncsrv::T_application_json,"{}");
                }else{
                    request->send(404);
                }
            }else */
    if (request->url() == Local::logListPath) {
        if (request->hasParam("dl")) {
            int index = request->getParam("dl")->value().toInt();
            char buf[36];
            brewLogger.getFilePath(buf, index);
            if (LittleFS.exists(buf)) {
                request->send(LittleFS, buf, asyncsrv::T_application_octet_stream, true);
            } else {
                request->send(404);
            }
        } else if (request->hasParam("rm")) {
            int index = request->getParam("rm")->value().toInt();
            DBG_PRINTF("Delete log file %d\n", index);
            brewLogger.rmLog(index);

            request->send(200, asyncsrv::T_application_json, brewLogger.fsinfo());
        } else if (request->hasParam("start")) {
            String filename = request->getParam("start")->value();
            DBG_PRINTF("start logging:%s\n", filename.c_str());
            bool cal = false;
            float tiltwater, hydroreading;
            if (request->hasParam("tw") && request->hasParam("hr")) {
                cal = true;
                tiltwater = request->getParam("tw")->value().toFloat();
                hydroreading = request->getParam("hr")->value().toFloat();
            }

            if (brewLogger.startSession(filename.c_str(), cal)) {
                if (cal) {
                    brewLogger.addTiltInWater(tiltwater, hydroreading);
                    externalData.setCalibrating(true);
                    DBG_PRINTF("Start BrweNCal log\n");
                }

                brewLogger.addCorrectionTemperature(externalData.hydrometerCalibration());

                request->send(202);
                notifyLogStatus();
            } else
                request->send(404);
        } else if (request->hasParam("stop")) {
            DBG_PRINTF("Stop logging\n");
            brewLogger.endSession();
            externalData.setCalibrating(false);
            request->send(202);
            notifyLogStatus();
        } else {
            // default. list information
            String status = brewLogger.loggingStatus();
            request->send(200, asyncsrv::T_application_json, status);
        }
        return;
    } // end of logist path
    // charting

    int offset;
    if (request->hasParam("offset")) {
        offset = request->getParam("offset")->value().toInt();
        //DBG_PRINTF("offset= %d\n",offset);
    } else {
        offset = 0;
    }

    size_t index;
    bool indexValid;
    if (request->hasParam("index")) {
        index = request->getParam("index")->value().toInt();
        //DBG_PRINTF("index= %d\n",index);
        indexValid = true;
    } else {
        indexValid = false;
    }

    if (!brewLogger.isLogging()) {
        // volatile logging
        if (!indexValid) {
            // client in Logging mode. force to reload
            offset = 0;
            index = 0;
        }
        size_t size = brewLogger.volatileDataAvailable(index, offset);
        size_t logoffset = brewLogger.volatileDataOffset();

        if (size > 0) {
            AsyncWebServerResponse *response = request->beginResponse(
                asyncsrv::T_application_octet_stream, size,
                [](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
                    return brewLogger.readVolatileData(buffer, maxLen, index);
                });
            response->addHeader("LogOffset", String(logoffset));
            request->send(response);
        } else {
            request->send(204);
        }
    } else {
        if (indexValid) {
            // client in volatile Logging mode. force to reload
            offset = 0;
        }

        size_t size = brewLogger.beginCopyAfter(offset);
        if (size > 0) {
            request->send(asyncsrv::T_application_octet_stream, size,
                          [](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
                              return brewLogger.read(buffer, maxLen, index);
                          });
        } else {
            request->send(204);
        }
    }
}


void bpl::webHandler::LogHandler::notifyLogStatus()
{
    externalData.waitFormula();
    const char *logname = brewLogger.currentLog();
    String logstr = (logname) ? String(logname) : String("");
    String status = String("A:{\"reload\":\"chart\", \"log\":\"") + logstr + String("\"}");
    stringAvailable(status.c_str());
}
