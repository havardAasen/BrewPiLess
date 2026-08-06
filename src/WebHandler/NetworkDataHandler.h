#ifndef BREWPILESS_NETWORKDATAHANDLER_H
#define BREWPILESS_NETWORKDATAHANDLER_H

#include <ESPAsyncWebServer.h>

namespace bpl::webHandler {
    class NetworkDataHandler final : public AsyncWebHandler {
    public:
        bool canHandle(AsyncWebServerRequest *request) const override;
        void handleRequest(AsyncWebServerRequest *request) override;
        [[nodiscard]] bool isRequestHandlerTrivial() const override;

    private:
        static void handleNetworkScan(AsyncWebServerRequest *request);
        static void handleNetworkConnect(AsyncWebServerRequest *request);
        static void handleNetworkDisconnect(AsyncWebServerRequest *request);
    };
}

#endif
