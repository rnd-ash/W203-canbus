#include "mirror_controls.h"


Mirrors::Mirrors(CanbusComm *c) {
   this->c = c; 
}

void Mirrors::lowerMirror(uint8_t degrees, bool drivers, bool passengers) {
    can_frame f;
    f.can_dlc = 1;
    f.can_id = 0x2CC;
    if (drivers) {
        f.data[0] = 0x88;
        for (int i = 0; i < degrees * 10; i++) {
            c->sendFrame(CAN_BUS_B, &f);
            delay(10);
        }
    }
    delay(10);
    if (passengers) {
        f.data[0] = 0x08;
        for (int i = 0; i < degrees * 10; i++) {
            c->sendFrame(CAN_BUS_B, &f);
            delay(10);
        }
    }
}

void Mirrors::raiseMirror(uint8_t degrees, bool drivers, bool passengers) {
    can_frame f;
    f.can_dlc = 1;
    f.can_id = 0x2CC;
    if (drivers) {
        f.data[0] = 0x84;
        for (int i = 0; i < degrees * 10; i++) {
            c->sendFrame(CAN_BUS_B, &f);
            delay(10);
        }
    }
    delay(10);
    if (passengers) {
        f.data[0] = 0x04;
        for (int i = 0; i < degrees * 10; i++) {
            c->sendFrame(CAN_BUS_B, &f);
            delay(10);
        }
    }
}