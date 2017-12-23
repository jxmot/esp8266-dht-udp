/* ************************************************************************ */
/*
    sensor-dht.cpp - support for DHTxx type temperature and humidity sensors.
*/
// required include files...
#include "esp8266-ino.h"
#include "esp8266-udp.h"
#include "sensor-dht.h"

// Sensor Settings
// what digital pin we're connected to.
// If you are not using NodeMCU change D6 to real pin
const uint8_t DHTPIN = D6;     		
const uint8_t DHTTYPE = DHT22;

// Initialize the temperature/ humidity sensor
//DHT dht(DHTPIN, DHTTYPE);
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

        if(connWiFi->GetConnInfo(&conn))
        {
            // {"hostname":"ESP_290767","t":"71.5","h":"37","unit":"F"}
            sensorData = "{\"hostname\":\"" + conn.hostname + "\",\"appname\":\"" + a_cfgdat->getAppName() + "\"";
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

    if(!checkDebugMute()) Serial.println("cfg.pin = " + cfg.pin);

    if(cfg.pin == "D6") pin = D6;
    else if(cfg.pin == "D4") pin = D4;

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


