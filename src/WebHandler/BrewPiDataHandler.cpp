#include "AutoCapControl.h"
#include "BPLSettings.h"
#include "BrewKeeper.h"
#include "BrewPiDataHandler.h"
#include "BrewPiProxy.h"
#include "Config.h"
#include "Display.h"
#include "MqttRemoteControl.h"
#include "ParasiteTempController.h"
#include "PressureMonitor.h"
#include "TimeKeeper.h"
#include "WiFiSetup.h"
#include "WatchdogTimerHelper.h"

#include <ESPAsyncWebServer.h>
#include <LittleFS.h>


namespace {
    namespace Local {
        constexpr char configPath[] = "/config";
        constexpr char timePath[] = "/time";
        constexpr char resetWifiPath[] = "/erasewifisetting";
        constexpr char fputsPath[] = "/fputs";
        constexpr char flistPath[] = "/list";
        constexpr char deletePath[] = "/rm";
        constexpr char beerProfilePath[] = "/tschedule";
        constexpr char getStatusPath[] = "/getstatus";
        constexpr char mqttPath[] = "/mqtt";
        constexpr char parasiteTempControlPath[] = "/ptc";
        constexpr char loggingPath[] = "/log";
        constexpr char capperPath[] = "/cap";
        constexpr char pressurePath[] = "/psi";

        constexpr char defaultIndexFile[] = "index.htm";
    }
}

const char *public_list[] = {
    "/bwf.js",
    "/brewing.json"
};

const char *nocache_list[] = {
    "/brewing.json",
    "/brewpi.cfg"
};

extern String getContentType(String filename);
extern const uint8_t *getEmbeddedFile(const char *filename, bool &gzip, unsigned int &size);
extern void requestRestart(bool disc);

#if AUTO_CAP
extern void capStatusReport();
#endif


bool bpl::webHandler::BrewPiDataHandler::canHandle(AsyncWebServerRequest *request) const
{
    if (request->method() == HTTP_GET) {
        if (request->url() == Local::configPath
            || request->url() == Local::timePath
            || request->url() == Local::flistPath
            || request->url() == Local::resetWifiPath
            || request->url() == Local::getStatusPath
            || request->url() == Local::beerProfilePath
            || request->url() == Local::mqttPath
#ifdef ENABLE_LOGGING
            || request->url() == Local::loggingPath
#endif
#if EanbleParasiteTempControl
            || request->url() == Local::parasiteTempControlPath
#endif
#if AUTO_CAP
            || request->url() == Local::capperPath
#endif
#if SupportPressureTransducer
            || request->url() == Local::pressurePath
#endif
        ) {
            return true;
        } else {
            // get file
            String path = request->url();
            if (path.endsWith("/")) path += Local::defaultIndexFile;
            //DBG_PRINTF("request:%s\n",path.c_str());
            if (fileExists(path)) return true; //if(LittleFS.exists(path)) return true;
            //DBG_PRINTF("request:%s not found\n",path.c_str());
        }
    } else if (request->method() == HTTP_DELETE && request->url() == Local::deletePath) {
        return true;
    } else if (request->method() == HTTP_POST) {
        if (request->url() == Local::configPath
            || request->url() == Local::fputsPath
            || request->url() == Local::timePath
            || request->url() == Local::beerProfilePath
            || request->url() == Local::mqttPath
#ifdef ENABLE_LOGGING
            || request->url() == Local::loggingPath
#endif
#if EanbleParasiteTempControl
            || request->url() == Local::parasiteTempControlPath
#endif
#if SupportPressureTransducer
            || request->url() == Local::pressurePath
#endif
        )
            return true;
    }
    return false;
}


