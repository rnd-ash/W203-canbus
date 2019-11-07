#include "console_control.h"


centerConsole::centerConsole(CanbusComm *c) {
    this->c = c;
}


void centerConsole::lockDoors() {
    Serial.println("Locking doors");
    can_frame f;
    f.can_id = 0x002C;
    f.can_dlc = 0x03;
    f.data[0] = 0x80;
    f.data[1] = 0x00;
    f.data[2] = 0x00;
    if (!this->c->sendFrame(CAN_BUS_B ,&f)) {
        Serial.println("Error sending lock doors command");
    }
}

void centerConsole::unlockDoors() {
    Serial.println("Unlocking doors");
    can_frame f;
    f.can_id = 0x002C;
    f.can_dlc = 0x03;
    f.data[0] = 0x40;
    f.data[1] = 0x00;
    f.data[2] = 0x00;
    if (!this->c->sendFrame(CAN_BUS_B, &f)) {
        Serial.println("Error sending unlock doors command");
    }
}

void centerConsole::retractHeadRest() {
    Serial.println("Lowering head rests");
    can_frame f;
    f.can_id = 0x002C;
    f.can_dlc = 0x03;
    f.data[0] = 0x08;
    f.data[1] = 0x00;
    f.data[2] = 0x00;
    if (!this->c->sendFrame(CAN_BUS_B, &f)) {
        Serial.println("Error sending lower head rests command");
    }
}

void centerConsole::toggleESP() {
    Serial.println("Toggling ESP");
    can_frame f;
    f.can_id = 0x002C;
    f.can_dlc = 0x03;
    f.data[0] = 0x02;
    f.data[1] = 0x00;
    f.data[2] = 0x00;
    if (!this->c->sendFrame(CAN_BUS_B, &f)) {
        Serial.println("Error sending ESP off command");
    }
}
