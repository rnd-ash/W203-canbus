#include "EngineData.h"


char* EngineData::getRpm() {
    if(!isOn) {
        return "ENGINE OFF";
    } else {
        char buffer[8];
        sprintf(buffer, "%d RPM", rpm);
        return buffer;
    }
}

char* EngineData::getSpeed() {
    char str[] = "ENGINE OFF";
    if(!isOn) {
        sprintf(str, "%d MPH", speed);
    }
    return str;
}

char* EngineData::getCoolantTemp() {
    return "C.TEMP UNK";
}