#include "EngineData.h"
#include "HardwareSerial.h"
#include "wiring_private.h"

EngineData::EngineData() {
    this->lastTime = millis();
}

char* EngineData::getRpm() {
    memset(dataStr, 0x00, sizeof(dataStr));
    if (!isOn) {
        strncpy(dataStr, "IGN. OFF!", 9);
    } else {
        snprintf(dataStr, 9, "%d RPM", ((int)this->rpm/10)*10);
    }
    return dataStr;
}

char* EngineData::getSpeed() {
    memset(dataStr, 0x00, sizeof(dataStr));
    if (!isOn) {
        strncpy(dataStr, "IGN. OFF!", 9);
    } else {
        snprintf(dataStr, 9, "%d MPH", (int) this->speed);
    }
    return dataStr;
}

char* EngineData::getCoolantTemp() {
    memset(dataStr, 0x00, sizeof(dataStr));
    if (!isOn) {
        strncpy(dataStr, "IGN. OFF!", 9);
    } else {
        snprintf(dataStr, 9, "%d*C", this->coolantTemp);
    }
    return dataStr;
}

void EngineData::calculateBHP() {
    if (millis() - this->lastTime >= 200 && speed - lastSpd > 0.0) {
        // Velocity change in M/s divided by Time taken in Seconds
        float dv = ((speed - lastSpd) * 0.44704) / ((millis() - lastTime) / 1000.0);
        // Divide by 745.7 to go from Newtons to HP
        this->bhp = (dv * WEIGHT_KG) / 745.7;
        lastTime = millis();
        lastSpd = speed;
    }
}

char* EngineData::getBhp() {
    calculateBHP();
    memset(dataStr, 0x00, sizeof(dataStr));
    if (!isOn) {
        strncpy(dataStr, "IGN. OFF!", 9);
    } else if (speed == 0) {
        strncpy(dataStr, "CAR IDLE", 9);
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
    calculateBHP();
    this->torque = ((5252 * this->bhp) / this->rpm) / 0.73756;
    memset(dataStr, 0x00, sizeof(dataStr));
    if (!isOn) {
        strncpy(dataStr, "IGN. OFF!", 9);
    } else if (speed == 0) {
        strncpy(dataStr, "CAR IDLE", 9);
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