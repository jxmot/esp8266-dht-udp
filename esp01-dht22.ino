/* ************************************************************************ */
/*
*/

// required include files...
#include "esp8266-ino.h"

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
    // 
    //startSensor();
    // initial setup is complete, wrap up and continue...
    setupDone();

    // announce that we're ready to any interested clients.
    ready();
}

/*
    Application Execution Loop
*/
void loop()
{
    yield();

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
static done = false;

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
        // read sensor data if it's time...

        // send latest sensor data
        // {"hostname":"ESP_290767","t":"71.5","h":"37","unit":"F"}


        //handleComm();
        //handleSensor();
    }
}


