#include "can_comm.h"

CANBUS_COMMUNICATOR::CANBUS_COMMUNICATOR(uint8_t cs_pin, CAN_SPEED speed, CAN_CLOCK clock, CAN_NET c) {
    this->mcp = new MCP2515(cs_pin);
    this->mcp->reset();
    this->mcp->setBitrate(speed, clock);
    this->mcp->setNormalMode();
    frame_string.reserve(140);
    this->pin = cs_pin;
    this->busID = c == CAN_C ? 'C' : 'B';
}

CANBUS_COMMUNICATOR::CANBUS_COMMUNICATOR(uint8_t cs_pin, CAN_SPEED speed, CAN_NET c) {
    this->mcp = new MCP2515(cs_pin);
    this->mcp->reset();
    this->mcp->setBitrate(speed);
    this->mcp->setNormalMode();
    frame_string.reserve(140);
    this->pin = cs_pin;
    this->busID = c == CAN_C ? 'C' : 'B';
}

void CANBUS_COMMUNICATOR::sendToBus(can_frame *send) {
    digitalWrite(this->pin, HIGH);
    MCP2515::ERROR x = mcp->sendMessage(send);
}

can_frame *CANBUS_COMMUNICATOR::read_frame() {
    digitalWrite(this->pin, LOW);
    // Setup default Error frame
    read.can_id = 0x00;
    read.can_dlc = 0x00;

    // Try and read a frame from Bus. If frame cannot be read, then the read frame
    // Retains its data
    mcp->readMessage(&read);
    digitalWrite(this->pin, HIGH);
    return &read;
}

char idBuffer[7];
char hexBuffer[4];

String *CANBUS_COMMUNICATOR::frame_to_string(can_frame *f, bool includeBinary) {
    frame_string = this->busID;
    frame_string += F("FRAME ID: ");
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


void CANBUS_COMMUNICATOR::printFrame(can_frame *f) {
    frame_string = "";
    frame_string += millis();
    frame_string += ",";
    frame_string += f->can_id;
    frame_string += ",";
    frame_string += f->can_dlc;
    frame_string += ",";
    for (uint8_t i = 0; i < f->can_dlc; i++) {
        frame_string += f->data[i];
        frame_string += ",";
    }
    frame_string = frame_string.substring(0, frame_string.length() - 1);
    Serial.println(frame_string);
}

void CANBUS_COMMUNICATOR::sendSerialFrame() {
    // Input type for CAN FRAME
    // Byte 0-1 -> ID
    // Byte   2 -> DLC
    // Byte 3-10 -> DATA
    if (Serial.available() == 11) {
        can_frame f;
        uint8_t payload[11];
        Serial.readBytes(payload, 11);
        f.can_id = payload[0] << 8 | payload[1];
        f.can_dlc = payload[2];
        for (uint8_t i = 3; i < 11; i++) {
            f.data[i-3] = payload[i];
        }
        sendToBus(&f);
        Serial.println("SENT");
    }
}