/* ************************************************************************ */
/*
    ParseIPReply.h
*/
#pragma once

#include <memory>
#include <WString.h>

class ipreply{
    public:
        String reply;
        String ip;
        int port;
};

class ParseIPReply {
    public:
        ParseIPReply(bool muted = false);

        //////////////////////////////////////////////////////////////////////
        /*
            This is one of the places where you would customize this class to  
            be used with your application specific configuration data.
            
            The other place is in ParseIPReply.cpp
        */
        ipreply parseReply(String buf);

    private:
        bool muteDebug;
        ipreply temp;

        void parseJSON(const char *);
};

