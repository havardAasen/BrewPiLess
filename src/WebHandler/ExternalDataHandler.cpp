#include "Config.h"
#include "ExternalDataHandler.h"

#include <ESPAsyncWebServer.h>

extern void stringAvailable(const char *);

namespace {
    namespace Local {
        constexpr char gravityDeviceConfigPath[] = "/gdc";
        constexpr char gravityFormulaPath[] = "/coeff";
        constexpr char gravityPath[] = "/gravity";
    }
}

bpl::webHandler::ExternalDataHandler::ExternalDataHandler()
{
    _data = &(_buffer[2]);
    _buffer[0] = 'G';
    _buffer[1] = ':';
}


void bpl::webHandler::ExternalDataHandler::loadConfig()
{
    externalData.loadConfig();
}


bool bpl::webHandler::ExternalDataHandler::canHandle(AsyncWebServerRequest *request) const
{
    DBG_PRINTF("req: %s\n", request->url().c_str());
    if (request->url() == Local::gravityPath) return true;
    if (request->url() == Local::gravityDeviceConfigPath) return true;
    if (request->url() == Local::gravityFormulaPath) return true;

    return false;
}


void bpl::webHandler::ExternalDataHandler::handleRequest(AsyncWebServerRequest *request)
{
    if (request->url() == Local::gravityPath) {
        if (request->method() != HTTP_POST) {
            request->send(400);
            return;
        }
        stringAvailable(_buffer);
        processGravity(request, _data, _dataLength);
        // Process the name
        externalData.sseNotify(_data);
        stringAvailable(_data);
        return;
    }
    if (request->url() == Local::gravityFormulaPath) {
        if (request->hasParam("a0") && request->hasParam("a1")
            && request->hasParam("a2") && request->hasParam("a3")
            && request->hasParam("pt")) {
            float coeff[4];
            coeff[0] = request->getParam("a0")->value().toFloat();
            coeff[1] = request->getParam("a1")->value().toFloat();
            coeff[2] = request->getParam("a2")->value().toFloat();
            coeff[3] = request->getParam("a3")->value().toFloat();
            uint32_t npt = (uint32_t) request->getParam("pt")->value().toInt();
            externalData.formula(coeff, npt);

            brewLogger.addIgnoredCalPointMask(npt & 0xFFFFFF);

            request->send(201);
        } else {
            DBG_PRINTF("Invalid parameter\n");
            request->send(400);
        }

        return;
    }
    // config
    if (request->method() == HTTP_POST) {
        if (externalData.processconfig(_data)) {
            request->send(201);
        } else {
            request->send(400);
        }
    } else {
        // get
        if (request->hasParam("data")) {
            request->send(200, asyncsrv::T_application_json, theSettings.jsonGravityConfig());
        } else {
            // get the HTML
            request->redirect(request->url() + asyncsrv::T__htm);
            //request->send_P(200, asyncsrv::T_text_html, externalData.html());
        }
    }
}


void bpl::webHandler::ExternalDataHandler::handleBody(AsyncWebServerRequest *request, uint8_t *data,
                                                      size_t len, size_t index, size_t total)
{
    if (!index) {
        DBG_PRINTF("BodyStart-len:%d total: %u\n", len, total);
        _dataLength = 0;
        _error = (total >= maxDataSize);
    }

    if (_error) return;
    for (size_t i = 0; i < len; i++) {
        //Serial.write(data[i]);
        _data[_dataLength++] = data[i];
    }
    if (index + len >= total) {
        _data[_dataLength] = '\0';
        DBG_PRINTF("Body total%u data:%s\n", total, _data);
    }
}


bool bpl::webHandler::ExternalDataHandler::isRequestHandlerTrivial() const
{
    return false;
}


void bpl::webHandler::ExternalDataHandler::processGravity(AsyncWebServerRequest *request,
                                                          char data[], size_t length)
{
    if (length == 0) return request->send(500);;
    SystemConfiguration *syscfg = theSettings.systemConfiguration();
    uint8_t error;
    if (externalData.processGravityReport(data, length,
                                          request->authenticate(syscfg->username, syscfg->password),
                                          error)) {
        request->send(202);
    } else {
        if (error == ErrorAuthenticateNeeded) return request->requestAuthentication();
        else request->send(500);
    }
}
