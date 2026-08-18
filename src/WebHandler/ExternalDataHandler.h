#ifndef BREWPILESS_EXTERNALDATAHANDLER_H
#define BREWPILESS_EXTERNALDATAHANDLER_H

#include "ExternalData.h"

#include <ESPAsyncWebServer.h>

namespace bpl::webHandler {
    class ExternalDataHandler : public AsyncWebHandler {
    public:
        ExternalDataHandler();

        bool canHandle(AsyncWebServerRequest *request) const override;
        void handleRequest(AsyncWebServerRequest *request) override;
        void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index,
                        size_t total) override;
        [[nodiscard]] bool isRequestHandlerTrivial() const override;

    private:
        void processGravity(AsyncWebServerRequest *request, char data[], size_t length);

        static constexpr std::uint16_t maxDataSize{256};
        char _buffer[maxDataSize + 2];
        char *_data;
        size_t _dataLength;
        bool _error;
    };
}

#endif
