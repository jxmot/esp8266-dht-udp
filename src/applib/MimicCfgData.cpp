/* ************************************************************************ */
/*
    MimicCfgData.cpp - Provides the port and any additional info needed to
    send a UDP multi-cast. 
*/
#include "MimicCfgData.h"
#include <ArduinoJson.h>

#define DEVID_REAL 0
#define DEVID_MIMIC 1

//////////////////////////////////////////////////////////////////////////////
/*
    Constructor
*/
MimicCfgData::MimicCfgData(const char *cfgfile, bool muted): ConfigData(cfgfile)
{
    muteDebug = muted;
}

/*
    JSON Parser - this function handles all of the parsing into an object. It
    must be present in any class(es) derived from ConfigData. 
*/
void MimicCfgData::parseJSON(std::unique_ptr<char[]>& buf)
{
    if(int err = getError(error) >= 0)
    {
        if(!muteDebug)
        {
            Serial.println();
            Serial.println("MimicCfgData parsing JSON - ");
            Serial.println(buf.get());
            Serial.println();
            Serial.flush();
        }
    
        // IMPORTANT : Changes made to the data file should be passed
        // through the following utility and the following lines must
        // be modified accordingly -
        //
        //      https://arduinojson.org/assistant/
        //
        // The data appears as - 
        //      
        //      {
        //      "mimic": "ESP_49ECCD"
        //      }
        //
        const size_t bufferSize = JSON_OBJECT_SIZE(1) + 20;
        StaticJsonBuffer<bufferSize> jsonBuffer;
    
        JsonObject& json = jsonBuffer.parseObject(buf.get());
    
        /*
            This is one of the places where you would customize this class to be 
            used with your application specific configuration data.
            
            The other place is in MimicCfgData.h
        */
        mimic = String((const char *)json["mimic"]);
    } else mimic = String("ESP_BEEFEE");
}

bool MimicCfgData::isMimic(String &_mimic) 
{
bool bRet = false;
int err;

    parseFile();

    if((err = getError(error)) >= 0) 
    {
        bRet = true;
    } else {
        if(!muteDebug)
        {
            Serial.println();
            Serial.print("MimicCfgData Error = ");
            Serial.println(error);
            Serial.println();
            Serial.flush();
        }
    }
    _mimic = mimic;
    return bRet;
}


