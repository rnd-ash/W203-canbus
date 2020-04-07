#include "Lights.h"


LIGHT_CONTROLS::LIGHT_CONTROLS(CANBUS_COMMUNICATOR *c) {
    this->canbus = c;
}

void LIGHT_CONTROLS::flash_lights(bool dipped, bool fog, int duration_msec) {
    #define FOG 0b10000000;
    #define DIPPED 0b01000000
    x.can_id = 0x0230;
    x.can_dlc = 2;
    x.data[0] = 0x00;
    if (fog) x.data[0] = x.data[0] | FOG;
    if (dipped) x.data[0] = x.data[0] | DIPPED;
    x.data[1] = (uint8_t) duration_msec / 10;
    DPRINTLN(F("DIPPED BEAM ON"));
    canbus->sendToBus(&x);
}

void LIGHT_CONTROLS::flash_indicator(INDICATOR i, int duration_msec) {
    x.can_id = 0x000E;
    x.can_dlc = 2;
    x.data[0] = i;
    x.data[1] = (uint8_t) duration_msec / 10;
    DPRINTLN(F("INDICATOR ON"));
    canbus->sendToBus(&x);
}   

void LIGHT_CONTROLS::flash_hazard(int duration_msec) {
    x.can_id = 0x000E;
    x.can_dlc = 2;
    x.data[0] = 0xE0;
    x.data[1] = (uint8_t) duration_msec / 10;
    DPRINTLN(F("HAZARDS ON"));
    canbus->sendToBus(&x);
}
