#ifndef W203_CANBUS_ENGINE_H
#define W203_CANBUS_ENGINE_H
#define WEIGHT_KG 1516
#define DRAG_COEFFICIENT 0.27
#include "debug.h"
#include <stdio.h>

// Needed for BHP / Torque calculation to get the engine outputs rather than wheel outputs
// https://en.wikipedia.org/wiki/Mercedes-Benz_5G-Tronic_transmission
#define GEAR_RATIO_1 3.9319
#define GEAR_RATIO_2 2.4079
#define GEAR_RATIO_3 1.4857
#define GEAR_RATIO_4 1.0
#define GEAR_RATIO_5 0.8305

class EngineData {
    public:
        EngineData();
        char* getBhp();
        char* getTorque();
        char* getSpeed();
        char* getRpm();
        char* getCoolantTemp();
        bool isOn = false;
        float speed = 0.0;
        int rpm = 0;
        int coolantTemp = 0;
        char dataStr[9];
        int lastSpd = 0.0;
        unsigned long lastTime;
    private:
        void calculateBHP();
        float bhp = 0.0;
        float torque = 0.0;
};

#endif