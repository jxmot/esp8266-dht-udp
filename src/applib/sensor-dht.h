/* ************************************************************************ */
/*
    sensor-dht.h - support for DHTxx type temperature and humidity sensors.

    (c) 2017 Jim Motyl - https://github.com/jxmot/esp8266-dht-udp
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

// 
class sensornow {
    public:
        uint16_t seq = 0;
        float hnow   = 0.0;
        float tnow   = 0.0;
        float hlast  = 0.0;
        float tlast  = 0.0;
};

#ifdef __cplusplus
extern "C" {
#endif

extern void startSensor();
extern bool sendSensorData();
extern unsigned long getSensorInterval();
extern void readSensorNow(sensornow &);
extern bool sendSensorNow(sensornow);

#ifdef __cplusplus
}
#endif
