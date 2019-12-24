#include "can_comm.h"

CANBUS_COMMUNICATOR::CANBUS_COMMUNICATOR(uint8_t cs_pin, CAN_SPEED speed, CAN_CLOCK clock) {
    SPI.begin();
    this->mcp = new MCP2515(cs_pin);
    this->mcp->setBitrate(speed, clock);
    this->mcp->setNormalMode();
    pinMode(RX_LED, OUTPUT);
    pinMode(TX_LED, OUTPUT);
    frame_string.reserve(150);
}

CANBUS_COMMUNICATOR::CANBUS_COMMUNICATOR(uint8_t cs_pin, CAN_SPEED speed) {
    SPI.begin();
    this->mcp = new MCP2515(cs_pin);
    this->mcp->setBitrate(speed, MCP_16MHZ);
    this->mcp->setNormalMode();
    pinMode(RX_LED, OUTPUT);
    pinMode(TX_LED, OUTPUT);
    frame_string.reserve(150);
}

void CANBUS_COMMUNICATOR::sendToBus(can_frame *send) {
    digitalWrite(TX_LED, HIGH);
    mcp->sendMessage(send);
    digitalWrite(TX_LED, LOW);
}

can_frame *CANBUS_COMMUNICATOR::read_frame() {
    digitalWrite(RX_LED, HIGH);
    // Setup default Error frame
    read.can_id = 0x00;
    read.can_dlc = 0x00;

    // Try and read a frame from Bus. If frame cannot be read, then the read frame
    // Retains its data
    mcp->readMessage(&read);
    digitalWrite(RX_LED, LOW);
    return &read;
}

char idBuffer[7];
char hexBuffer[4];

String *CANBUS_COMMUNICATOR::frame_to_string(can_frame *f, bool includeBinary) {
    frame_string = F("FRAME ID: ");
    sprintf(idBuffer, "0x%04X", f->can_id);
    frame_string+=idBuffer;
    if (includeBinary) {
        frame_string +=F(" BINARY: ");
        for (uint8_t i = 0; i < f->can_dlc; i++) {
            char bits[9] = {'0', '0', '0', '0', '0', '0', '0', '0', 0x00};
            uint8_t byte = f->data[i];
            uint8_t k = 7;
            while(byte > 0) {
                bits[k] = '0' + (byte & 0x01);
                byte >>=1;
                k--;
            }
            frame_string+=bits;
            frame_string+=" ";
        }
    }
    frame_string+=F(" HEX: ");
    for (uint8_t i = 0; i < f->can_dlc; i++) {
        sprintf(hexBuffer, "%02X ", f->data[i]);
        frame_string+=hexBuffer;
    }
    frame_string+=F(" CHARS: ");
    for (uint8_t i = 0; i < f->can_dlc; i++) {
        if (f->data[i] < 32) {
            frame_string += '.';
        } else {
            frame_string +=(char) f->data[i];
        }
    }
    return &frame_string;
}