
#include "diag.h"
#include "debug.h"



DIAG_DISPLAY::DIAG_DISPLAY(CanbusComm *c) {
    this->can = c;
    this->screens = 3;
}

String DIAG_DISPLAY::getSpeed() {
    String ret = "0 MPH";
    can_frame f = can->readFrameWithID(CAN_BUS_B, 0x0000, 10);
    if (f.can_id == 0x0000 && f.can_dlc == 8) {
        this->spd = f.data[1] * 5 / 8;
    if (spd > 0)
        ret = String(spd) + " MPH";
    } else if (spd <= 0) {
        ret = "ENG OFF!";
    }
    return ret;
}

String DIAG_DISPLAY::getRPM() {
    String ret = "RPM = N/A";
    char buffer[9];
    can_frame f = can->readFrameWithID(CAN_BUS_B, 0x0002, 30);
    if (f.can_id == 0x0002) {
        rpm = (f.data[2] << 8) | (f.data[3]);
    }
    if (rpm != 0) {
        ret = String(rpm) + " RPM";
    }
    return ret;
}

String DIAG_DISPLAY::getCoolantTemp() {
    String ret = "E.TEMP N/A";
    char buffer[9];
    can_frame f = can->readFrameWithID(CAN_BUS_B, 0x0002, 30);
    if (f.can_id == 0x0002) {
        temp = f.data[5] - 40;
    }
    if (temp != 0) {
        ret = "E:"+String(temp) + "*C";
    }
    return ret;
}