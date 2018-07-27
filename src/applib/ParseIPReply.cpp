/* ************************************************************************ */
/*
    ParseIPReply.cpp
*/
#include "ParseIPReply.h"

#include <ArduinoJson.h>

//////////////////////////////////////////////////////////////////////////////
/*
    Constructor
*/
ParseIPReply::ParseIPReply(bool muted)
{
    muteDebug = muted;
}

ipreply ParseIPReply::parseReply(String buf)
{
    parseJSON(buf.c_str());
    return temp;
}

/*
    JSON Parser - this function handles all of the parsing into an object.
*/
//void ParseIPReply::parseJSON(std::unique_ptr<char[]>& buf)
void ParseIPReply::parseJSON(const char *buf)
{
    if(!muteDebug)
    {
        Serial.println();
        Serial.println("ParseIPReply parsing JSON - ");
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
    //      {"reply":"IP_ADDR","ip":"192.168.0.7","port":48431}
    //
    const size_t bufferSize = JSON_OBJECT_SIZE(3) + 96;
    StaticJsonBuffer<bufferSize> jsonBuffer;
    
    JsonObject& json = jsonBuffer.parseObject(buf);

    temp.reply = String((const char *)json["reply"]);
    temp.ip = String((const char *)json["ip"]);
    temp.port = json["port"];
}


