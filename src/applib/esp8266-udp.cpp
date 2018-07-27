/* ************************************************************************ */
/*
    esp8266-udp.cpp - UDP support for the associated ino file.

    To Do :

        * support mulitple UDP server endpoints simultaneously for our client
        * make ACK requirement configurable per endpoints
        * 

*/
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include "esp8266-ino.h"

#ifdef __cplusplus
extern "C" {
#endif

int replyUDP(char *payload, int len);
int recvUDP();

// read & write buffers
unsigned char readBuffer[UDP_PAYLOAD_SIZE_READ];
unsigned char writeBuffer[UDP_PAYLOAD_SIZE_WRITE];

// Arduino UDP object
WiFiUDP udp;

srvcfg      udpServer;
clisrvcfg   udpClient;

/* ************************************************************************ */
/*
    Obtains the UDP configuration data, apply it and do any other necessary
    UDP config steps...
*/
int initUDP()
{
bool success = false;
int iRet = 0;

    // make sure the WiFi is connected...
    if((connWiFi != NULL) && connWiFi->IsConnected())
    {
        // NOTE: may not need to udp.begin() if we're
        // acting only as a client?
        if(s_cfgdat != NULL)
        {
            // configured as a server, init the UDP...
            s_cfgdat->getServerCfg("udp", udpServer);
            if(udp.begin(udpServer.port)) success = true;
        } 
        else
        {
            // configured as a client...
            if(c_cfgdat != NULL)
            {
                // use the default UDP endpoint, the client
                // can specify a different one via sendUDP()
                if(c_cfgdat->getServer("udp1", udpClient)) success = true;
            }
        }
    }
    if(!checkDebugMute()) Serial.println("initUDP() - success = " + String(success));

    if(success) iRet = UDP_PAYLOAD_SIZE;
    else iRet = 0;

    return iRet;
}

void setUDP(String ip, int port)
{
    if(!checkDebugMute()) Serial.println("setUDP() - ip = " + ip + "  port = " + String(port));

    udpClient.label = "udpx";
    udpClient.addr = ip;
    udpClient.ipaddr.fromString(ip); 
    udpClient.port = port;
}

void beginUDP(int port)
{
    udp.begin(port);
}

/*
    Send a UDP packet...
*/
int sendUDP(char *payload, int len, char *endpoint/* = NULL*/)
{
int iRet = 0;

    // if a endpoint has been provided then get its
    // config data...
    if(endpoint != NULL)
    {
        // we should be configured as a client...
        if(c_cfgdat != NULL)
        {
            // use the chosen UDP endpoint...
            c_cfgdat->getServer(endpoint, udpClient);
        }
    }

    if(!checkDebugMute()) Serial.println("sendUDP() - len = " + String(len));

    // set the entire write buffer contents to 0
    memset(writeBuffer, 0, UDP_PAYLOAD_SIZE_WRITE);

    // if the length of payload is valid then
    // assemble the UDP packet...
    if((len < UDP_PAYLOAD_SIZE_WRITE) && (len > 0))
    {
        // copy the payload into the write buffer
        memcpy(writeBuffer, payload, len);

        // "begin" the UDP packet...
        udp.beginPacket(udpClient.ipaddr, udpClient.port);
    
        // write & send the UDP packet...
        iRet = udp.write(writeBuffer, len);

        if(!checkDebugMute()) Serial.println("sendUDP("+String(iRet)+") - sending to " + udpClient.addr + ":" + udpClient.port);
    
        // finish & send the packet
        if(udp.endPacket() == 0) iRet = -1;

    } else memset(writeBuffer, 0, UDP_PAYLOAD_SIZE_WRITE);

    return iRet;
}


/*
    Reply with a UDP packet...
*/
int replyUDP(char *payload, int len)
{
int iRet = 0;

    if(!checkDebugMute()) Serial.println("replyUDP() - len = " + String(len));

    // set the entire write buffer contents to 0
    memset(writeBuffer, 0, UDP_PAYLOAD_SIZE_WRITE);

    // if the length of payload is valid then
    // assemble the UDP packet...
    if((len < UDP_PAYLOAD_SIZE_WRITE) && (len > 0))
    {
        // copy the payload into the write buffer
        memcpy(writeBuffer, payload, len);

        // "begin" the UDP packet...
        udp.beginPacket(udp.remoteIP(), udp.remotePort());
    
        // write & send the UDP packet...
        iRet = udp.write(writeBuffer, len);

        if(!checkDebugMute()) Serial.println("replyUDP("+String(iRet)+") - reply to " + IPAddress(udp.remoteIP()).toString() + ":" + udp.remotePort());
    
        // finish & send the packet
        if(udp.endPacket() == 0) iRet = -1;
    }
    return iRet;
}

/*
    If a UDP packet was received then read it and place it
    in the read buffer & return the length of the data read.
*/
int recvUDP()
{
int packetLen;
int readLen = 0;

    // begins processing a packet if one is available
    if(packetLen = udp.parsePacket())
    {
        // clear the read buffer
        memset(readBuffer, 0, UDP_PAYLOAD_SIZE_READ);

        if(packetLen < UDP_PAYLOAD_SIZE)
        {
            // read the payload
            readLen = udp.read(readBuffer, packetLen);
        } else readLen = packetLen;
    }
// to noisy...
//    if(!checkDebugMute()) Serial.println("recvUDP() - packetLen = " + String(packetLen) + "  readLen = " + readLen);
    return readLen;
}

/*
    Send a payload to a multi-cast address
*/
int multiUDP(char *payload, int len)
{
mcastcfg cfg;

    if(m_cfgdat->getCfg(cfg))
    {
        udp.beginPacketMulticast(cfg.ipaddr, cfg.port, WiFi.localIP());
        udp.write(payload, len);
        udp.endPacket();
    }
}

#ifdef __cplusplus
}
#endif

