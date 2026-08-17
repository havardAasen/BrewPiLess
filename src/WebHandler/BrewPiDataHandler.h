#ifndef BREWPILESS_BREWPIDATAHANDLER_H
#define BREWPILESS_BREWPIDATAHANDLER_H

#include "BPLSettings.h"

#include <ESPAsyncWebServer.h>

namespace bpl::webHandler {
    class BrewPiDataHandler : public AsyncWebHandler {
    public:
        bool canHandle(AsyncWebServerRequest *request) const override;
        void handleRequest(AsyncWebServerRequest *request) override;
        [[nodiscard]] bool isRequestHandlerTrivial() const override;

    private:
        static void handleFileList(AsyncWebServerRequest *request);
        static void handleFileDelete(AsyncWebServerRequest *request);
        static void handleFilePuts(AsyncWebServerRequest *request);
        static void handleBeerProfile(AsyncWebServerRequest *request,
                                      const SystemConfiguration &sysCfg);
        static void handleCapper(AsyncWebServerRequest *request, const SystemConfiguration &sysCfg);
        static void handleConfig(AsyncWebServerRequest *request, const SystemConfiguration &sysCfg);
        static void handleLogging(AsyncWebServerRequest *request,
                                  const SystemConfiguration &sysCfg);
        static void handleMqtt(AsyncWebServerRequest *request, const SystemConfiguration &sysCfg);
        static void handleStatus(AsyncWebServerRequest *request);
        static void handleTime(AsyncWebServerRequest *request);
        static void handleParasiteTempControl(AsyncWebServerRequest *request);
        static void handlePressure(AsyncWebServerRequest *request,
                                   const SystemConfiguration &sysCfg);

        [[nodiscard]] bool fileExists(const String &path) const;
        void sendProgmem(AsyncWebServerRequest *request, const char *html);
        void sendFile(AsyncWebServerRequest *request, const String &path);
    };
}

#endif
