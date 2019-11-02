#include "canbuscomm.h"

/**
 * A class that contains a couple functions for 
 * communicating to the car, built on the MCP2515 library
 * 
 * Also sets the LED state pins to blink based on activity
 * (Yellow LED Rx from car, Red LED Tx to car)
 * 
 * @author Ashcon Mohseninia
 */
CanbusComm::CanbusComm(MCP2515 *mcp) {
    this->m = mcp;
}

/**
 * Attempts to send a [can_frame] to the car. This function will try
 * 50 times to send the frame.
 * @param can_frame Can frame to attempt to send
 * 
 * @return True if frame was sent OK, False if frame was not sent
 */
bool CanbusComm::sendFrame(can_frame *f) {
    uint8_t attempts = 0;
    digitalWrite(16, HIGH);
    while (attempts <= 50) {
        if(this->m->sendMessage(f) == MCP2515::ERROR_OK) {
            digitalWrite(16, LOW);
            return true;
        } else {
            attempts++;
        }
    }
    digitalWrite(16, LOW);
    return false;
}

can_frame CanbusComm::readFrameWithID(int id, int maxTimeMillis) {
    unsigned long startTime = millis();
    while(millis() - startTime < maxTimeMillis) {
        if (this->m->readMessage(&read_frame) == MCP2515::ERROR_OK) {
            digitalWrite(17, HIGH);
            if (this->read_frame.can_id == id) {
                digitalWrite(17, LOW);
                return read_frame;
            }
        } else {
            digitalWrite(17, LOW);
        }
    }
    can_frame error;
    error.can_id = 0x00;
    error.can_dlc = 0x00;
    digitalWrite(17, LOW);
    return error;
}

String CanbusComm::frameToString(can_frame frame) {
    String msg = "FRAME ID: ";
    char buffer[7];
    sprintf(buffer,"0x%04X", frame.can_id);
    msg += buffer;
    msg += " BYTES: ";
    char tmp[2];
    for (int k = 0; k < frame.can_dlc; k++)  {  // print the data
        char buffer[5];
        sprintf(buffer,"%02X", frame.data[k]);
        msg += buffer;
        msg += " ";
    }
    msg += " CHARS: ";
    for (int k = 0; k < frame.can_dlc; k++)  {  // print the data
        int data = frame.data[k];
        if (data >= 32 && data <= 126) {
            msg += (char) data;
        } else {
            msg += ".";
        }
    }
    return msg;
}