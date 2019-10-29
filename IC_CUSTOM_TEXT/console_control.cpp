#include "console_control.h"

bool centerConsole::sendFrame(can_frame *frame, MCP2515 *m) {
  int attempts = 0;
  while(m->sendMessage(frame) != MCP2515::ERROR_OK) {
    if (attempts == 20) {
      return false;
    }
    attempts++;
  }
  return true;
}

void centerConsole::lockDoors(MCP2515* m) {
    Serial.println("Locking doors");
    can_frame f;
    f.can_id = 0x002C;
    f.can_dlc = 0x03;
    f.data[0] = 0x80;
    f.data[1] = 0x00;
    f.data[2] = 0x00;
    if (!sendFrame(&f, m)) {
        Serial.println("Error sending lock doors command");
    }
}

void centerConsole::unlockDoors(MCP2515* m) {
    Serial.println("Unlocking doors");
    can_frame f;
    f.can_id = 0x002C;
    f.can_dlc = 0x03;
    f.data[0] = 0x40;
    f.data[1] = 0x00;
    f.data[2] = 0x00;
    if (!sendFrame(&f, m)) {
        Serial.println("Error sending unlock doors command");
    }
}

void centerConsole::retractHeadRest(MCP2515* m) {
    Serial.println("Lowering head rests");
    can_frame f;
    f.can_id = 0x002C;
    f.can_dlc = 0x03;
    f.data[0] = 0x08;
    f.data[1] = 0x00;
    f.data[2] = 0x00;
    if (!sendFrame(&f, m)) {
        Serial.println("Error sending lower head rests command");
    }
}

void centerConsole::toggleESP(MCP2515* m) {
    Serial.println("Toggling ESP");
    can_frame f;
    f.can_id = 0x002C;
    f.can_dlc = 0x03;
    f.data[0] = 0x02;
    f.data[1] = 0x00;
    f.data[2] = 0x00;
    if (!sendFrame(&f, m)) {
        Serial.println("Error sending ESP off command");
    }
}
