# esp8266-dht-udp

A temperature and humidity sensor using a DHT22/11 device and UDP to transmit the data to a server.

# Status

**2017-12-25** : OTA updates for sketches is working. SPIFFS, not so much. The problem can be fixed by making a couple of modifications - 

* The file `C:\Users\SOME_USER\AppData\Local\Arduino15\packages\esp8266\hardware\esp8266\2.3.0\platform.txt` requires that the line - 

`tools.esptool.network_cmd.windows=python.exe`

be changed to - 

`tools.esptool.network_cmd.windows=C:\Python27amd64\python.exe`

The reason for the change is how python was installed. In my case it was installed after the Arduino IDE and it was installed as part of a Microsoft Visual Studio Community Edition installation. When that occurred two versions of python were installed, 2.7 and 3.6. And neither are in the Windows `path`. I could have modified the `path` but it would be difficult to know immediately if there were any side effects in regards to Visual Studio. So that made my other option to be the modification of settings in the Arduino IDE. And that would be isolated and have no external side effects.

* SPIFFS down load tool - The one I'm using is found at <https://github.com/esp8266/arduino-esp8266fs-plugin>. And it's java source files requires a small modification - 

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
* Create a proper README
* Finish the NodeJS server, it will - 
    * forward sensor data to - 
        * IFTTT
        * a database, mLab or a local MySQL
        * no storage, but provide live-data html pages
* consider a redesign where this code becomes "generic" and devices can be server-configured.


