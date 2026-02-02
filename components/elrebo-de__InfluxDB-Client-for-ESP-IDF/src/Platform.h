#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#define STRHELPER(x) #x
#define STR(x) STRHELPER(x) // stringifier

// form version string
#define VERSION_STR(MAJOR, MINOR, PATCH) STR(MAJOR) "." STR(MINOR) "." STR(PATCH)

// ESP8266 is not supported
// Platform defines are no longer needed
#define INFLUXDB_CLIENT_PLATFORM "ESP-IDF"
#define INFLUXDB_CLIENT_PLATFORM_VERSION "5.5.0"

#endif //_PLATFORM_H_