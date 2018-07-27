/* ************************************************************************ */
/*
   esp8266-ota.cpp - support for OTA updates

*/
// required include files...
#include <ArduinoOTA.h>
#include "esp8266-ino.h"
#include "OTACfgData.h"

#ifdef __cplusplus
extern "C" {
#endif

OTACfgData *o_cfgdat = NULL;
otacfg cfg;

unsigned long otaWaitUntil;

bool setupOTA(const String otaCfgFile)
{
String func = String(__func__);
bool bRet = false;

    // get the config data...
    o_cfgdat = new OTACfgData((const char *)otaCfgFile.c_str(), checkDebugMute());

    // check for errors
    if(!o_cfgdat->getError(errMsg)) 
    {
        // success, parse the JSON string
        o_cfgdat->parseFile();

        // check for errors
        if(o_cfgdat->getError(errMsg)) printError(func, errMsg);
        else 
        {
            // success
            o_cfgdat->getOTACfg(cfg);

            if(!checkDebugMute()) 
            {
                Serial.println("OTA port   - " + String(cfg.port));
                Serial.println("OTA host   - " + String(cfg.host));
                Serial.println("OTA passw  - " + String(cfg.passw));
                Serial.println("OTA otadur - " + String(cfg.otadur) + " = " + millisToTime(cfg.otadur));
                Serial.println();
                Serial.flush();
            }

            bRet = true;
        }
    } else printError(func, errMsg);

    // return the config-read status, true = success
    return bRet;
}

void setOTAOptions()
{
    if(cfg.port > 0)
    {
        ArduinoOTA.setPort(cfg.port);
    }

    if(cfg.host != "")
    {
        ArduinoOTA.setHostname((const char *)cfg.host.c_str());
    }

    if(cfg.passw != "")
    {
        ArduinoOTA.setPassword((const char *)cfg.passw.c_str());
    }
}

void initOTA()
{
    // check the WiFi connection and then read, parse, 
    // and save the config data
#ifdef CONFIG_DEMO
    if(connWiFi->IsConnected() && setupOTA("/otacfg.json"))
#else
    if(connWiFi->IsConnected() && setupOTA("/_otacfg.json"))
#endif
    {
        otaWaitUntil = millis() + cfg.otadur;

        setOTAOptions();

        ArduinoOTA.onStart([]() {
            if(!checkDebugMute()) Serial.println("\nOTA Start");
            sendStatus("OTA_START");
        });
    
        ArduinoOTA.onEnd([]() {
            if(!checkDebugMute()) Serial.println("\nOTA End");
            sendStatus("OTA_END");
        });
    
        // although this is shown in examples, it doesn't seem to ever
        // occur.
        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
            if(!checkDebugMute()) Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
        });
    
        ArduinoOTA.onError([](ota_error_t error) {

            if(!checkDebugMute()) {
                Serial.printf("OTA Error[%u]: ", error);
                if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
                else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
                else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
                else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
                else if (error == OTA_END_ERROR) Serial.println("End Failed");
            }

            String errcode = "I_DUNNO";

            switch(error)
            {
                case OTA_AUTH_ERROR:
                    errcode = "OTA_AUTH_ERROR";
                    break;

                case OTA_BEGIN_ERROR:
                    errcode = "OTA_BEGIN_ERROR";
                    break;

                case OTA_CONNECT_ERROR:
                    errcode = "OTA_CONNECT_ERROR";
                    break;

                case OTA_RECEIVE_ERROR:
                    errcode = "OTA_RECEIVE_ERROR";
                    break;

                case OTA_END_ERROR:
                    errcode = "OTA_END_ERROR";
                    break;
            }
            sendStatus(errcode);
        });
    
        ArduinoOTA.begin();
        if(!checkDebugMute()) Serial.println("OTA Ready - IP address: " + WiFi.localIP().toString());
        if(!checkDebugMute()) Serial.println("OTA Ready - Duration  : " + String(cfg.otadur));
        sendStatus("OTA_READY", String(cfg.otadur / 1000));

    } else Serial.println("OTA WiFi NOT Ready");
}

bool waitForOTA()
{
    if(connWiFi->IsConnected())
        if(otaWaitUntil != 0)
            if(otaWaitUntil > millis())
            {
                ArduinoOTA.handle();
                return true;
            } else {
                if(!checkDebugMute()) Serial.println("OTA Stopped Waiting");
                sendStatus("OTA_STOP");
                otaWaitUntil = 0;
            }

    return false;
}

#ifdef __cplusplus
}
#endif
