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
    Constructor
*/
SensorCfgData::SensorCfgData(const char *cfgfile, bool muted): ConfigData(cfgfile)
{
    muteDebug = muted;
}

//////////////////////////////////////////////////////////////////////////////
/*
    JSON Parser - this function handles all of the parsing into an object. It
    must be present in any class(es) derived from ConfigData. 
*/
void SensorCfgData::parseJSON(std::unique_ptr<char[]>& buf)
{
    if(!muteDebug)
    {
        Serial.println();
        Serial.println("SensorCfgData parsing JSON - ");
        Serial.println(buf.get());
    }
 
    // IMPORTANT : Changes made to the data file should be passed
    // through the following utility and the following lines must
    // be modified accordingly -
    //
    //      https://arduinojson.org/assistant/
    const size_t bufferSize = JSON_OBJECT_SIZE(8) + 95;
    StaticJsonBuffer<bufferSize> jsonBuffer;

    JsonObject& json = jsonBuffer.parseObject(buf.get());

    /*
        This is one of the places where you would customize this class to be 
        used with your application specific configuration data.
        
        Another place is in SensorCfgData.h
    */
    sensorcfg.type = String((const char *)json["type"]);
    sensorcfg.pin = String((const char *)json["pin"]);
    sensorcfg.scale = String((const char *)json["scale"]);
    sensorcfg.interval = json["interval"];
    sensorcfg.error_interval = json["error_interval"];
    sensorcfg.report = String((const char *)json["report"]);
    sensorcfg.delta_t = json["delta_t"];
    sensorcfg.delta_h = json["delta_h"];
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
