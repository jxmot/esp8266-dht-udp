/* ************************************************************************ */
/*
    OTACfgData.h - Provides OTA configuration data
*/
#include "OTACfgData.h"
#include <ArduinoJson.h>

//////////////////////////////////////////////////////////////////////////////
/*
    Constructor
*/
OTACfgData::OTACfgData(const char *cfgfile, bool muted): ConfigData(cfgfile)
{
    muteDebug = muted;
}

//////////////////////////////////////////////////////////////////////////////
/*
    Parse the JSON data that is specific to this configuration object.
*/
void OTACfgData::parseJSON(std::unique_ptr<char[]>& buf)
{
    if(!muteDebug)
    {
        Serial.println();
        Serial.println("OTACfgData parsing JSON - ");
        Serial.println(buf.get());
        Serial.println();
        Serial.flush();
    }
 
    // For getting the size correct, use the following to calculate
    // how much is requried for a given bit of JSON data - 
    //
    //      https://bblanchon.github.io/ArduinoJson/assistant/
    const size_t bufferSize = JSON_OBJECT_SIZE(4) + 33;
    StaticJsonBuffer<bufferSize> jsonBuffer;

    JsonObject& json = jsonBuffer.parseObject(buf.get());

    /*
        This is one of the places where you would customize this class to be 
        used with your application specific configuration data.
        
        Another place is in OTACfgData.h
    */
    config.port   = json["port"];
    config.host   = String((const char *)json["host"]);
    config.passw  = String((const char *)json["passw"]);
    config.otadur = json["otadur"];
}

void OTACfgData::getOTACfg(otacfg &cfg)
{
    cfg = config;
}

