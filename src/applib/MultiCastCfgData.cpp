/* ************************************************************************ */
/*
    MultiCastCfgData.cpp - Provides the port and any additional info needed to
    send a UDP multi-cast. 
*/
#include "MultiCastCfgData.h"
#include <ArduinoJson.h>

//////////////////////////////////////////////////////////////////////////////
/*
*/
MultiCastCfgData::MultiCastCfgData(const char *cfgfile, bool muted): ConfigData(cfgfile)
{
    muteDebug = muted;
}

/*
*/
void MultiCastCfgData::parseJSON(std::unique_ptr<char[]>& buf)
{
    if(!muteDebug)
    {
        Serial.println();
        Serial.println("MultiCastCfgData parsing JSON - ");
        Serial.println(buf.get());
        Serial.println();
        Serial.flush();
    }
 
    // https://bblanchon.github.io/ArduinoJson/assistant/
    // {"addr":"224.0.0.1","port":54321}
    const size_t bufferSize = JSON_OBJECT_SIZE(2) + 30;
    StaticJsonBuffer<bufferSize> jsonBuffer;

    JsonObject& json = jsonBuffer.parseObject(buf.get());

    config.addr = String((const char *)json["addr"]);
    config.ipaddr.fromString(config.addr);
    config.port = json["port"];
}

//////////////////////////////////////////////////////////////////////////////
/*
    This is one of the places where you would customize this class to be 
    used with your application specific configuration data.
    
    Another place is in MultiCastCfgData.h
*/
bool MultiCastCfgData::getCfg(mcastcfg &cfgout)
{
String emsg;
bool bRet = false;

    if(!getError(emsg))
    {
        cfgout.addr   = config.addr;
        cfgout.ipaddr = config.ipaddr;
        cfgout.port   = config.port;
        bRet = true;
    }
    return bRet;
}


