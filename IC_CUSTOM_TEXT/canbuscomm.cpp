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
CanbusComm::CanbusComm(int pinCanB, int pinCanC) {
    SPI.begin();
    // Can B MCP2515
    this->canB = new MCP2515(pinCanB);
    this->setCanB();
    this->canB->setBitrate(CAN_83K3BPS);
    this->canB->setNormalMode();
    this->canBPin = pinCanB;

    
    // Can C MCP2515
    this->canC = new MCP2515(pinCanC);
    this->setCanC();
    this->canC->setBitrate(CAN_500KBPS, MCP_8MHZ);
    this->canC->setNormalMode();
    this->canCPin = pinCanC;
}

/**
 * Attempts to send a [can_frame] to the car. This function will try
 * 50 times to send the frame.
 * @param can_frame Can frame to attempt to send
 * 
 * @return True if frame was sent OK, False if frame was not sent
 */
bool CanbusComm::sendFrame(int canDevice, can_frame *f) {
    int ledPin = 0;
    switch (canDevice)
    {
    case CAN_BUS_B:
        setCanB();
        ledPin = CAN_B_TX_LED;
        break;
    case CAN_BUS_C:
        setCanC();
        ledPin = CAN_C_TX_LED;
        break;
    default:
        return false;
    }
    uint8_t attempts = 0;
    digitalWrite(ledPin, HIGH);
    while (attempts <= 50) {
        if(this->currentCan->sendMessage(f) == MCP2515::ERROR_OK) {
            digitalWrite(ledPin, LOW);
            return true;
        } else {
            attempts++;
        }
    }
    digitalWrite(ledPin, LOW);
    return false;
}

can_frame CanbusComm::readFrameWithID(int canDevice, int id, int maxTimeMillis) {
    int ledPin = 0;
    can_frame error;
    error.can_id = 0x00;
    error.can_dlc = 0x00;
    switch (canDevice)
    {
    case CAN_BUS_B:
        setCanB();
        ledPin = CAN_B_RX_LED;
        break;
    case CAN_BUS_C:
        setCanC();
        ledPin = CAN_C_RX_LED;
        break;
    default:
        return error;
    }

    unsigned long startTime = millis();
    while(millis() - startTime < maxTimeMillis) {
        digitalWrite(ledPin, HIGH);
        uint8_t res = this->currentCan->readMessage(&read_frame);
        if (res == MCP2515::ERROR_OK || res == MCP2515::ERROR_NOMSG) {
            if (this->read_frame.can_id == id) {
                digitalWrite(ledPin, LOW);
                return read_frame;
            }
        } else {
            digitalWrite(ledPin, LOW);
        }
    }
    digitalWrite(ledPin, LOW);
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

void CanbusComm::setCanB() {
    currentCan = canB;
    canC->endSPI();
    canB->startSPI();
}

void CanbusComm::setCanC() {
    currentCan = canC;
    canB->endSPI();
    canC->startSPI();
}