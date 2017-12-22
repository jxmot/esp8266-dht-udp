/* ************************************************************************ */
/*
    sensor-dht.cpp - support for DHTxx type temperature and humidity sensors.
*/
// required include files...
#include "esp8266-ino.h"
#include "sensor-dht.h"

// Sensor Settings
// what digital pin we're connected to.
// If you are not using NodeMCU change D6 to real pin
const uint8_t DHTPIN = D6;     		
const uint8_t DHTTYPE = DHT22;

// Initialize the temperature/ humidity sensor
DHT dht(DHTPIN, DHTTYPE);

sensorconfig scfg;
livesensor sensor;
uint16_t usc = 0;

void startSensor()
{
    if(sens_cfgdat != NULL)
    {
        sens_cfgdat->getSensor(scfg);
    }
}

void updateSensorData() 
{
    sensor.seq += 1;
    if(!checkDebugMute()) Serial.println("updateSensorData() - " + String(sensor.seq));

    // read values from the sensor
    sensor.h = dht.readHumidity();
    sensor.t = dht.readTemperature(!(scfg.unit == "F" ? false : true));

    if(!checkDebugMute()) Serial.println(String(sensor.t) + "  " + String(sensor.h));
}
