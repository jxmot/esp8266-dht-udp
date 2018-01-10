# esp8266-dht-udp

A temperature and humidity sensor using a DHT22/11 device and UDP to transmit the data to a server.

# History

I had been tinkering with using an ESP8266 and a temperature/humidity sensor to record data within my home. I hadn't really moved forward with it until the end of December 2017. And what really prompted me to move forward with the project was the brutally cold temperatures we experienced in Chicago. I was concerned about water pipes freezing, or my furnace breaking down. So the completion of this project (*sensors, server, database, and web client*) became *very* important. Fortunately no pipes froze, and my furnace continued working! I was luckier than some of my neighbors!

# Overview

Each *sensor* consists of an ESP-01S and a DHT22 temperature/humidity sensor. They are somewhat configurable, and utilize OTA for firmware and configuration file updates. In addition the sensors utilize UDP to communicate status and data to a server implemented to run on NodeJS.

# Details

## Operation

This application requires a server capable of receiving UDP packets. I've chosen NodeJS for this, and a server can be found in my [node-dht-udp](https://github.com/jxmot/node-dht-udp) repository.

It is the responsibility of that server to listen for data from the sensors and forward it to a Firebase database.

### Network Traffic

## Configuration

The configuration source code is based on my [ESP8266-config-data-V2](<https://github.com/jxmot/ESP8266-config-data-V2>) repository. Therefor only the configurable items and their use will be described here.

The configuration files are kept in the `data` folder.

### File Naming Convention

Some configuration files may contain *sensitive* information that should not be placed into a public repository. In order to prevent them from getting into the repository their filenames begin with an underscore. This is accomplished with an entry in this repository's `.gitignore` file. However there are incomplete configuration files provided that to not have the underscore in their names.

### Application Configuration

This file is the first configuration file read by the application. It provides a location where the remaining configuration files are listed. It also contains the *application name*, and a boolean value for enabling or disabling run-time output to the serial port. Here are the contents of the sample `appcfg.dat` file -

```json
{
    "appname":"Your App Name Here",
    "debugmute":false,
    "wificonfig":"/wificfg.dat",
    "clientconfig":"/clientcfg.dat",
    "mcastconfig":"/multicfg.dat",
    "sensorconfig":"/sensorcfg.dat"
}
```

To keep the contents of this file secure make a copy of it and prepend the underscore to its name. 

### WiFi Configuration

The `data/wificfg.dat` file contains one or more WiFi SSIDs and passwords. Here are the contents of the sample `wificfg.dat` file -

```json
{ "apoints":[
{"ssid":"your ssid-1 here","pass":"your wifi password-1 here"},
{"ssid":"your ssid-2 here","pass":"your wifi password-2 here"}
],"apcount":2}
```

The code responsible for connecting to an access point will multiple attempts. This is describe in the README of the [ESP8266-config-data-V2](<https://github.com/jxmot/ESP8266-config-data-V2>) repository. 

To keep the contents of this file secure make a copy of it and prepend the underscore to its name. Be sure to edit your `data/_appcfg.data` file to access the correct file.

### UDP Client Configuration

The `data/clientcfg.dat` file contains one or more entries that each contain the IP address and port number of a UDP enabled server that the application can access. Here are the contents of the sample `clientcfg.dat` file -

```json
{
"udp1":{"addr":"server IP address","port":54321},
"udp2":{"addr":"server IP address","port":54321}
}
```

The server is chosen in `esp8266-udp.cpp`, `initUDP()`. Edit this line - 

```c++
if(c_cfgdat->getServer("udp1", udpClient)) success = true;
```

and change `"udp1"` to a different string as needed.

To keep the contents of this file secure make a copy of it and prepend the underscore to its name. Be sure to edit your `data/_appcfg.data` file to access the correct file.

### Multi-cast UDP Configuration

The `data/multicfg.dat` file contains the IP address and port number for the intended multi-cast destination. Here are the contents of the `clientcfg.dat` file -

```json
{
    "addr":"224.0.0.1",
    "port":54321
}
```

This file does not contain sensitive configuration data. So it is not necessary to prepend the underscore to its name.

### Sensor Configuration

The `data/sensorcfg.dat` file contains the configure the application for one of two specific temperature/humdity sensors - 

* Sensor type, either `"DHT11"` or `"DHT22"`. At this time these are the only sensors supported.
* EPS8266 pin number, this is the pin number of the ESP8266 that is used for communication with the DHT sensor. 
    * **NOTE** : This pin setting is ignored if an ESP-01 is used. On that platform GPIO2 will be used instead and is not configurable. See `sensor-dht.cpp` and look for `ARDUINO_ESP8266_ESP01` for the associated code.
* Temperature scale, this is used to select Fahrenheit or Celsius.
* Sensor reading interval, this is the duration in milliseconds between subsequent sensor data readings.
* Reporting type, the current choices are `"ALL"` or `"CHG"`. Their meanings are - 
    * `"ALL"` - report the sensor data *every time* the sensor data is read.
    * `"CHG"` - only report sensor data *if* the temperature or humidity values have changed.
* If the reporting type is `"CHG"` then this is the amount of required change before the temperature or humidity are reported. The integer value kept here is divided by 10 to create a floating point value. Then if the amount of change (*temperature or humidity*) is greater then the data is sent.

Here are the contents of the `sensorcfg.dat` file -

```json
{
    "type":"DHT22",
    "pin":"D6",
    "scale":"F",
    "interval":90000,
    "report":"CHG",
    "delta": 1
}
```

This file does not contain sensitive configuration data. So it is not necessary to prepend the underscore to its name.

## OTA

I experimented with OTA with limited results. And the device would not appear reliably in the Arduino IDE. So it has been disabled. To enable OTA removed the comment on the line `//#define USE_OTA` in `esp8266-dht-udp.ino`.

### Configuration

The OTA configuration is located in `data/otacfg.dat`.

## Sending Data and Status

## Schematic and Build Details

### Parts List and Sources

## DHTxx Library Modifications

The following modifications were made to the *Adafruit* DHT source files `src/adafruit/DHT.h` and `src/adafruit/DHT.cpp` - 

* Added an overloaded contructor - `void DHT::DHT(void)`
* Added an overloaded begin function - `void DHT::begin(uint8_t pin, uint8_t type)`

The purpose of the changes was to allow the DHT class to be instantiated before the sensor configuration was read and parsed. 

# Future Modifications

## Application Version

Add a "version" member to the `appcfg.dat` file. This would contain a string formatted as `"0.0.0"` using the typical "major"."minor"."patch" scheme. For example - 

```json
{
    "appname":"Your App Name Here",
    "appver":"0.1.0",
    "debugmute":false,
    "wificonfig":"/wificfg.dat",
    "clientconfig":"/clientcfg.dat",
    "mcastconfig":"/multicfg.dat",
    "sensorconfig":"/sensorcfg.dat"
}
```

In addition single letters such as "`b`" could be added to the end of the version string.

## Configuration File Naming

To accommodate devices with configuration differences the following changes will be made - 

* One or more configuration files will can be named using the ESP-01 device *hostname*. For example in `appcfg.dat` - 

```json
{
    "appname":"Your App Name Here",
    "debugmute":false,
    "wificonfig":"/wificfg.dat",
    "clientconfig":"/clientcfg.dat",
    "mcastconfig":"/multicfg.dat",
    "sensorconfig":"/sensorcfg-ESP_49F542.dat"
}
```

Where `"sensorconfig":"/sensorcfg-ESP_49F542.dat"` would specifiy the sensor config file for a specific device. The addtion of `ESP_49F542` would match the *hostname* of a specific device.

## Run-time Configuration

Commands could be issued from the server that would alter one or more configuration items. For example the following could be reconfigured - 

* Sensor - 
    * scale
    * interval
    * report type
    * delta
* *TBD*

# Status (Historical)

The statuses below are listed with the latest status at the top - 

**2017-12-26** : All OTA is working, sketches and SPIFFS! Instead of modifying the `ESP8266FS.java` and `platform.txt` I ended up adding the python path to my enviroment path variable. This was only due to the fact that I could not figure out how to *build* the modified flash tool jar file. I understand the process because I was able to get it to build successfully on *Travis CI*. But I could not find a quick way to get the jar file back from it. So after spending a measurable amount of time on this it just seemed quicker (and it was) to just go ahead and modifiy the path variable.

**2017-12-25** : OTA updates for sketches is working. SPIFFS, not so much. The problem can be fixed by making a couple of modifications - 

* The file `C:\Users\SOME_USER\AppData\Local\Arduino15\packages\esp8266\hardware\esp8266\2.3.0\platform.txt` requires that the line - 

`tools.esptool.network_cmd.windows=python.exe`

be changed to - 

`tools.esptool.network_cmd.windows=C:\Python27amd64\python.exe`

The reason for the change is how python was installed. In my case it was installed after the Arduino IDE and it was installed as part of a **Microsoft Visual Studio Community Edition** installation. When that occurred two versions of python were installed, 2.7 and 3.6. And neither are in the Windows `path`. I could have modified the `path` but it would be difficult to know immediately if there were any side effects in regards to Visual Studio. So that made my other option to be the modification of settings in the Arduino IDE. And that would be isolated and have no external side effects.

* SPIFFS download tool - The one I'm using is found at <https://github.com/esp8266/arduino-esp8266fs-plugin>. And it's java source file `ESP8266FS.java` requires a small modification - 

The line that reads - 

`pythonCmd = "python.exe";`

should be changed to - 

`pythonCmd = PreferencesData.get("tools.esptool.network_cmd.windows");`

**NOTE** : This has not been tested yet, I'll have to build and test it.


**2017-12-23** : So far, so good! The code will run on NodeMCU *and* ESP-01S! At this point the following is working - 

* On power up and configuration a *UDP multi-cast* packet is sent. It's purpose is to identify the device and announce that is is "ready".
* Uses configuration file(s) to specify the type and pin number used by the DHTxx sensor.
* Uses configuration file(s) to specify the interval between sensor readings
* Sends a UDP packet to a configured server, does not require a reply from the server.
* Using `#ifdef` to build differently for NodeMCU vs ESP-01

**To Do** : 

* Implement both configurable reading modes - "ALL" and "CHG". - **DONE**
* Create a proper README - **WIP**
* Finish the NodeJS server, it will - 
    * forward sensor data to - 
        * IFTTT
        * a database, mLab or a local MySQL
        * no storage, but provide live-data html pages
* consider a redesign where this code becomes "generic" and devices can be server-configured.


