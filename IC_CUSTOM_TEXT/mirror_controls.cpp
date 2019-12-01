#include "mirror_controls.h"


Mirrors::Mirrors(CanbusComm *c) {
   this->c = c; 
}

void Mirrors::lowerPassengerMirror() {
    can_frame f;
    f.can_dlc = 1;
    f.can_id = 0x2CC;
    f.data[0] = 0x08;
    for (int i = 0; i < 150; i++) {
        c->sendFrame(CAN_BUS_B, &f);
        delay(10);
    }
}

void Mirrors::raisePassengerMIrror() {
    can_frame f;
    f.can_dlc = 1;
    f.can_id = 0x2CC;
    f.data[0] = 0x04;
    for (int i = 0; i < 150; i++) {
        c->sendFrame(CAN_BUS_B, &f);
        delay(10);
    }
}