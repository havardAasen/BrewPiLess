#ifndef BREWPILESS_BREWPIDATAHANDLER_H
#define BREWPILESS_BREWPIDATAHANDLER_H

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

        [[nodiscard]] bool fileExists(const String &path) const;
        void sendProgmem(AsyncWebServerRequest *request, const char *html);
        void sendFile(AsyncWebServerRequest *request, const String &path);
    };
}

#endif
