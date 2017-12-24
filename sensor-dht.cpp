/* ************************************************************************ */
/*
    sensor-dht.cpp - support for DHTxx type temperature and humidity sensors.
*/
// required include files...
#include "esp8266-ino.h"
#include "esp8266-udp.h"
#include "sensor-dht.h"


// Initialize the temperature/ humidity sensor
DHT dht;

sensorconfig scfg;
livesensor sensor;
livesensor sensorlast;

void updateSensorData() 
{
    sensor.seq += 1;
    if(!checkDebugMute()) Serial.println("updateSensorData() - " + String(sensor.seq));

    // read values from the sensor
    sensor.h = dht.readHumidity();
    sensor.t = dht.readTemperature(!(scfg.unit == "F" ? false : true));

    if(isnan(sensor.t)) sensor.t = sensorlast.t;
    if(isnan(sensor.h)) sensor.h = sensorlast.h;

    if(!checkDebugMute()) Serial.println(String(sensor.t) + "  " + String(sensor.h));
}

bool chkReport()
{
bool bRet = false;

    if(scfg.report == "ALL") bRet = true;
    else
    {
        if(scfg.report == "CHG")
        {
            float t_temp = abs(sensor.t - sensorlast.t);
            float h_temp = abs(sensor.h - sensorlast.h);

            if((t_temp > (scfg.delta / 10)) || (h_temp > (scfg.delta / 10))) bRet = true;
        } else bRet = true;
    }
    return bRet;
}

bool sendSensorData()
{
bool bRet = false;
conninfo conn;
String sensorData;

    if(sensor.nextup < millis())
    {
        sensorlast = sensor;
        updateSensorData();
        sensor.nextup = scfg.interval + millis();

        if(!checkDebugMute())
        {
            Serial.println("last - " + String(sensorlast.t) + "  " + String(sensorlast.h));
            Serial.println("live - " + String(sensor.t) + "  " + String(sensor.h));
        }

        if(connWiFi->GetConnInfo(&conn) && chkReport())
        {
            // {"hostname":"ESP_290767","t":"71.5","h":"37","unit":"F"}
            sensorData = "{\"hostname\":\"" + conn.hostname + "\"";
            sensorData = sensorData + ",\"appname\":\"" + a_cfgdat->getAppName() + "\"";
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

uint8_t getType(sensorconfig &cfg)
{
uint8_t type = 0;

    if(!checkDebugMute()) Serial.println("cfg.type = " + cfg.type);

    if(cfg.type == "DHT22") type = DHT22;
    else if(cfg.type == "DHT11") type = DHT11;

    return type;
}

void startSensor()
{
    if(sens_cfgdat != NULL)
    {
        sens_cfgdat->getSensor(scfg);

        //dht.begin(DHTPIN, DHTTYPE);
        dht.begin(getPin(scfg), getType(scfg));

        // "fake" the time, it will force an update
        // and send...
        sensor.nextup = 0;
        sendSensorData();
    }
}


