#include "EngineData.h"
#include "HardwareSerial.h"
#include "wiring_private.h"

EngineData::EngineData() {
    this->lastTime = millis();

    // Set gear to gear 4 (1:1) until we can get the current gear
    this->curr_gear = 4;
}

const char * const IGN_OFF_MSG PROGMEM = "IGN. OFF!";
const char * const IDLE_MSG PROGMEM = "CAR IDLE";

char* EngineData::getRpm() {
    memset(dataStr, 0x00, sizeof(dataStr));
    if (!isOn) {
        strncpy(dataStr, IGN_OFF_MSG, 9);
    } else {
        snprintf(dataStr, 9, "%d RPM", ((int)this->rpm/10)*10);
    }
    return dataStr;
}

char* EngineData::getSpeed() {
    memset(dataStr, 0x00, sizeof(dataStr));
    if (!isOn) {
        strncpy(dataStr, IGN_OFF_MSG, 9);
    } else {
        snprintf(dataStr, 9, "%d MPH", (int) this->speed);
    }
    return dataStr;
}

char* EngineData::getCoolantTemp() {
    memset(dataStr, 0x00, sizeof(dataStr));
    if (!isOn) {
        strncpy(dataStr, IGN_OFF_MSG, 9);
    } else {
        snprintf(dataStr, 9, "%d*C", this->coolantTemp);
    }
    return dataStr;
}

/**
 * Calculates Horsepower and Torque AT THE WHEELS
 */
void EngineData::calculatePower() {
    // Run this update every 1/2 second
    if (millis() - this->lastTime >= 250 && speed - lastSpd != 0.0) {

        // Time difference in seconds
        float time_sec = (millis() - lastTime) / 1000.0;
        lastTime = millis();

        // Calculate distance traveled using average speed in m/s
        float distance_m = (((speed + lastSpd) / 2.0) * 0.44704) * time_sec;

        // Calculate acceleration in ms^2
        float accel_ms = ((speed - lastSpd) / time_sec) * 0.44704;

        // Convert engine RPM to wheel RPM
        /*
        float wheel_rpm = this->rpm / DIFF_RATIO;
        switch (curr_gear)
        {
        case 1:
            wheel_rpm /= GEAR_RATIO_1;
            break;
        case 2:
            wheel_rpm /= GEAR_RATIO_2;
            break;
        case 3:
            wheel_rpm /= GEAR_RATIO_3;
            break;
        case 4:
            wheel_rpm /= GEAR_RATIO_4;
            break;
        case 5:
            wheel_rpm /= GEAR_RATIO_5;
            break;
        default:
            break;
        }
        */

        //                accel * mass * distance
        // Power (BHP) =  ------------------------ x (Drivetrain loss + Drag)
        //                      time * 745.7
        //
        this->bhp = ((accel_ms * WEIGHT_KG * distance_m) / (time_sec * 745.7)) * (1 + DRIVETRAIN_EFFICIENCY_LOSS + DRAG_COEFFICIENT);


        //                bhp * 33000
        //  Torque(NM) =  -----------  x 1.35582
        //                 2π * rpm
        //
        this->torque = ((this->bhp * 33000) / (2 * 3.14 * this->rpm)) * 1.35582;

        lastSpd = speed;
    }
}

char* EngineData::getBhp() {
    calculatePower();
    memset(dataStr, 0x00, sizeof(dataStr));
    if (!isOn) {
        strncpy(dataStr, IGN_OFF_MSG, 9);
    } else if (speed == 0) {
        strncpy(dataStr, IDLE_MSG, 9);
    } else if (bhp < 0) {
        strncpy(dataStr, "0.0 BHP", 9);
    } else {
        if (this->bhp < 100) {
            char x[5];
            // Convert float to 3 digits, decimal point and one precision, store it in buffer x
            dtostrf(this->bhp, 4, 1, x);
            snprintf(dataStr, 9, "%s BHP", x);
        } else {
            snprintf(dataStr, 9, "%d BHP", (int) this->bhp);
        }
    }
    return dataStr;
}

char* EngineData::getTorque() {
    calculatePower();
    memset(dataStr, 0x00, sizeof(dataStr));
    if (!isOn) {
        strncpy(dataStr, IGN_OFF_MSG, 9);
    } else if (speed == 0) {
        strncpy(dataStr, IDLE_MSG, 9);
    } else if (torque < 0) {
        strncpy(dataStr, "0.0 NM", 9);
    } else {
        if (this->torque < 100) {
            char x[5];
            // Convert float to 3 digits, decimal point and one precision, store it in buffer x
            dtostrf(this->torque, 4, 1, x);
            snprintf(dataStr, 9, "%s NM", x);
        } else {
            snprintf(dataStr, 9, "%d NM", (int) this->torque);
        }
    }
    return dataStr;
}