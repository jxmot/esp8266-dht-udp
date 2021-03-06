/* ************************************************************************ */
/*
    esp8266-ino.cpp - support for the associated ino file.

    Stuff that's useful for multiple sketches and helps in keeping the 
    sketch file(s) uncluttered. Some of the code here might be modified
    from project to project. 

    Functionality included here - 

        * Configuration file reading and parsing
        * Application configuration and set update
        * Miscellaneous support functions.

    (c) 2017 Jim Motyl - https://github.com/jxmot/esp8266-dht-udp
*/
#include "esp8266-ino.h"

#ifdef __cplusplus
extern "C" {
#endif


// if the application configuration is present, and if the debug mute flag is 
// true then mute debug output
#define DEBUG_MUTE checkDebugMute()

// default serial baud rate, modify as needed
const int DEFAULT_SERIAL_BAUD = 115200;

// the default interval for the on-board LED on/off
int toggInterv = TOGGLE_INTERVAL;

// functions that are only called from within this module
void initLED();
void printAppCfg();
void printWiFiCfg();
bool connectWiFi(String ssid, String pass);
void printSrvCfg();
void printClientCfg();

// for keeping track of the use of the on-board LED
bool obLEDinUse = false;

// pointers to configuration data objects, after they're 
// created and contain the data they will be available
// through out the run-time of the sketch.
AppCfgData *a_cfgdat = NULL;
WifiCfgData *w_cfgdat = NULL;
SrvCfgData *s_cfgdat = NULL;
ClientCfgData *c_cfgdat = NULL;
MultiCastCfgData *m_cfgdat = NULL;
SensorCfgData *sens_cfgdat = NULL;

// error message string
String errMsg;

// pointer to the WiFi connection object
ConnectWiFi *connWiFi = NULL;

/* ************************************************************************ */
/*
    Print a start up message to the serial port
*/
void setupStart()
{
    Serial.begin(DEFAULT_SERIAL_BAUD);
    Serial.println();
    Serial.println();
    Serial.println("setup START");
    Serial.flush();
}

/*
    Read and parse the configuration files
*/
void setupConfig()
{
#ifdef CONFIG_DEMO
    if(setupApp("/appcfg.json")) 
    {
        if(setupWiFi("/wificfg.json")) 
        {
            if(!setupClient("/clientcfg.json")) toggInterv = ERR_TOGGLE_INTERVAL;
            else if(!setupMultiCast("/multicfg.json")) toggInterv = ERR_TOGGLE_INTERVAL;
            else if(!setupSensor("/sensorcfg.json")) toggInterv = ERR_TOGGLE_INTERVAL;
#else
    if(setupApp("/_appcfg.json")) 
    {
        // NOTE: The .gitignore in this repo is configured to ignore ALL
        // files that start with an underscore ('_'). This allows for
        // versions of these files that contain "sensitive" information
        // to be ignored by git. The reason that there are two copies of 
        // this code block is to serve as a gentle reminder that there can
        // be additional differences between modes. For example, some config 
        // operations might not be necessary in CONFIG_DEMO.
        if(setupWiFi(a_cfgdat->getWifiConfig())) 
        {
            if(!setupClient(a_cfgdat->getClientConfig())) toggInterv = ERR_TOGGLE_INTERVAL;
            else if(!setupMultiCast(a_cfgdat->getMcastConfig())) toggInterv = ERR_TOGGLE_INTERVAL;
            else if(!setupSensor(a_cfgdat->getSensorConfig())) toggInterv = ERR_TOGGLE_INTERVAL;
#endif
        } else toggInterv = ERR_TOGGLE_INTERVAL;
    } else  toggInterv = ERR_TOGGLE_INTERVAL;
}

/*
    Perform any required initialization steps after the config files
    have been read and parsed.
*/
void setupInit()
{
    // if we're not indicating an error the continue with the 
    // initialization of the UDP functionality...
    if(toggInterv == TOGGLE_INTERVAL) 
    {
        if(!initUDP()) 
        {
            printError(String(__func__), "UDP init failed!");
            toggInterv = ERR_TOGGLE_INTERVAL;
        }
    }
}
/*
    Print a start up done message to the serial port
*/
void setupDone()
{
    Serial.flush();
    Serial.println();
    Serial.println("setup DONE - " + (errMsg == "" ? "No Errors" : errMsg));
    Serial.println();
    Serial.flush();

#ifdef ARDUINO_ESP8266_NODEMCU
    initLED();
#endif
}

String millisToTime(int interval)
{
char buf[12];   

    unsigned long sec = interval / 1000;
    int hr = sec / 3600;
    sec %= 3600;
    int min = sec / 60;
    sec %= 60;

    sprintf(buf, "%02d:%02d:%02d", hr, min, sec);
    return String(buf);
}

void initLED()
{
    pinMode(LED_BUILTIN, OUTPUT);
    delay(10);
    digitalWrite(LED_BUILTIN, HIGH);
    obLEDinUse = true;
}

/*
    Toggle the on-board LED

    Checks to see if the on-board LED pin has been
    configured. If not it will configure it.

    NOTE: The first call to this function will turn on 
    the on-board LED. This behavior can be changed by
    modifying the declaration of ledTogg.

    ledTogg = false - the LED will turn ON with the 
                      first call.

    ledTogg = true  - the LED will turn OFF with the 
                      first call.

    The change in the LED state is also dependent on
    its initialization. That can be changed in setupDone().

*/
bool toggleLED()
{
static bool ledTogg = false;
//static bool ledTogg = true;

#ifdef ARDUINO_ESP8266_NODEMCU
    // NOTE: ESP-01's do not have the on-board LED
    if(obLEDinUse == false) initLED();

    ledTogg = !ledTogg;

    if(ledTogg) digitalWrite(LED_BUILTIN, LOW);
    else digitalWrite(LED_BUILTIN, HIGH);
#else
    // if nothing else, at least toggle the flag so that
    // it can be used elsewhere...
    ledTogg = !ledTogg;
#endif

    return ledTogg;
}

/*
    Print an error message, and change the baud rate if
    necessary.
*/
void printError(String func, String _errMsg)
{
    if(Serial.baudRate() != DEFAULT_SERIAL_BAUD)
    {
        Serial.flush();
        Serial.end();
        Serial.begin(DEFAULT_SERIAL_BAUD);
    }
    Serial.println(func + " ERROR! - " + _errMsg);
    Serial.flush();
}

/*
    Read and parse the application configuration settings, 
    and report any errors.
*/
bool setupApp(const String appCfgFile)
{
String func = String(__func__);
bool bRet = false;

    // get the config data...
    a_cfgdat = new AppCfgData((const char *)appCfgFile.c_str());

    // check for errors
    if(a_cfgdat->getError(errMsg) == 0) 
    {
        // success, parse the JSON string
        a_cfgdat->parseFile();

        // check for errors
        if(a_cfgdat->getError(errMsg)) printError(func, errMsg);
        else 
        {
            // success, display the config data
            printAppCfg();
            bRet = true;
        }
    } else printError(func, errMsg);

    // return the config-read status, true = success
    return bRet;
}

/*
    Print(to the serial port) the application setting values
*/
void printAppCfg()
{
    if(a_cfgdat != NULL)
    {
        Serial.println("getAppName - " + a_cfgdat->getAppName());
        Serial.println("getDebugMute - " + String(a_cfgdat->getDebugMute() ? "ON" : "OFF"));
        Serial.println();
    }
}

/*
    Read and parse the WiFi configuration settings, and make a
    connection to one of the configured access points. If successful
    then return true, otherwise return false.
*/
bool setupWiFi(const String wifiCfgFile)
{
String func = String(__func__);
bool isconnected = false;

    // get the config data...
    w_cfgdat = new WifiCfgData((const char *)wifiCfgFile.c_str(), DEBUG_MUTE);

    // check for errors
    if(!w_cfgdat->getError(errMsg)) 
    {
        // success, parse the JSON string
        w_cfgdat->parseFile();

        // check for errors
        if(w_cfgdat->getError(errMsg)) printError(func, errMsg);
        else 
        {
            // success, display the config data
            printWiFiCfg();

            // iterate through the configured APs until there's a successful
            // connection or none have occurred.
            int ix = 0;
            for(ix = 0; ix < w_cfgdat->getAPCount() && isconnected == false; ix++)
            {
                // connectWiFi() will retry for a successful connection for
                // specific number of attempts. It will return true if a 
                // connection is made.
                isconnected = connectWiFi(w_cfgdat->getSSID(ix), w_cfgdat->getPASS(ix));
            }

            // success?
            if(!isconnected) 
            {
                // no
                errMsg = "Could not connect to - ";
                for(ix = 0; ix < w_cfgdat->getAPCount(); ix++) errMsg += String("\n    ") + w_cfgdat->getSSID(ix);
                printError(func, errMsg);
            } else errMsg = "";
        }
    } else printError(func, errMsg);

    // return the connection status, true = success
    return isconnected;
}

/*
    Print the WiFi configuration values.
*/
void printWiFiCfg()
{
    if(!checkDebugMute())
    {
        Serial.println("getAPCount - " + String(w_cfgdat->getAPCount()));
        Serial.println();

        for(int ix = 0; ix < w_cfgdat->getAPCount(); ix++)
        {
            Serial.println("AP #" + String(ix));
            Serial.println("getSSID - " + w_cfgdat->getSSID(ix));
            Serial.println("getPASS - " + w_cfgdat->getPASS(ix));
            Serial.println();
        }
    }
}

/*
    Get the server address and port configuration values. These
    are typically servers that this application will communicate
    with.
*/
bool setupServers(const String srvCfgFile)
{
String func = String(__func__);
bool bRet = false;

    // get the config data...
    // NOTE: The total quantity of server configs is limited,
    // see the SrvCfgData class for details.
    s_cfgdat = new SrvCfgData((const char *)srvCfgFile.c_str(), DEBUG_MUTE);

    // check for errors
    if(!s_cfgdat->getError(errMsg)) 
    {
        // success, parse the JSON string
        s_cfgdat->parseFile();

        // check for errors
        if(s_cfgdat->getError(errMsg)) printError(func, errMsg);
        else 
        {
            // success, display the config data
            printSrvCfg();
            bRet = true;
        }
    } else printError(func, errMsg);

    // return the config-read status, true = success
    return bRet;
}

/*
    Print the server configuration values
*/
void printSrvCfg()
{
srvcfg cfg;
// NOTE: These strings are used in the configuration file
// to separate the various servers that the application 
// might use. Retrieving the information using a string
// eliminates the need to keep indices in sync between the
// application and the config data.
// 
// To DO: Obtain these strings from a config file, and/or
//        seek a better method.
const String labels[] = {"udp","http","END"};

    if(!checkDebugMute())
    {
        for(int ix = 0; labels[ix] != "END"; ix++)
        {
            if(s_cfgdat->getServerCfg(labels[ix], cfg))
            {
                Serial.println("label = " + cfg.label);
                Serial.println("port  = " + String(cfg.port));
                Serial.println();
            } else Serial.println("not found in cfg - " + labels[ix]);
        }
    }
}

/*
    get the client config data, like the server address and port
    that we'll connect with
*/
bool setupClient(const String clientCfgFile)
{
String func = String(__func__);
bool bRet = false;

    // get the config data...
    // NOTE: The total quantity of server configs is limited,
    // see the SrvCfgData class for details.
    c_cfgdat = new ClientCfgData((const char *)clientCfgFile.c_str(), DEBUG_MUTE);

    // check for errors
    if(!c_cfgdat->getError(errMsg)) 
    {
        // success, parse the JSON string
        c_cfgdat->parseFile();

        // check for errors
        if(c_cfgdat->getError(errMsg)) printError(func, errMsg);
        else 
        {
            // success, display the config data
            printClientCfg();
            bRet = true;
        }
    } else printError(func, errMsg);

    // return the config-read status, true = success
    return bRet;
}

/*
    Print the client configuration values
*/
void printClientCfg()
{
clisrvcfg cfg;
// NOTE: These strings are used in the configuration file
// to separate the various servers that the application 
// might use. Retrieving the information using a string
// eliminates the need to keep indices in sync between the
// application and the config data.
// 
// To DO: Obtain these strings from a config file, and/or
//        seek a better method.
const String labels[] = {"udp1","udp2","END"};

    if(!checkDebugMute())
    {
        for(int ix = 0; labels[ix] != "END"; ix++)
        {
            if(c_cfgdat->getServer(labels[ix], cfg))
            {
                Serial.println("label = " + cfg.label);
                Serial.println("port  = " + String(cfg.port));
                Serial.println();
            } else Serial.println("not found in cfg - " + labels[ix]);
        }
    }
}

/*
    Connect to an access point with its SSID and password,
    this function will retry for 'N' times if unsuccessful.
    Returns true if connected, otherwise returns false.
*/
bool connectWiFi(String ssid, String pass)
{
conninfo conn;

    // debug stuff
    if(!checkDebugMute())
    {
        Serial.println("Attempting connection to - ");
        Serial.println("ssid : " + ssid);
        Serial.println("pass : " + pass);
    }
    // /debug stuff

    // attempt to connect with the specified access point...
    connWiFi = new ConnectWiFi(ssid.c_str(), pass.c_str());

    // debug stuff
    if(!checkDebugMute())
    {
        Serial.println("connected? - " + String(connWiFi->IsConnected()? "yes" : "no"));
        Serial.println();

        // connWiFi is global to any code that includes esp8266-ino.h
        // 
        // NOTE: The values are static, and obtained when the connection
        // was successful.
        if(connWiFi->GetConnInfo(&conn)) 
        {
            Serial.println("ip  : " + conn.localIP);
            Serial.println("mac : " + conn.macAddress);
            Serial.println("host: " + conn.hostname);
            Serial.println("Connection Attempt Stats : ");
            Serial.println("attempts = " + String(conn.attempts));
            Serial.println("time     = " + String(conn.timeToConnect));
            Serial.println("rssi     = " + String(conn.rssi) + " dBm");
            Serial.println();
        }
    }
    // /debug stuff

    return connWiFi->IsConnected();
}

/*
    Read and parse the UDP multicast config data
*/
bool setupMultiCast(const String mcastCfgFile)
{
String func = String(__func__);
bool bRet = false;

    // get the config data...
    m_cfgdat = new MultiCastCfgData((const char *)mcastCfgFile.c_str(), DEBUG_MUTE);

    // check for errors
    if(!m_cfgdat->getError(errMsg)) 
    {
        // success, parse the JSON string
        m_cfgdat->parseFile();

        // check for errors
        if(m_cfgdat->getError(errMsg)) printError(func, errMsg);
        else 
        {
            // debug stuff
            // success, display the config data
            if(!checkDebugMute())
            {
                mcastcfg cfg;
                if(m_cfgdat->getCfg(cfg)) 
                {
                    Serial.println("Multicast addr - " + cfg.addr);
                    Serial.println("Multicast port - " + String(cfg.port));
                }
                Serial.flush();
            }
            // /debug stuff
            bRet = true;
        }
    } else printError(func, errMsg);

    // return the config-read status, true = success
    return bRet;
}

/*
    Read and parse the sensor configuration file
*/
bool setupSensor(const String sensorCfgFile)
{
String func = String(__func__);
bool bRet = false;

    sens_cfgdat = new SensorCfgData((const char *)sensorCfgFile.c_str(), DEBUG_MUTE);

    if(!sens_cfgdat->getError(errMsg)) 
    {
        // success, parse the JSON string
        sens_cfgdat->parseFile();

        // check for errors
        if(sens_cfgdat->getError(errMsg)) printError(func, errMsg);
        else 
        {
            bRet = true;

            // debug stuff
            // success, display the config data
            if(!checkDebugMute())
            {
                sensorconfig cfg;
                if(sens_cfgdat->getSensor(cfg)) 
                {
                    Serial.println("Sensor type - " + cfg.type);
                    Serial.println("Sensor scale - " + cfg.scale);
                    Serial.println("Sensor interval - " + String(cfg.interval) + " = " + millisToTime(cfg.interval));
                    Serial.println("Error interval - " + String(cfg.error_interval) + " = " + millisToTime(cfg.error_interval));
                    Serial.println("Sensor report - " + cfg.report);
                    Serial.println("Sensor delta T - " + String(((float)(cfg.delta_t)/10)));
                    Serial.println("Sensor delta H - " + String(((float)(cfg.delta_h)/10)));
                }
                Serial.flush();
            }
            // /debug stuff
        }
    } else printError(func, errMsg);
    return bRet;
}

/*
    return the debug mute flag, true = muted

    If the application configuration doesn't exist this function
    will return true (muted)
*/
bool checkDebugMute()
{
    if(a_cfgdat != NULL) return a_cfgdat->getDebugMute();
    return true;
}

#define QUERY_SERVER
#ifdef QUERY_SERVER
#include "ParseIPReply.h"
ParseIPReply *pr = new ParseIPReply();

bool queryServer(String, String &);
#endif

/*
    send a UDP multi-cast to any interested clients
        - OR - 
    broadcast a request for the UDP server's IP address
*/
void ready()
{
#ifdef QUERY_SERVER
String data = "";
ipreply r;

    if(queryServer("REQ_IP", data)) 
    {
        if(data.indexOf("IP_ADDR") > 0)
        {
            r = pr->parseReply(data);
            setUDP(r.ip, r.port);
        }
    }
#else
    sendStatus("APP_READY");
#endif
}

#ifdef QUERY_SERVER
/*
    TODO: constants should be defined or configurable
*/
void sendQuery(String query) 
{
    int port = 43210;
    beginUDP(port);
    sendStatus(query, String(port));
}

#define MAX_WAIT 8

bool queryServer(String query, String &datain)
{
conninfo conn;
int waitcount = 0;
bool bRet = false;

    // connected?
    if(connWiFi->GetConnInfo(&conn))
    {
        //int port = 43210;
        //beginUDP(port);
        //sendStatus(query, String(port));

        sendQuery(query);

        while(true) {
            yield();
            if(recvUDP() > 0) 
            {
                if(!checkDebugMute()) 
                {
                    Serial.println();
                    Serial.println("queryServer() reply - " + String((char *)&readBuffer[0]));
                }
                sendStatus("APP_READY");
                datain = (char *)&readBuffer[0];
                bRet = true;
                break;
            }
            if(!checkDebugMute()) Serial.print(".");
            delay(250);
            waitcount += 1;
            if(waitcount >= MAX_WAIT)
            {
                waitcount = 0;
                if(!checkDebugMute()) Serial.println("");
                sendQuery(query);
            }
        }
        //stopUDP();
    }
    return bRet;
}
#endif

/*
    Send a status message via UDP multicast
*/
void sendStatus(String status, String msg)
{
conninfo conn;
String statusData;

    // connected?
    if(connWiFi->GetConnInfo(&conn)) 
    {
        statusData = "{\"dev_id\":\"" + conn.hostname + "\"";
        statusData = statusData + ",\"status\":\"" + status + "\"";
        if(strlen(msg.c_str()) > 0) statusData = statusData + ",\"msg\":\"" + msg + "\"";
        statusData = statusData + "}";

        if(!checkDebugMute()) Serial.println("sendStatus() - " + statusData);

        if(strlen(statusData.c_str()) <= UDP_PAYLOAD_SIZE) multiUDP((char *)statusData.c_str(), strlen(statusData.c_str()));
        else if(!checkDebugMute()) Serial.println("sendStatus() - NOT sent, too long");
    }
}

#ifdef __cplusplus
}
#endif

