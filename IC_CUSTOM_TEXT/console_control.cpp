#include "console_control.h"

void centerConsole::lockDoors(MCP2515* m) {
    Serial.println("Locking doors");
    can_frame f;
    f.can_id = 0x002C;
    f.can_dlc = 0x03;
    f.data[0] = 0x80;
    f.data[1] = 0x00;
    f.data[2] = 0x00;
    m->sendMessage(&f);
}

void centerConsole::unlockDoors(MCP2515* m) {
    Serial.println("Unlocking doors");
    can_frame f;
    f.can_id = 0x002C;
    f.can_dlc = 0x03;
    f.data[0] = 0x40;
    f.data[1] = 0x00;
    f.data[2] = 0x00;
    m->sendMessage(&f);
}

void centerConsole::retractHeadRest(MCP2515* m) {
    Serial.println("Lowering head rests");
    can_frame f;
    f.can_id = 0x002C;
    f.can_dlc = 0x03;
    f.data[0] = 0x08;
    f.data[1] = 0x00;
    f.data[2] = 0x00;
    m->sendMessage(&f);
}

void centerConsole::toggleESP(MCP2515* m) {
    Serial.println("Toggling ESP");
    can_frame f;
    f.can_id = 0x002C;
    f.can_dlc = 0x03;
    f.data[0] = 0x02;
    f.data[1] = 0x00;
    f.data[2] = 0x00;
    m->sendMessage(&f);
}
