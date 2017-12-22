/* ************************************************************************ */
/*
    sensor-dht.h - support for DHTxx type temperature and humidity sensors.
*/
#pragma once

#include "src/adafruit/DHT.h"

class livesensor {
    public:
        uint16_t seq = 0;
        float t = 0.0;
        float h = 0.0;
        
        
};

extern void startSensor();

