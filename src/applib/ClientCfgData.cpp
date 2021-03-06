/* ************************************************************************ */
/*
    ClientfgData.h - Intended to provide external server configuration data,
    such as - address and port. 

    (c) 2017 Jim Motyl - https://github.com/jxmot/esp8266-dht-udp
*/
#include "ClientCfgData.h"
#include <ArduinoJson.h>


// these are known, but not referenced in the client application
String ClientCfgData::labels[] = {"udp1","udp2","END"};

//////////////////////////////////////////////////////////////////////////////
/*
*/
ClientCfgData::ClientCfgData(const char *cfgfile, bool muted): ConfigData(cfgfile)
{
    muteDebug = muted;

    for(int ix = 0; ix < MAX_SRVCFG; ix++)
    {
        configs[ix] = NULL;
    } 
}

//////////////////////////////////////////////////////////////////////////////
/*
*/
void ClientCfgData::parseJSON(std::unique_ptr<char[]>& buf)
{
    if(!muteDebug)
    {
        Serial.println();
        Serial.println("ClientCfgData parsing JSON - ");
        Serial.println(buf.get());
    }
 
    // IMPORTANT : Changes made to the data file should be passed
    // through the following utility and the following lines must
    // be modified accordingly -
    //
    //      https://arduinojson.org/assistant/
    const size_t bufferSize = (3 * JSON_OBJECT_SIZE(2)) + 68;
    StaticJsonBuffer<bufferSize> jsonBuffer;

    JsonObject& json = jsonBuffer.parseObject(buf.get());

    /*
        This is one of the places where you would customize this class to be 
        used with your application specific configuration data.
        
        Another place is in ClientCfgData.h
    */
    for(int ix = 0; labels[ix] != "END"; ix++)
    {
        configs[ix] = new clisrvcfg();
        configs[ix]->label    = labels[ix];
        configs[ix]->addr     = String((const char *)json[(char *)labels[ix].c_str()]["addr"]);
        configs[ix]->ipaddr.fromString(configs[ix]->addr);
        configs[ix]->port     = json[(char *)labels[ix].c_str()]["port"];
    }
}

//////////////////////////////////////////////////////////////////////////////
/*
    This is one of the places where you would customize this class to be 
    used with your application specific configuration data.
    
    Another place is in ClientCfgData.h
*/
bool ClientCfgData::getServer(String label, clisrvcfg &cfgout)
{
bool bRet = false;

    for(int ix = 0; (ix < MAX_SRVCFG) && !bRet; ix++)
    {
        if(configs[ix]->label == label)
        {
            cfgout.label  = configs[ix]->label;
            cfgout.addr   = configs[ix]->addr;
            cfgout.ipaddr = configs[ix]->ipaddr;
            cfgout.port   = configs[ix]->port;
            bRet = true;
        }
    }
    return bRet;
}


