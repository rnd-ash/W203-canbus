#include "canbuscomm.h"
#include "debug.h"

/**
 * A class that contains a couple functions for 
 * communicating to the car, built on the MCP2515 library
 * 
 * Also sets the LED state pins to blink based on activity
 * (Yellow LED Rx from car, Red LED Tx to car)
 * 
 * @author Ashcon Mohseninia
 * 
 * @param pinCanB CS Pin for MCP2515 module that talks to Can B
 * @param pinCanC CS Pin for MCP2515 module that talks to Can C
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
 * 
 * @param canDevice Bus number to send to (0 -> Can B, 1 -> Can C)
 * @param can_frame Can frame to attempt to send
 * 
 * @return True if frame was sent OK, False if frame was not sent
 */
bool CanbusComm::sendFrame(byte canDevice, can_frame *f) {
    #ifdef DEBUG
        char x = canDevice == CAN_BUS_B ? 'B' : 'C';
        Serial.println("Sending to bus "+String(x)+": "+this->frameToString(f));
    #endif

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
        // Incorrect canDevice ID. Return false
        Serial.println("Invalid canDevice ID.");
        return false;
    }
    uint8_t attempts = 0;
    digitalWrite(ledPin, HIGH);
    MCP2515::ERROR res = this->currentCan->sendMessage(f);           
    digitalWrite(ledPin, LOW);
    return res == MCP2515::ERROR_OK;
}

/**
 * Attempts to read a can frame from canbus.
 * 
 * If no can frame is found matching the query, a blank frame with ID if 0x00 and 0 length is returned
 * 
 * @param canDevice Bus number to read from (0 -> Can B, 1 -> Can C)
 * @param id ID of can frame to attempt to scan for
 * @param maxTimeMillis Max time allowed to attempt to locate frame
 * 
 */
can_frame CanbusComm::readFrameWithID(byte canDevice, int id, int maxTimeMillis) {
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
        // Incorrect canDevice ID. Return error
        Serial.println("Invalid canDevice ID.");
        return error;
    }

    unsigned long startTime = millis();
    // Check if maxTimeMillis has been reached yet
    while(millis() - startTime < maxTimeMillis) {
        digitalWrite(ledPin, HIGH);
        MCP2515::ERROR res = this->currentCan->readMessage(&read_frame);
        if (res == MCP2515::ERROR_OK) {
            if (this->read_frame.can_id == id) {
                digitalWrite(ledPin, LOW);
                // Found the frame we are looking for!
                return read_frame;
            }
        } else {
            digitalWrite(ledPin, LOW);
        }
    }
    // Cannot find can frame within time limit. Return error frame
    digitalWrite(ledPin, LOW);
    return error;
}

/**
 * Function to convert a can frame to String. String has the following format:
 * 
 *  'Frame ID: 0x0000 BYTES: 0x00 0x00 0x00..... CHARS: ........'
 * 
 *  Chars are rendered as ASCII characters, and if not possible, '.'
 *  Bytes are rendered as hex bytes from 0x00 to 0xFF
 *  ID is a hex byte from 0x0 to 0xFFFF
 */
String CanbusComm::frameToString(can_frame *f) {
    String msg = "FRAME ID: ";
    char buffer[7];
    sprintf(buffer,"0x%04X", f->can_id);
    msg += buffer;
    msg += " BYTES: ";
    char tmp[2];
    for (uint8_t k = 0; k < f->can_dlc; k++)  {
        char buffer[5];
        sprintf(buffer,"%02X", f->data[k]);
        msg += buffer;
        msg += " ";
    }
    msg += " CHARS: ";
    for (uint8_t k = 0; k < f->can_dlc; k++)  {
        uint8_t data = f->data[k];
        if (data >= 32 && data <= 126) {
            msg += (char) data;
        } else {
            msg += ".";
        }
    }
    return msg;
}

/**
 * Switches to Can B MCP2515 module for communication
 */
void CanbusComm::setCanB() {
    currentCan = canB;
    canC->endSPI();
    canB->startSPI();
}

/**
 * Switches to Can C MCP2515 module for communication
 */
void CanbusComm::setCanC() {
    currentCan = canC;
    canB->endSPI();
    canC->startSPI();
}