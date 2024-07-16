# Changelog

## Unreleased


## v3.6

* update framework to 2.2.0
* **4m2m flash layout for All but SONOFF, due to size growth of framework.**
* update OLED library to 4.0 (Not verified by me, but SOMEONE@HBT did report working)
* update to ArduinoJson V6
* add revised LCD page. at /lcd
* SOFF OTA configuraton not longer available for space limit
* Using interrupt for more responsive button operation.
* MQTT publish/subscribe, NEW UI only.

## 3.5.1

* fixed iSpindel temperature unit issue
* update new OLED library(not verified)
* The version number is still "3.5"

## 3.5

* MQTT Remote Control
* Pressure transducer support
* Add iSpindel WiFi signal if available, "Now" button in profile editing
* "Fixed" incorrect time issue after restart.
* Add DNS setting for static IP

## 3.4

* Add back STATAION mode only.
* fixing Cap control tab display bug.
* fixing blocking "Communication to BPL" dialog under AP mode

## 3.3

* LCD information push from server instead of request/response.
* Replace ServerSideEvent by WebSocket.
* add Brazil Portugues support.

## 3.2

* Revise remote logging settings. Simpler interface and special process for ubidots.com.
* Update to Arduino/ESP8266 framework 1.8.0

## 3.1

* Fix beer profile which uses gravity as conditions
* Ditch WiFiManager.
* AP mode is always avaible.
* Plato supported
* Spanish supported
* Tom's frontend embedded.

## 3.0

* Avoid frequent access access of file system
* Revise settings. Merge all settings into one file.
* Applying temperature corection of iSpindel calibration informatoin.
* Selective iSpindel calibration points.
* Brew&Cal option moved to start of logs.
* Merge classic HTML/JS files into grunt
* Using push for beer profile information to reduce additional connection establishment.
* bug fixes.

## 2.7

* Cap(spunding) control
* revise parasite temperature control

## 2.6

* fix the bug that don't save Gravity Device settings
* write formula back to ESP8266 in brew and calibrate  mode
* update temperature correction formula in brew and calibrate mode

## 2.5.1

* LCD backlight timer setting
* support 2 buttons
* merge Tom's front-end

## 2.5 - 2018-01-18

* optional: Latest ESP8266/Arduino framework w/ ESPAsyncTcp & ESPAsyncWebServer
* revise network configuration and system config; change setting at "Config" page
* fixed(static) IP bug fixed
* fix redundant data requests
* revise log resumption

## 2.4.2x - 2018-01-15

* Fix TILT zero display

## 2.4.2 - 2017-12-27
* bug fixed for resume display

## 2.4.1 - 2017-11-28

* URL to Format File System
* missing "Calculated by BPL" in v2.4

## 2.4 - 2017-11-09

* Brew and calibrate iSpindel.
* Use iSpindel temperature reading as Beer Sensor.
* Display tilt value of iSpindel.
* Enhance SSE re-establishment
* Default configurable minimum cooling/heating time & back-up sensor. (That is, Glycol supported.)
* HTTP Port settings.

## 2.3.3 - 2017-10-08

* All HTML files can be replaced by files on SPIFFS. Gzip support.
* updated HTML/JS
* Add "Title" to be displayed at banner in config page.
* Workaround for accepting HTTP Post body length not equal to Content-Length.( for iSpindel v5.2+)

## 2.3.2

* Beer Profile scheculde bug fix
* Show hostname at banner

## 2.3.1

* WiFi signal
* /getstatus web service

## 2.3

* Fix error in time of reset. (New log format! Use new log viewer)
* State coloring in chart
* Remove "view" action in log list.

## 2.2

* 4 decimals of gravity
* Switch to PlatformIO instead of Arduino IDE.

## 2.1

* more gravity-based condition

## 2.0

* gravity-based beer profil schedule

## 1.2.7

* iSpindel support
