/* ************************************************************************ */
/*
    MimicCfgData.h - Intended to provide the ability to force this device
    to report its statuses and data using a different ID. This is useful 
    when replacing the ESP-01 in situations where status and data are
    being saved and introducing a new ID isn't convenient.
*/
#pragma once

// Configuration Object base class
#include "ConfigData.h"

#define MAX_MIMICS 5

class MimicCfgData : public ConfigData {

    public:
        MimicCfgData(const char *cfgfile, bool muted = false);

    private:
        void parseJSON(std::unique_ptr<char[]>& buf) override;

    //////////////////////////////////////////////////////////////////////////
    /*
        This is one of the places where you would customize this class to be 
        used with your application specific configuration data.
        
        The other place is in MimicCfgData.cpp
    */
    public:
        bool isMimic(String &mimic);

    private:
        String error;
        bool muteDebug;

        String mimic;
};

