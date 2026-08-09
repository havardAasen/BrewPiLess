#ifndef BREWPILESS_LOGHANDLER_H
#define BREWPILESS_LOGHANDLER_H

#include <ESPAsyncWebServer.h>

namespace bpl::webHandler {
    class LogHandler : public AsyncWebHandler {
    public:
        bool canHandle(AsyncWebServerRequest *request) const override;
        void handleRequest(AsyncWebServerRequest *request) override;

    private:
        void notifyLogStatus();
    };
}

#endif
