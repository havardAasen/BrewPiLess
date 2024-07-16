# BrewPiLess
 **Note: ALL BPL settings will be gone after upgrading to v3.0**

 **Note: re-SETUP is necessary after upgrading to v2.4**

## Features
 * I2C LCD support.
 * Rotary encoder support (* not supported by default).
 * Remote LCD display on browser.
 * Remote temperature control.
 * Temperature schedule.
 * Device(temperature sensor and actuator) setup.
 * Temperature logging to specified **remote** destination. 
 * Web-based OTA firmware update.
 * Web-based network setting.
 * SoftAP mode.
 * Local temperature log and temperature chart.
 * iSpindel support. 
 * Gravity logging. The gravity data can be manually input or from iSpindel.
 * Gravity-based temperature schedule.
 * Save and resuse of beer profiles.
 * Static IP setting.
 * Export saved data to csv format by offline log viewer.
 * Brew and calibrate iSpindel. **new!**
   
# Introduction
This project uses a single ESP8266 to replace RPI and Arduino.
![Main Screen](img/bplmain.jpg)
BrewPi is the greatest, if not the ultimate, fermentation temperature controller. The original design uses a RPI to log temperatures and maintain a temperature schedule. The RPI also hosts a web server for the browser-based front-end. 
Using a RPI or a PC allows the maximum power of BrewPi to be used but requires additional hardware (namely a RPI or PC). 

ESP8266 is cheap and powerful WiFi-enabling IOT solution. 
Although it isn't as powerful as a RPI, it's a good solution to maximize the functionality and minimize the cost. Using a single ESP8266 as the temperature controller (replacing Arduino) and as the web server and schedule maintainer (replacing RPI) also reduces the work in building a brewpi system.

## !!Special Note
You will need to run the hardware setup procedure after upgrading to v2.4 from prior versions other than Glycol option enabled. Take a note of your configuration or save the options before you update the firmware so that you can recover the settings quickly.
 
## Known issues
* ESP8266 won't restart after saving system configuration.
 Sometimes ESP8266 can't restart after a software watchdog timer reset, which is the only way to reset the system by software. It happened on my NodeMcu and D1 mini boards that didn't connect to anything but USB. I have no solution for it.
* ESP8266 won't start after selecting WiFi network.
 The web server used is ESPAsyncWebServer which uses ESPAsyncTCP. I found that if ESP8266 ever enters SoftAP mode before connecting to the network, the Web server will fail on tcp_bind() and the web service will be unavailable. Not tracing the source code of the LWIP, I just worked around by reseting the system. However, ESP8266 sometimes doesn't reset.
* The page can't be loaded correctly.
 It rarely happens after HTTP caching is used, but it does happen especially when Javascript Console is opened. During developing and testing, I found corrupted html/javascript pages. Without the abliity and time to debug or develop the web server and or TCP/IP stack, I decide to live with it.
* Incorrect temperature chart.
 The log format before v2.0 is vulnerable. There seems to be some unconsidered conditions that break the log. 

# Software configuration

If you want to build the BrewPiLess firmware by yourself, we strongly recommend using platformIO and VisualStudio Code with the PlatformIO IDE extension.

You can find further details in the wiki: [Software Installation](https://github.com/vitotai/BrewPiLess/wiki/Software-Installation).

# wiki

* [Hardware Setup](https://github.com/vitotai/BrewPiLess/wiki/Hardware-Setup)
  * [Example#1](https://github.com/vitotai/BrewPiLess/wiki/Hardware-Setup-example)
  * [SONOFF](https://github.com/vitotai/BrewPiLess/wiki/SONOFF)
  * [Thorrax’s Board](https://github.com/thorrak/brewpi-esp8266)
* [Software Installation](https://github.com/vitotai/BrewPiLess/wiki/Software-Installation)
  * [Software Configuration](https://github.com/vitotai/BrewPiLess/wiki/Software-Configuration)
* [Initial WiFi Setup](https://github.com/vitotai/BrewPiLess/wiki/Initial-WiFi-Setup)
* [System Setup](https://github.com/vitotai/BrewPiLess/wiki/System-Setup)
  * [SoftAP mode](https://github.com/vitotai/BrewPiLess/wiki/SoftAP-mode)
* [Device Setup](https://github.com/vitotai/BrewPiLess/wiki/BrewPi-Device-Setup)
* [Temperature Logging](https://github.com/vitotai/BrewPiLess/wiki/Temperature-logging,-locally)
* [Cloud Logging](https://github.com/vitotai/BrewPiLess/wiki/Log-data-to-clouds)
  * [Log to Google Spreadsheet](https://github.com/vitotai/BrewPiLess/wiki/Log-data-to-Google-Spreadsheet)
* [Beer Profile](https://github.com/vitotai/BrewPiLess/wiki/Beer-Profile)
* [iSpindel Support](https://github.com/vitotai/BrewPiLess/wiki/iSpindel-Support)
  * [Brew and Calibrate](https://github.com/vitotai/BrewPiLess/wiki/Brew-and-Calibrate-iSpindel)
  * [iSpindel as Beer Sensor](https://github.com/vitotai/BrewPiLess/wiki/Using-iSpindel-as-Beer-Temperature-Sensor)
* [Manual Gravity Logging](https://github.com/vitotai/BrewPiLess/wiki/Manual-Gravity-Logging)
* [Use with Glycol](https://github.com/vitotai/BrewPiLess/wiki/Use-with-Glycol)
* [Other URLs](https://github.com/vitotai/BrewPiLess/wiki/Other-URLs)
  * Clear WiFi setting
  * Format file system
  * OTA update
* [Overwrite pages](https://github.com/vitotai/BrewPiLess/wiki/Overwrite-web-pages)
* [JSON Commands](https://github.com/vitotai/BrewPiLess/wiki/JSON-command)
  * Temperature Unit
  * Sensor Calibration
* [FAQ](https://github.com/vitotai/BrewPiLess/wiki/FAQ)