void bpl::webHandler::BrewPiDataHandler::handleRequest(AsyncWebServerRequest *request)
{
    SystemConfiguration *syscfg = theSettings.systemConfiguration();

    if (request->url() == Local::configPath) handleConfig(request, *syscfg);
    else if (request->url() == Local::timePath) handleTime(request);
    else if (request->url() == Local::beerProfilePath) handleBeerProfile(request, *syscfg);
    else if (request->method() == HTTP_GET && request->url() == Local::getStatusPath)
        handleStatus(request);
#if SupportMqttRemoteControl
    else if (request->url() == Local::mqttPath) handleMqtt(request, *syscfg);
#endif
#ifdef ENABLE_LOGGING
    else if (request->url() == Local::loggingPath) handleLogging(request, *syscfg);
#endif
#if EanbleParasiteTempControl
    else if (request->url() == Local::parasiteTempControlPath) handleParasiteTempControl(request);
#endif
#if AUTO_CAP
    else if (request->url() == Local::capperPath) handleCapper(request, *syscfg);
#endif
#if SupportPressureTransducer
    else if (request->url() == Local::pressurePath) handlePressure(request, *syscfg);
#endif
    // Every call below this point requires authentication
    if (!request->authenticate(syscfg->username, syscfg->password))
        return request->requestAuthentication();

    if (request->method() == HTTP_GET && request->url() == Local::resetWifiPath) {
        request->send(200, asyncsrv::T_text_html, "Done, restarting..");
        requestRestart(true);
    } else if (request->method() == HTTP_GET && request->url() == Local::flistPath) {
        handleFileList(request);
    } else if (request->method() == HTTP_DELETE && request->url() == Local::deletePath) {
        handleFileDelete(request);
    } else if (request->method() == HTTP_POST && request->url() == Local::fputsPath) {
        handleFilePuts(request);
    } else if (request->method() == HTTP_GET) {
        String path = request->url();
        if (path.endsWith("/")) path += Local::defaultIndexFile;

        if (request->url().equals("/")) {
            if (!syscfg->passwordLcd) {
                sendFile(request, path);
                return;
            }
        }

        if (syscfg->passwordLcd && !request->authenticate(syscfg->username, syscfg->password))
            return request->requestAuthentication();

        sendFile(request, path);
    }
}


bool bpl::webHandler::BrewPiDataHandler::isRequestHandlerTrivial() const
{
    return false;
}


void bpl::webHandler::BrewPiDataHandler::handleFileList(AsyncWebServerRequest *request)
{
    if (!request->hasParam("dir")) {
        request->send(400);
        return;
    }

    const String path = request->getParam("dir")->value();
    Dir dir = LittleFS.openDir(path);

    String output = "[";
    while (dir.next()) {
        if (output != "[") {
            output += ',';
        }
        output += R"({"type":")";
        output += dir.isDirectory() ? "dir" : "file";
        output += R"(","name":")";
        output += dir.fileName();
        output += "\"}";
    }
    output += "]";
    request->send(200, asyncsrv::T_application_json, output);
}


void bpl::webHandler::BrewPiDataHandler::handleFileDelete(AsyncWebServerRequest *request)
{
    if (!request->hasParam("path")) {
        request->send(400);
        return;
    }

    bpl::watchdog::disable();
    LittleFS.remove(request->getParam("path")->value());
    bpl::watchdog::enable();
    request->send(204);
}


void bpl::webHandler::BrewPiDataHandler::handleFilePuts(AsyncWebServerRequest *request)
{
    if (!request->hasParam("path") && !request->hasParam("content", true)) {
        request->send(400);
        return;
    }

    EspClass::wdtDisable();
    bpl::watchdog::disable();
    const String file = request->getParam("path")->value();
    File fh = LittleFS.open(file, "w");
    if (!fh) {
        request->send(404);
        return;
    }
    fh.print(request->getParam("content", true)->value());
    fh.close();
    bpl::watchdog::enable();
    request->send(201);
    DBG_PRINTF("fputs path=%s\n", file.c_str());
}


