/* ************************************************************************ */
/*
    SensorCfgData.cpp - Intended to provide configuration data for the 
    attached sensor.

    For now the choices are DHT22 and DHT11. More will be added later as this
    code evolves.
*/
#include "SensorCfgData.h"
#include <ArduinoJson.h>

//////////////////////////////////////////////////////////////////////////////
/*
*/
SensorCfgData::SensorCfgData(const char *cfgfile, bool muted): ConfigData(cfgfile)
{
    muteDebug = muted;
}

//////////////////////////////////////////////////////////////////////////////
/*
*/
void SensorCfgData::parseJSON(std::unique_ptr<char[]>& buf)
{
    if(!muteDebug)
    {
        Serial.println();
        Serial.println("SensorCfgData parsing JSON - ");
        Serial.println(buf.get());
    }
 
    // https://bblanchon.github.io/ArduinoJson/assistant/
    const size_t bufferSize = JSON_OBJECT_SIZE(7) + 68;
    StaticJsonBuffer<bufferSize> jsonBuffer;

    JsonObject& json = jsonBuffer.parseObject(buf.get());

    /*
        This is one of the places where you would customize this class to be 
        used with your application specific configuration data.
        
        Another place is in SensorCfgData.h
    */
    sensorcfg.type = String((const char *)json["type"]);
    sensorcfg.pin = String((const char *)json["pin"]);
    sensorcfg.unit = String((const char *)json["unit"]);
    sensorcfg.interval = json["interval"];
    sensorcfg.report = String((const char *)json["report"]);
    sensorcfg.delta = json["delta"];
    sensorcfg.dest = String((const char *)json["dest"]);
}

//////////////////////////////////////////////////////////////////////////////
/*
    This is one of the places where you would customize this class to be 
    used with your application specific configuration data.
    
    Another place is in SensorCfgData.h
*/
bool SensorCfgData::getSensor(sensorconfig &cfgout)
{
bool bRet = false;

    cfgout = sensorcfg;
    return true;
}
