/* ************************************************************************ */
/*
    esp8266-dht-udp.ino - An ESP8266 application that reads the temperature
    and humidity using a DHT22 or DHT11 sensor. The values are sent to a
    server via UDP.
*/

// required include files...
#include "src/applib/esp8266-ino.h"
#include "src/applib/sensor-dht.h"

// disabled OTA due to unreliablity in regards to
// seeing the device on the Arduino IDE
//#define USE_OTA
#ifdef USE_OTA
#include "src/applib/esp8266-ota.h"

bool start = false;
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
#ifdef USE_OTA
    // init for ota...
    initOTA();
#else
    // announce that we're ready to any interested clients
    ready();
    // start up the sensor and begin reading data from it
    startSensor();
#endif
}

/*
    Application Execution Loop
*/
void loop()
{
#ifndef ARDUINO_ESP8266_NODEMCU
static bool error_sent = false;
String lasterr = "";
#endif

    yield();

#ifdef USE_OTA
    // wait until the OTA wait-time exprires, then
    // run the rest of the app...
    if(waitForOTA()) return;
    else if(start == false)
    {
        start = true;
        // announce that we're ready to any interested clients
        ready();
        // start up the sensor and begin reading data from it
        startSensor();
    }
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
        // need some other type of error indicator
        if(error_sent == false) 
        {
            lasterr = "Last error - " + errMsg;
            Serial.println(lasterr);
            sendStatus("ERROR", lasterr);
            error_sent = true;
        }
#endif
    }
    else
    {
        // read sensor data if it's time and send the new data...
        sendSensorData();
    }
}