void bpl::webHandler::BrewPiDataHandler::handleBeerProfile(AsyncWebServerRequest *request,
                                                           const SystemConfiguration &sysCfg)
{
    if (request->method() == HTTP_GET) {
        request->send(200, asyncsrv::T_application_json, theSettings.jsonBeerProfile());
        return;
    }

    if (!request->authenticate(sysCfg.username, sysCfg.password))
        return request->requestAuthentication();

    if (request->hasParam("data", true)) {
        if (theSettings.dejsonBeerProfile(request->getParam("data", true)->value())) {
            theSettings.save();
            brewKeeper.profileUpdated();
            request->send(201);
        } else
            request->send(402);

        return;
    }

    request->send(401);
}


void bpl::webHandler::BrewPiDataHandler::handleCapper(AsyncWebServerRequest *request,
                                                      const SystemConfiguration &sysCfg)
{
    if (!request->authenticate(sysCfg.username, sysCfg.password))
        return request->requestAuthentication();

    if (request->hasParam("psi")) {
        theSettings.pressureMonitorSettings()->psi = request->getParam("psi")->value().
                toInt();
        DBG_PRINTF("set pressure:%d", theSettings.pressureMonitorSettings()->psi);
    }
    bool response = true;
    if (request->hasParam("cap")) {
        const AsyncWebParameter *value = request->getParam("cap");
        autoCapControl.capManualSet(value->value().toInt() != 0);
        // manual
    } else if (request->hasParam("at")) {
        // time
        const AsyncWebParameter *value = request->getParam("at");
        autoCapControl.capAtTime(value->value().toInt());
    } else if (request->hasParam("sg")) {
        // gravity
        const AsyncWebParameter *value = request->getParam("sg");
        autoCapControl.catOnGravity(value->value().toFloat());
    } else {
        request->send(400);
        response = false;
    }
    if (response) request->send(202);
    capStatusReport();
}


void bpl::webHandler::BrewPiDataHandler::handleConfig(AsyncWebServerRequest *request,
                                                      const SystemConfiguration &sysCfg)
{
    if (!request->authenticate(sysCfg.username, sysCfg.password))
        return request->requestAuthentication();

    if (request->method() == HTTP_GET) {
        if (request->hasParam("cfg"))
            request->send(200, asyncsrv::T_application_json,
                          theSettings.jsonSystemConfiguration());
        else
            request->redirect(request->url() + asyncsrv::T__htm);

        return;
    }

    if (request->method() == HTTP_POST) {
        if (!request->hasParam("data", true)) {
            request->send(400);
            DBG_PRINTF("no data in post\n");
            return;
        }

        DBG_PRINTF("Config to save: %s\n",
                   request->getParam("data", true)->value().c_str());

        if (theSettings.
            dejsonSystemConfiguration(request->getParam("data", true)->value())) {
            theSettings.save();
            DBG_PRINT("Config saved\n");
            request->send(201);
            display.setAutoOffPeriod(theSettings.systemConfiguration()->backlite);

            WiFiSetup.setMode(
                static_cast<WiFiMode>(theSettings.systemConfiguration()->wifiMode));

            if (!request->hasParam("nb")) {
                requestRestart(false);
            }
            return;
        }

        request->send(400, asyncsrv::T_text_plain, "Invalid configuration data.");
    }
}


void bpl::webHandler::BrewPiDataHandler::handleLogging(AsyncWebServerRequest *request,
                                                       const SystemConfiguration &sysCfg)
{
    if (request->method() == HTTP_POST) {
        if (!request->authenticate(sysCfg.username, sysCfg.password))
            return request->requestAuthentication();

        if (request->hasParam("data", true)) {
            if (theSettings.dejsonRemoteLogging(request->getParam("data", true)->value())) {
                request->send(202);
                theSettings.save();
            } else {
                request->send(401);
            }
        } else {
            request->send(404);
        }
    } else {
        if (request->hasParam("data")) {
            request->send(200, asyncsrv::T_application_json,
                          theSettings.jsonRemoteLogging());
        } else {
            request->redirect(request->url() + asyncsrv::T__htm);
        }
    }
}


