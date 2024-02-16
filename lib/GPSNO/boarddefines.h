#include <Arduino.h>

#include "gitHeader.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define VERSION_MAJOR 0
#define VERSION_MINOR 0
#define VERSION_PATCH 1

#define VERSION_STRING TOSTRING(VERSION_MAJOR) "." TOSTRING(VERSION_MINOR) "." TOSTRING(VERSION_PATCH)

#define PIN_RST 27
#define PIN_IRQ 34
#define PIN_SS 4