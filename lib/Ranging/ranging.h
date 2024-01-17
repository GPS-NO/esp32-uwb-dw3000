#include "SPI.h"
#include "dw3000.h"

/*
    https://github.com/gurubrahmayya/UWB/tree/ca98d8854cec5c4dacb902a89844cb877e5de1c6
*/

#pragma once
#ifndef ranging_h
#define ranging_h

int8_t ranging_init(int irq, int rst, int ss, bool isInitatot);

int8_t ranging_initator();

int8_t ranging_responder();
#endif