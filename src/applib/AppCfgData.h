/* ************************************************************************ */
/*
    AppCfgData.h - Intended to provide application configuration data, this
    could be things such as - 

        1) Application name
        2) Feature control flags. A "debug mute" is implemented here.
        3) 

*/
#pragma once

// Configuration Object base class
#include "ConfigData.h"

class AppCfgData : public ConfigData {

    public:
        AppCfgData(const char *cfgfile);

    private:
        void parseJSON(std::unique_ptr<char[]>& buf) override;

    //////////////////////////////////////////////////////////////////////////
    /*
        This is one of the places where you would customize this class to be 
        used with your application specific configuration data.
        
        The other place is in AppCfgData.cpp
    */
    public:
        String getAppName();
        bool getDebugMute();
        bool setDebugMute(bool _debugmute);

        String getWifiConfig();
        String getClientConfig();
        String getMcastConfig();
        String getSensorConfig();

    private:
        String appname;
        bool debugmute;

        String wificonfig;
        String clientconfig;
        String mcastconfig;
        String sensorconfig;
};

