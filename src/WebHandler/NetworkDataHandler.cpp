#include "BPLSettings.h"
#include "NetworkDataHandler.h"
#include "WiFiSetup.h"

namespace {
    namespace Local {
        constexpr char scan[] = "/wifiscan";
        constexpr char connect[] = "/wificon";
        constexpr char disconnect[] = "/wifidisc";

        IPAddress scanIP(char const *str)
        {
            // DBG_PRINTF("Scan IP length=%d :\"%s\"\n",len,buffer);
            // this doesn't work. the last byte always 0: ip.fromString(buffer);

            std::array<std::uint8_t, 4> Parts{};
            std::uint8_t Part = 0;
            char *ptr = (char *) str;
            for (; *ptr; ptr++) {
                char c = *ptr;
                if (c == '.') {
                    Part++;
                    continue;
                }
                Parts[Part] *= 10;
                Parts[Part] += c - '0';
            }

            IPAddress sip(Parts[0], Parts[1], Parts[2], Parts[3]);
            return sip;
        }
    }
}


bool bpl::webHandler::NetworkDataHandler::canHandle(AsyncWebServerRequest *request) const
{
    if (request->url() == Local::scan) return true;
    else if (request->url() == Local::connect) return true;
    else if (request->url() == Local::disconnect) return true;

    return false;
}


void bpl::webHandler::NetworkDataHandler::handleRequest(AsyncWebServerRequest *request)
{
    if (request->url() == Local::scan) handleNetworkScan(request);
    else if (request->url() == Local::connect) handleNetworkConnect(request);
    else if (request->url() == Local::disconnect) handleNetworkDisconnect(request);
}


bool bpl::webHandler::NetworkDataHandler::isRequestHandlerTrivial() const
{
    return false;
}


void bpl::webHandler::NetworkDataHandler::handleNetworkScan(AsyncWebServerRequest *request)
{
    if (WiFiSetup.requestScanWifi())
        request->send(202);
    else
        request->send(403);
}


void bpl::webHandler::NetworkDataHandler::handleNetworkConnect(AsyncWebServerRequest *request)
{
    if (!request->hasParam("nw", true)) {
        request->send(400);
        return;
    }

    SystemConfiguration *syscfg = theSettings.systemConfiguration();


    String ssid = request->getParam("nw", true)->value();
    const char *pass = nullptr;
    if (request->hasParam("pass", true)) {
        pass = request->getParam("pass", true)->value().c_str();
    }
    if (request->hasParam("ip", true) && request->hasParam("gw", true) && request->hasParam(
            "nm", true)) {
        DBG_PRINTF("static IP\n");
        IPAddress ip = Local::scanIP(request->getParam("ip", true)->value().c_str());
        IPAddress gw = Local::scanIP(request->getParam("gw", true)->value().c_str());
        IPAddress nm = Local::scanIP(request->getParam("nm", true)->value().c_str());

        IPAddress dns = request->hasParam("dns", true)
                            ? Local::scanIP(request->getParam("dns", true)->value().c_str())
                            : IPAddress(0, 0, 0, 0);

        WiFiSetup.connect(ssid.c_str(), pass,
                          ip,
                          gw,
                          nm,
                          dns
        );
        // save to config
        syscfg->ip = ip;
        syscfg->gw = gw;
        syscfg->netmask = nm;
        theSettings.save();
    } else {
        WiFiSetup.connect(ssid.c_str(), pass);
        DBG_PRINTF("dynamic IP\n");
    }

    DBG_PRINTF("Saving WiFI credentials for SSID: %s\n", ssid.c_str());
    theSettings.setWiFiConfiguration(ssid.c_str(), pass);
    theSettings.save();

    request->send(201);
}


void bpl::webHandler::NetworkDataHandler::handleNetworkDisconnect(AsyncWebServerRequest *request)
{
    theSettings.systemConfiguration()->wifiMode = WIFI_AP;
    WiFiSetup.setMode(WIFI_AP);

    request->send(202);
}
