/* ************************************************************************ */
/*
    ClientfgData.h - Intended to provide external server configuration data,
    such as - address and port. 
*/
#pragma once

#include <IPAddress.h>

#include "ConfigData.h"

// the description of a server
class clisrvcfg {
    public:
        String label;
        String addr;
        IPAddress ipaddr;
        int port;
};

// to limit memory use only 4 servers can be configured
#define MAX_SRVRS  4
// the last slot is a "end of list" marker.
#define MAX_SRVCFG  MAX_SRVRS + 1

class ClientCfgData : public ConfigData {

    public:
        ClientCfgData(const char *cfgfile, bool muted = false);

    private:
        void parseJSON(std::unique_ptr<char[]>& buf) override;

    //////////////////////////////////////////////////////////////////////////
    /*
        This is one of the places where you would customize this class to be 
        used with your application specific configuration data.
        
        The other place is in ClientCfgData.cpp
    */
    public:
        bool getServer(String label, clisrvcfg &cfgout);

    private:
        bool muteDebug;
        static String labels[];
        clisrvcfg *configs[MAX_SRVCFG];
};

