/* ************************************************************************ */
/*
    sensor-dht.h - support for DHTxx type temperature and humidity sensors.

    To Do: see the cpp file
*/
#pragma once

#include "../adafruit/DHT.h"

// 
class livesensor {
    public:
        uint16_t seq = 0;
        float t = 0.0;
        float h = 0.0;
        unsigned long nextup = 0;
        int16_t nancount = 0;
        int16_t errcount = 0;
};

#ifdef __cplusplus
extern "C" {
#endif

extern void startSensor();
extern bool sendSensorData();

#ifdef __cplusplus
}
#endif
