#ifndef W203_CANBUS_ENGINE_H
#define W203_CANBUS_ENGINE_H
#define WEIGHT_KG 1516
#define DRAG_COEFFICIENT 0.27
#include "debug.h"
#include <stdio.h>

// Ratio of engine turns to driveshaft turns (W211's / AMG cars may be different!)
#define GEAR_RATIO_1 3.9319
#define GEAR_RATIO_2 2.4079
#define GEAR_RATIO_3 1.4857
#define GEAR_RATIO_4 1.0
#define GEAR_RATIO_5 0.8305

// Automatic drive train loss is typically 15%
#define DRIVETRAIN_EFFICIENCY_LOSS 0.15

// 3 rotations of driveshaft to 1 rotation of wheel (W211's / AMG cars may be different!)
#define DIFF_RATIO 3.0

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
        uint8_t curr_gear = 0;
        float fuel_level_l = 66.0;
        unsigned long lastTime;
        bool isInReverse = false;
    private:
        void calculatePower();
        float bhp = 0.0;
        float torque = 0.0;
};

#endif