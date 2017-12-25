/* ************************************************************************ */
/*
    esp8266-dht-udp.ino - An ESP8266 application that reads the temperature
    and humidity using a DHT22 or DHT11 sensor. The values are sent to a
    server via UDP.
*/

// required include files...
#include "esp8266-ino.h"
#include "sensor-dht.h"

#define USE_OTA
#ifdef USE_OTA
#include "esp8266-ota.h"
#endif
/* ************************************************************************ */
/*
    Application Set Up
*/
void setup()
{
    // begin the set up process...
    setupStart();
    // read and parse the necessary config files
    setupConfig();
    // initialize prior to running the application
    setupInit();
    // initial setup is complete, wrap up and continue...
    setupDone();
    // announce that we're ready to any interested clients
    ready();
    // start up the sensor and begin reading data from it
    startSensor();
#ifdef USE_OTA
    // init the for ota...
    initOTA();
#endif
}

/*
    Application Execution Loop
*/
void loop()
{
    yield();

#ifdef USE_OTA
    if(waitForOTA()) return;
#endif

    // NOTE: using the LED toggle interval value to indicate 
    // an error comes from a prior interation of this code. 
    // however this application only flashes the LED if an
    // error has non-recoverable occurred. such errors include :
    //      * missing or invalid config file(s)
    //      * a received UDP packet length exceeded the buffer size
    //      * could not initialize for UDP
    if(toggInterv == ERR_TOGGLE_INTERVAL)
    {
#ifdef ARDUINO_ESP8266_NODEMCU
        // wait now before proceeding...
        delay(toggInterv);
        toggleLED();
#else
static bool done = false;

        // need some other type of error indicator
        if(done == false) 
        {
            Serial.println("Last error - " + errMsg);
            done = true;
        }
#endif
    }
    else
    {
        // read sensor data if it's time and send the new data...
        sendSensorData();
    }
}


