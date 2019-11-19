#ifndef W203_CANBUS_ENGINE_H
#define W203_CANBUS_ENGINE_H

#include "debug.h"
#include <stdio.h>


class EngineData {
    public:
        char* getSpeed();
        char* getRpm();
        char* getCoolantTemp();
        bool isOn = false;
        int speed = 0;
        int rpm = 0;
        int coolantTemp = 0;
};

#endif