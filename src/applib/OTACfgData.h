/* ************************************************************************ */
/*
    OTACfgData.h - Provides OTA configuration data
*/
#pragma once

// Configuration Object base class
#include "ConfigData.h"

class otacfg {
    public:
        int port = 0;
        String host = "";
        String passw = "";
        unsigned long otadur = 60000;
};


class OTACfgData : public ConfigData {

    public:
        OTACfgData(const char *cfgfile, bool muted);

    private:
        void parseJSON(std::unique_ptr<char[]>& buf) override;

    //////////////////////////////////////////////////////////////////////////
    /*
        This is one of the places where you would customize this class to be 
        used with your application specific configuration data.
        
        The other place is in OTACfgData.cpp
    */
    public:
        void getOTACfg(otacfg &cfg);

    private:
        bool muteDebug;
        otacfg config;

};

