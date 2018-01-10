/* ************************************************************************ */
/*
   esp8266-ota.h - support for OTA updates

*/
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

extern void initOTA();
extern bool waitForOTA();

#ifdef __cplusplus
}
#endif
