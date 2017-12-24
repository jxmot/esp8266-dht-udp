/* ************************************************************************ */
/*
    esp8266-udp.h - UDP support for the associated ino file.

*/
#pragma once

#include "udp-defs.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int initUDP();

extern int sendUDP(char *payload, int len, char *endpoint = NULL);
extern int replyUDP(char *payload, int len);
extern int recvUDP();

extern int multiUDP(char *payload, int len);

extern unsigned char readBuffer[];
extern unsigned char writeBuffer[];

#ifdef __cplusplus
}
#endif

