/* ************************************************************************ */
/*
    esp8266-ino.h - support for the associated ino file.

    Stuff that's useful for multiple sketches and helps in keeping the 
    sketch file(s) uncluttered.

    CHANGES:
        moved LED toggle interval defines to this file - update the
        ESP8266-udp repo!

*/
#pragma once

#include "src/applib/AppCfgData.h"
#include "src/applib/WifiCfgData.h"
#include "src/applib/SrvCfgData.h"
#include "src/applib/ClientCfgData.h"
#include "src/applib/MultiCastCfgData.h"
#include "src/applib/SensorCfgData.h"
#include "src/applib/connectWiFi.h"

#include "esp8266-udp.h"

// The on-board LED is used for indicating the post-setup state. The LED 
// will be toggled using one of two intervals (OFF/ON). The intent is to
// use a slower toggle rate to indicate success, and a faster one to 
// indicate an error during setup.
#define TOGGLE_INTERVAL 1000
#define ERR_TOGGLE_INTERVAL 250

#ifdef __cplusplus
extern "C" {
#endif

// the default interval for the on-board LED on/off
extern int toggInterv;

// support functions - 
extern void setupStart();
extern void setupConfig();
extern void setupInit();
extern void setupDone();

extern bool setupApp(const String appCfgFile);
extern bool setupWiFi(const String wifiCfgFile);
extern bool setupServers(const String srvCfgFile);
extern bool setupClient(const String clientCfgFile);
extern bool setupMultiCast(const String mcastCfgFile);
extern bool setupSensor(const String sensorCfgFile);

extern bool toggleLED();

extern void printError(String func, String _errMsg);

extern bool checkDebugMute();

extern void ready();
extern void sendStatus(String status, String msg = "");

extern int handleComm();
//extern void stopDisplay();
//extern void handleDisplay();

// pointers to configuration data objects - 
extern AppCfgData *a_cfgdat;
extern WifiCfgData *w_cfgdat;
extern SrvCfgData *s_cfgdat;
extern ClientCfgData *c_cfgdat;
extern MultiCastCfgData *m_cfgdat;
extern SensorCfgData *sens_cfgdat;

// pointer to the WiFi connection object -
extern ConnectWiFi *connWiFi;

#ifdef __cplusplus
}
#endif
