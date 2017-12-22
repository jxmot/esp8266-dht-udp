/* ************************************************************************ */
/*
    udp-defs.h - UDP #defines used by esp8266-udp.cpp and the client code.
*/
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// try to keep this reasonably small. 
#define UDP_PAYLOAD_SIZE 75

// The size (in bytes) of the UDP data that we're expecting 
// to exchange with the server. The +1 is a place holder 
// for a NULL if a string uses the full capacity of the
// buffer.
#define UDP_PAYLOAD_SIZE_READ (UDP_PAYLOAD_SIZE + 1)
#define UDP_PAYLOAD_SIZE_WRITE (UDP_PAYLOAD_SIZE + 1)

#ifdef __cplusplus
}
#endif

