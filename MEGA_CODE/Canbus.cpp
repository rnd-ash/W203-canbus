#include "Canbus.h"
#include "helper.h"

char* Canbus::buffer;

Canbus::Canbus(uint8_t cspin, CAN_SPEED bitRate, CAN_CLOCK oscFreq) {
    this->mcp = new MCP2515(cspin);
    this->mcp->reset();
    this->mcp->setBitrate(bitRate, oscFreq);
    this->mcp->setNormalMode();
    LOG_INFO(F_TC("Canbus ready!. CS Pin is %d\n"), cspin);
}

Canbus::~Canbus() {
    delete this->mcp;
}

bool Canbus::sendMessage(can_frame *f) {
    return mcp->sendMessage(f) == MCP2515::ERROR_OK;
}

bool Canbus::readMessage(can_frame *r) {
    return mcp->readMessage(r) == MCP2515::ERROR_OK;
}

const char* Canbus::frameToString(can_frame *f) {
    delete[] buffer;
    int counter = 0;
    // 3 chars per byte in frame (+1 space per byte)
    // 6 chars for can id
    // 7 chars for 'BYTES: '
    // 5 chars for ' ID: '
    // 1 char for null termination
    // 8 char for ASCII (1 char per byte)
    // 2 spaces
    // Total = 29
    buffer = new char[(f->can_dlc*3)+19+10];
    counter += sprintf(buffer, "ID: 0x%04X BYTES: ", f->can_id);
    for (int i = 0; i < f->can_dlc; i++) {
        counter += sprintf(&buffer[counter], "%02X ", f->data[i]);
    }
    counter += sprintf(&buffer[counter], "  ");
    for (int i = 0; i < f->can_dlc; i++) {
        if (f->data[i] >= 20 && f->data[i] < 127) { // check if valid ascii
            counter += sprintf(&buffer[counter], "%c", (char)f->data[i]);
        } else {
            counter += sprintf(&buffer[counter], "."); // This means invalid ascii
        }
    }
    return buffer;
}

// Just to make compiler happy - Actual setting is done in ino file
Canbus* canB = nullptr;
Canbus* canC = nullptr;