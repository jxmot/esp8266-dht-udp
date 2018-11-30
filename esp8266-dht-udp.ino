/* ************************************************************************ */
/*
    esp8266-dht-udp.ino - An ESP8266 application that reads the temperature
    and humidity using a DHT22 or DHT11 sensor. The values are sent to a
    server via UDP.

    (c) 2017 Jim Motyl - https://github.com/jxmot/esp8266-dht-udp
*/
#define HEARTBEAT

// required include files...
#include "src/applib/esp8266-ino.h"
#include "src/applib/sensor-dht.h"

// disabled OTA due to unreliability in regards to
// seeing the device on the Arduino IDE
//#define USE_OTA
#ifdef USE_OTA
#include "src/applib/esp8266-ota.h"

bool start = false;
#endif

#ifdef HEARTBEAT
void startHeart();
void heartBeat();

// can disable sending the heartbeat status
bool sendbeat = false;
// toggles on every beat
bool pulse = true;
// default rate is 15 sec (very short!)
// uses (sensor interval * 4) in startHeart()
unsigned long heartrate = 15000;
// time of last heartbeat, in milliseconds
unsigned long lastbeat = 0;
// count of beats so far
unsigned long beatcount = 0;
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
#ifdef HEARTBEAT
    startHeart();
#endif
}

/*
    Application Execution Loop
*/
void loop()
{
bool datasent = false;

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
    // an error comes from a prior iteration of this code. 
    // however this application only flashes the LED if an
    // error has non-recoverable occurred. such errors include :
    //      * missing or invalid config file(s)
    //      * a received UDP packet length exceeded the buffer size
    //      * could not initialize UDP
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
        datasent = sendSensorData();
    }
#ifdef HEARTBEAT
    // works best when sensor reporting 
    // mode is "CHG"
    if(!datasent) heartBeat();
    else lastbeat = millis();
#endif
}

#ifdef HEARTBEAT
void startHeart()
{
    lastbeat  = millis();
    beatcount = 0;
    heartrate = getSensorInterval() * 4;
    Serial.println("heart started, beats @ "+String((float(heartrate/1000)/60))+" min");
    sendStatus("INFO", "heartbeats @ "+String((float(heartrate/1000)/60))+" min");
}

void heartBeat()
{
sensornow tmp;

    if((lastbeat + heartrate) < millis())
    {
        lastbeat = millis();
        beatcount += 1;

// a "safety", have seen where the heart rate appears
// to have been shortened by some unknown means. This
// should reset the rate back to the configured setting.
        heartrate = getSensorInterval() * 4;

        if(sendbeat) 
        {
            sendStatus((pulse ? "TICK" : "TOCK"), "beatcount = "+String(beatcount));
            pulse = !pulse;
        }
        readSensorNow(tmp);
        sendSensorNow(tmp);
    }
}
#endif
