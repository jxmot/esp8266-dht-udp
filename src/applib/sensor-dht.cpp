/* ************************************************************************ */
/*
    sensor-dht.cpp - support for DHTxx type temperature and humidity sensors.

    To Do : Currently this code can only handle a single sensor during 
    run-time. Modifiy it to be able to operate on a selected individual 
    sensor. 
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
    object. Also check it for "is a NaN" and if true use the last
    data read.
*/
bool updateSensorData() 
{
bool bRet = true;

    // each time we update the data from the sensor increment the
    // sequence number. this will assist in determining data updates vs
    // data reports.
    sensor.seq += 1;
    if(!checkDebugMute()) Serial.println("updateSensorData() - " + String(sensor.seq));

    // read values from the sensor
    sensor.h = dht.readHumidity();
    sensor.t = dht.readTemperature(!(scfg.scale == "F" ? false : true));

    if(!checkDebugMute()) Serial.println(String(sensor.t) + "  " + String(sensor.h) + "    " + String((isnan(sensor.t) || isnan(sensor.h)) ? "NaN!" : "ok")  + "    " + String(sensor.nancount));

    if(isnan(sensor.t) || isnan(sensor.h))
    {
        sensor.h = sensorlast.t = 0;
        sensor.t = sensorlast.h = 0;

        bRet = false;

        if(sensor.nancount >= MAX_NAN) 
        {
            sendStatus("SENSOR_ERROR", "Too many NaN readings from sensor");
            sensor.nancount = 0;
        } else sensor.nancount += 1;
    } else sensor.nancount = 0;

    return bRet;
}

/*
    Check the configured reporting type and decide if the data should
    be reported (sent via UDP)
*/
bool chkReport()
{
bool bRet = false;
float f_deltaT = ((float)(scfg.delta_t) / 10);
float f_deltaH = ((float)(scfg.delta_h) / 10);

    // report ALL values as they are read
    if(scfg.report == "ALL") bRet = true;
    else
    {
        // report only if a change was detected...
        if(scfg.report == "CHG")
        {
            // calculate the amount of change (if any)
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
            sensorlast = sensor;

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
        // update the sensor data, if an error occured then 
        // change the interval between retries... success?
        if(updateSensorData()) sensor.nextup = scfg.interval + millis();
        else sensor.nextup = scfg.error_interval + millis();

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
                bRet = true;
                if(!checkDebugMute()) Serial.println("data - " + sensorData);
            }
        }
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
        // NOTE: the DHT class was orignally authored by AdaFruit. I 
        // made a copy and have modified it a little. See the comments
        // in src/adafruit/DHT.*
        dht.begin(getPin(scfg), getType(scfg));

        // "fake" the time, it will force an update
        // and send...
        sensor.nextup = 0;
        sendSensorData();
    }
}

#ifdef __cplusplus
}
#endif