void bpl::webHandler::BrewPiDataHandler::handleMqtt(AsyncWebServerRequest *request,
                                                    const SystemConfiguration &sysCfg)
{
    if (request->method() == HTTP_GET) {
        request->send(200, asyncsrv::T_application_json,
                      theSettings.jsonMqttRemoteControlSettings());
        return;
    }

    if (request->method() == HTTP_POST) {
        if (!request->authenticate(sysCfg.username, sysCfg.password))
            return request->requestAuthentication();

        if (!request->hasParam("data", true)) {
            request->send(400);
            DBG_PRINTF("no data in post\n");
        }

        if (theSettings.dejsonMqttRemoteControlSettings(request->getParam("data", true)->value())) {
            theSettings.save();
            request->send(201);
            mqttRemoteControl.reset();
        } else {
            request->send(500);
            DBG_PRINTF("json format error\n");
        }
    }
}


void bpl::webHandler::BrewPiDataHandler::handleStatus(AsyncWebServerRequest *request)
{
    Mode mode;
    State state;
    float beerSet, beerTemp, fridgeTemp, fridgeSet, roomTemp;
    brewPi.getAllStatus(state, mode, &beerTemp, &beerSet, &fridgeTemp, &fridgeSet,
                        &roomTemp);
#define TEMPorNull(a) (IS_FLOAT_TEMP_VALID(a)?  String(a):String("null"))
    String json = String("{\"mode\":\"") + String((char) mode)
                  + String("\",\"state\":") + String(state)
                  + String(",\"beerSet\":") + TEMPorNull(beerSet)
                  + String(",\"beerTemp\":") + TEMPorNull(beerTemp)
                  + String(",\"fridgeSet\":") + TEMPorNull(fridgeSet)
                  + String(",\"fridgeTemp\":") + TEMPorNull(fridgeTemp)
                  + String(",\"roomTemp\":") + TEMPorNull(roomTemp)
                  + String("}");
    request->send(200, asyncsrv::T_application_json, json);
}


void bpl::webHandler::BrewPiDataHandler::handleTime(AsyncWebServerRequest *request)
{
    if (request->method() == HTTP_GET) {
        AsyncResponseStream *response = request->beginResponseStream(asyncsrv::T_application_json);
        response->printf("{\"t\":\"%s\",\"e\":%lld,\"o\":%d}", TimeKeeper.getDateTimeStr(),
                         static_cast<std::int64_t>(TimeKeeper.getTimeSeconds()),
                         TimeKeeper.getTimezoneOffset());
        request->send(response);
    }

    if (request->method() == HTTP_POST) {
        if (request->hasParam("time", true)) {
            const AsyncWebParameter *tvalue = request->getParam("time", true);
            const auto time = tvalue->value().toInt();
            DBG_PRINTF("Set Time:%lld from:%s\n", static_cast<std::int64_t>(time),
                       tvalue->value().c_str());
            TimeKeeper.setCurrentTime(time);
        }
        if (request->hasParam("off", true)) {
            const AsyncWebParameter *tvalue = request->getParam("off", true);
            DBG_PRINTF("Set timezone:%ld\n", tvalue->value().toInt());
            TimeKeeper.setTimezoneOffset(tvalue->value().toInt());
        }
        request->send(202);
    }
}


void bpl::webHandler::BrewPiDataHandler::handleParasiteTempControl(
    AsyncWebServerRequest *request)
{
    if (request->method() == HTTP_POST) {
        if (request->hasParam("c", true)) {
            String content = request->getParam("c", true)->value();
            if (parasiteTempController.updateSettings(content))
                request->send(201);
            else
                request->send(400);
        } else
            request->send(404);
    } else {
        String status = parasiteTempController.getSettings();
        request->send(200, asyncsrv::T_application_json, status);
    }
}


