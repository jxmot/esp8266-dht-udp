/* ************************************************************************ */
/*
    sensor-dht.cpp - support for DHTxx type temperature and humidity sensors.

    (c) 2017 Jim Motyl - https://github.com/jxmot/esp8266-dht-udp
*/
// required include files...
#include "esp8266-ino.h"
#include "esp8266-udp.h"
#include "sensor-dht.h"

#ifdef __cplusplus
extern "C" {
#endif

// max number of NaN results from the sensor in succession 
// before reporting an error
#define MAX_NAN 5

// Initialize the temperature/humidity sensor
// NOTE: The DHT class has been modified from its original.
DHT dht;

// sensor config data
sensorconfig scfg;
// current and previous sensor readings
livesensor sensor;
livesensor sensorlast;

/*
    Get fresh data from the sensor and save it in the `sensor`
    object. Also check it for "is a NaN" and id it is then
    return `false` and let the caller decide the next step.
*/
bool updateSensorData() 
{
bool bRet = true;

    // read values from the sensor
    sensor.h = dht.readHumidity();
    sensor.t = dht.readTemperature(!(scfg.scale == "F" ? false : true));

    if(isnan(sensor.t) || isnan(sensor.h))
    {
        sensor.nancount += 1;

        if(!checkDebugMute()) sendStatus("SENSOR_FAULT", "NaN " + String(isnan(sensor.t)) + "  "  + String(isnan(sensor.h)));
        sendStatus("SENSOR_FAULT", "NaN " + String(sensor.nancount));

        if(!checkDebugMute()) Serial.println("updateSensorData() - nancount = " + String(sensor.nancount));

        // if/when we get a good data reading this will make
        // sure that chkReport() will return 'true'
        sensor.h = sensorlast.h = 0;
        sensor.t = sensorlast.t = 0;

        bRet = false;

        if(sensor.nancount >= MAX_NAN) 
        {
            sendStatus("SENSOR_ERROR", "Too many NaN readings from sensor");
            sensor.nancount = 0;
            sensor.errcount += 1;
        }

    } else {
        // if any previous readings were NaN then announce
        // that we've recovered and have good data
        if(sensor.errcount > 0)
        {
            if(!checkDebugMute()) sendStatus("SENSOR_RECOVER", "Recovered after NaN from sensor - errcount = " + String(sensor.errcount));
            sendStatus("SENSOR_RECOVER", "nancount = " + String(sensor.nancount));
            sensor.errcount = 0;
            sensor.nancount = 0;
        }
        // each time we successfully update the data from the sensor increment 
        // the sequence number. this will assist in determining data updates vs
        // data reports.
        sensor.seq += 1;
        if(!checkDebugMute()) Serial.println("updateSensorData() - " + String(sensor.seq) + "   " + String(sensor.t) + "  " + String(sensor.h));
    }
    return bRet;
}

void readSensorNow(sensornow &_sensor)
{
    // read values from the sensor
    _sensor.hnow = dht.readHumidity();
    _sensor.tnow = dht.readTemperature(!(scfg.scale == "F" ? false : true));
    if(isnan(_sensor.hnow) || isnan(_sensor.tnow))
        _sensor.hnow = _sensor.tnow = 0;

    // also provide the last readings and seq #
    _sensor.hlast = sensorlast.h;
    _sensor.tlast = sensorlast.t;
    _sensor.seq   = sensorlast.seq = (sensor.seq += 1);
}

bool sendSensorNow(sensornow _sensor)
{
bool bRet = false;
conninfo conn;
String sensorData;

    // if the WiFi is connected...
    if(connWiFi->GetConnInfo(&conn))
    {
        // construct the JSON string with our data inside...
        //
        // example : {"dev_id":"ESP_290767","seq":1,"t":71.5,"h":37.40}
        sensorData = "{\"dev_id\":\"" + conn.hostname + "\"";
        sensorData = sensorData + ",\"seq\":" + String(_sensor.seq);
        sensorData = sensorData + ",\"t\":" + String(_sensor.tnow) + ",\"h\":" + String(_sensor.hnow);
        sensorData = sensorData + ",\"last\":{\"t\":"+ String(_sensor.tlast) + ",\"h\":" + String(_sensor.hlast) +"}";
        sensorData = sensorData + "}";

        int sent = sendUDP((char *)sensorData.c_str(), strlen(sensorData.c_str()));
        if(sent > 0) bRet = true;
    }
    return bRet;
}

/*
    Check the configured reporting type and decide if the data should
    be reported (sent via UDP)
*/
bool chkReport()
{
bool bRet = false;

    // report ALL values as they are read
    if(scfg.report == "ALL") 
    {
        // save the last reading 
        sensorlast = sensor;
        bRet = true;
    }
    else
    {
        float f_deltaT = ((float)(scfg.delta_t) / 10);
        float f_deltaH = ((float)(scfg.delta_h) / 10);

        // report only if a change was detected...
        if(scfg.report == "CHG")
        {
            // calculate the amount of change (if any)
            // NOTE: Small incremental (below delta
            // threshold) will not be sent. Then it's 
            // possible for a sensor to appear "frozen"
            // and not sending any updates. However a 
            // small fix by moving "sensorlast = sensor;"
            // to when the data is actually sent.
            float t_diff = abs(sensor.t - sensorlast.t);
            float h_diff = abs(sensor.h - sensorlast.h);

            // Using the configured delta value determine if the
            // temperature or humidity have changed enough to be
            // reported. The delta is stored as a integer that
            // represents the number of "tenths" of change that
            // must occur to allow the values to be reported.
            if((t_diff > f_deltaT) || (h_diff > f_deltaH)) bRet = true;

            if(!checkDebugMute())
            {
                Serial.println("deltaT = " + String(f_deltaT) + "    deltaH = " + String(f_deltaH));
                Serial.println("t_diff = " + String(t_diff) + "    h_diff = " + String(h_diff));
            }

            // save the last reading 
            // NOTE: removal should fix frozen sensor, issue #11
            //sensorlast = sensor;

        } else bRet = true;
    }
    return bRet;
}

/*
    Send the current sensor data to the server if the specified interval
    has elapsed.
*/
bool sendSensorData()
{
bool bRet = false;
conninfo conn;
String sensorData;

    // Is this sensor up next for a reading?
    if(sensor.nextup < millis())
    {
        // update the sensor data, if an error occurred then 
        // change the interval between retries... success?
        if(updateSensorData()) 
        {
            // success!
            sensor.nextup = scfg.interval + millis();

            if(!checkDebugMute())
            {
                Serial.println("last - " + String(sensorlast.t) + "  " + String(sensorlast.h));
                Serial.println("live - " + String(sensor.t) + "  " + String(sensor.h));
            }
    
            // if the WiFi is connected and we're supposed to report the values...
            if(connWiFi->GetConnInfo(&conn) && chkReport())
            {
                // construct the JSON string with our data inside...
                //
                // example : {"dev_id":"ESP_290767","t":71.5,"h":37.40,"scale":"F"}
                sensorData = "{\"dev_id\":\"" + conn.hostname + "\"";
                // 'app_id' currently not used, removed from sensor data.
                //sensorData = sensorData + ",\"app_id\":\"" + a_cfgdat->getAppName() + "\"";
                // convenient for tracking data updates vs. data reports
                sensorData = sensorData + ",\"seq\":" + String(sensor.seq);
                sensorData = sensorData + ",\"t\":" + String(sensor.t) + ",\"h\":" + String(sensor.h);
                sensorData = sensorData + "}";
    
                int sent = sendUDP((char *)sensorData.c_str(), strlen(sensorData.c_str()));
                if(sent > 0)
                {
                    // NOTE: fixes frozen sensor, issue #11
                    sensorlast = sensor;

                    bRet = true;
                    if(!checkDebugMute()) Serial.println("data - " + sensorData);
                } else if(!checkDebugMute()) Serial.println("sendUDP() failed, sent = " + String(sent));
            }
        } else sensor.nextup = scfg.error_interval + millis();
    }
    return bRet;
}

/*
    Get the relative number of the pin we're supposed to use for
    getting data from the sensor.
*/
uint8_t getPin(sensorconfig &cfg)
{
uint8_t pin = 0;

#ifdef ARDUINO_ESP8266_NODEMCU
    if(!checkDebugMute()) Serial.println("cfg.pin = " + cfg.pin);

    if(cfg.pin == "D6") pin = D6;
    else if(cfg.pin == "D4") pin = D4;
#endif
#ifdef ARDUINO_ESP8266_ESP01
    if(!checkDebugMute()) Serial.println("pin will be GPIO2");
    pin = 2;
#endif
    return pin;
}

/*
    Get the sensor type from the config
*/
uint8_t getType(sensorconfig &cfg)
{
uint8_t type = 0;

    if(!checkDebugMute()) Serial.println("cfg.type = " + cfg.type);

    if(cfg.type == "DHT22") type = DHT22;
    else if(cfg.type == "DHT11") type = DHT11;

    return type;
}


/*
    Get the sensor read interval
*/
unsigned long getSensorInterval()
{
    return scfg.interval;
}

/*
    Start the sensor - finish any necessary initialization and
    get the first data reading.
*/
void startSensor()
{
    if(sens_cfgdat != NULL)
    {
        // get a copy of the sensor's configuration data
        sens_cfgdat->getSensor(scfg);

        // initialize the DHT...
        // NOTE: the DHT class was originally authored by AdaFruit. I 
        // made a copy and have modified it a little. See the comments
        // in src/adafruit/DHT.*
        dht.begin(getPin(scfg), getType(scfg));

        // "fake" the time, it will force an update
        // and send... 30 seconds is long enough to
        // let the sensor stabilize
        sensor.nextup = 30000 + millis();
    }
}

#ifdef __cplusplus
}
#endif
