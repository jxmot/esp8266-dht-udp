/* ************************************************************************ */
/*
    SensorCfgData.h - Intended to provide configuration data for the 
    attached sensor.

    For now the choices are DHT22 and DHT11. More will be added later as this
    code evolves.
*/
#pragma once

#include "ConfigData.h"

// Sensor Configuration 
//
// NOTE: Don't change the values here, these values are commentary
// showing what can be contained in the configuration data file.
class sensorconfig {
    public:
        String type = "DHT22 or DHT11 goes here";
        String pin = "D6 or D4(ESP-01 will be GP2)";
        String scale = "F or C";
        // interval between sensor readings in milliseconds
        unsigned long interval = 5000;
        // retry interval when the sensor returns a NaN or
        // some other error
        unsigned long error_interval = 5000;
        String report = "CHG or ALL";
        // divided by 10 gives the amount of change in temp
        // or humidity needed before reporting
        int delta_t = 1;
        int delta_h = 1;
};

// Sensor Configuration File Reader/Parser
class SensorCfgData : public ConfigData {

    public:
        SensorCfgData(const char *cfgfile, bool muted = false);

    private:
        void parseJSON(std::unique_ptr<char[]>& buf) override;

    //////////////////////////////////////////////////////////////////////////
    /*
        This is one of the places where you would customize this class to be 
        used with your application specific configuration data.
        
        The other place is in ClientCfgData.cpp
    */
    public:
        bool getSensor(sensorconfig &cfgout);

    private:
        bool muteDebug;
        sensorconfig sensorcfg;
};