void bpl::webHandler::BrewPiDataHandler::handlePressure(AsyncWebServerRequest *request,
                                                        const SystemConfiguration &sysCfg)
{
    if (!request->authenticate(sysCfg.username, sysCfg.password))
        return request->requestAuthentication();

    if (request->method() == HTTP_GET) {
        if (request->hasParam("r")) {
            int reading = PressureMonitor.currentAdcReading();
            request->send(200, asyncsrv::T_application_json,
                          String("{\"a0\":") + String(reading) + String("}"));
        } else {
            request->send(200, asyncsrv::T_application_json,
                          theSettings.jsonPressureMonitorSettings());
        }
        return;
    }

    if (request->method() == HTTP_POST) {
        if (request->hasParam("data", true)) {
            if (theSettings.dejsonPressureMonitorSettings(
                request->getParam("data", true)->value())) {
                theSettings.save();
                request->send(201);
                return;
            }

            DBG_PRINTF("invalid JSON\n");
        } else {
            DBG_PRINTF("no data\n");
        }
    request->send(401);
    }
}


bool bpl::webHandler::BrewPiDataHandler::fileExists(const String &path) const
{
    if (LittleFS.exists(path)) return true;
    bool dum;
    unsigned int dum2;

    if (getEmbeddedFile(path.c_str(), dum, dum2)) return true;

    const String pathWithGz = path + asyncsrv::T__gz;
    if (LittleFS.exists(pathWithGz)) return true;

    return false;
}


void bpl::webHandler::BrewPiDataHandler::sendProgmem(AsyncWebServerRequest *request,
                                                     const char *html)
{
    AsyncWebServerResponse *response = request->beginResponse(String(asyncsrv::T_text_html),
                                                              strlen_P(html),
                                                              [=](uint8_t *buffer, size_t maxLen,
                                                          size_t alreadySent) -> size_t {
                                                                  if (strlen_P(
                                                                      html+alreadySent) > maxLen) {
                                                                      memcpy_P((char *) buffer,
                                                                          html + alreadySent,
                                                                          maxLen);
                                                                      return maxLen;
                                                                  }
                                                                  // Ok, last chunk
                                                                  memcpy_P((char *) buffer,
                                                                      html + alreadySent,
                                                                      strlen_P(html+alreadySent));
                                                                  return strlen_P(html+alreadySent);
                                                                  // Return from here to end of indexhtml
                                                              });
    response->addHeader(asyncsrv::T_Cache_Control, "max-age=2592000");
    request->send(response);
}


void bpl::webHandler::BrewPiDataHandler::sendFile(AsyncWebServerRequest *request,
                                                  const String &path)
{
    String pathWithGz = path + asyncsrv::T__gz;
    if (LittleFS.exists(pathWithGz)) {
        File file = LittleFS.open(pathWithGz, "r");
        if (!file) {
            request->send(500);
            return;
        }
        AsyncWebServerResponse *response = request->beginResponse(
            file, path, getContentType(path));
        //			response->addHeader(asyncsrv::T_Content_Encoding, asyncsrv::T_gzip);
        response->addHeader(asyncsrv::T_Cache_Control, "max-age=2592000");
        request->send(response);
        return;
    }

    if (LittleFS.exists(path)) {
        //request->send(LittleFS, path);
        const bool nocache = std::any_of(std::cbegin(nocache_list),
                                         std::cend(nocache_list), [&](const char *p) {
                                             return path.equals(p);
                                         });

        AsyncWebServerResponse *response = request->beginResponse(LittleFS, path, "");
        if (nocache)
            response->addHeader(asyncsrv::T_Cache_Control, asyncsrv::T_no_cache);
        else
            response->addHeader(asyncsrv::T_Cache_Control, "max-age=2592000");
        request->send(response);
        return;
    }

    // Embedded HTML or JS file
    bool gzip;
    uint32_t size;
    if (const uint8_t *file = getEmbeddedFile(path.c_str(), gzip, size)) {
        assert(gzip == true && "All files must be gzipped");
        DBG_PRINTF("using embedded file: '%s'\n", path.c_str());
        AsyncWebServerResponse *response = request->beginResponse_P(
            200, getContentType(path), file, size);
        response->addHeader(asyncsrv::T_Cache_Control, "max-age=2592000");
        response->addHeader(asyncsrv::T_Content_Encoding, asyncsrv::T_gzip);
        request->send(response);
    }
}
