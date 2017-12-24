# esp8266-dht-udp

A temperature and humidity sensor using a DHT22/11 device and UDP to transmit the data to a server.

# Status

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


