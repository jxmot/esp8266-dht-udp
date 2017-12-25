/* ************************************************************************ */
/*
   esp8266-ota.cpp - support for OTA updates

*/
// required include files...
#include <ArduinoOTA.h>
#include "esp8266-ino.h"

#ifdef __cplusplus
extern "C" {
#endif

unsigned long otaWaitUntil;

unsigned long _duration = 60000;

void initOTA()
{
    if(connWiFi->IsConnected())
    {
        otaWaitUntil = millis() + _duration;

        ArduinoOTA.onStart([]() {
            Serial.println("\nOTA Start");
            sendStatus("OTA_START");
        });
    
        ArduinoOTA.onEnd([]() {
            Serial.println("\nOTA End");
            sendStatus("OTA_END");
        });
    
        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
            Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
        });
    
        ArduinoOTA.onError([](ota_error_t error) {
            Serial.printf("OTA Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
            else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
            else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
            else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
            else if (error == OTA_END_ERROR) Serial.println("End Failed");

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
        Serial.println("OTA Ready - IP address: " + WiFi.localIP().toString());
        sendStatus("OTA_READY", String(_duration / 1000));

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
                Serial.println("OTA Stopped Waiting");
                sendStatus("OTA_STOP");
                otaWaitUntil = 0;
            }

    return false;
}

#ifdef __cplusplus
}
#endif
