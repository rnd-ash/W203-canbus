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
    this->frameString = "";
    this->frameString.reserve(64);
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
    int ledPin = 0;
    switch (canDevice)
    {
    case CAN_BUS_B:
        setCanB();
        ledPin = CAN_B_TX_LED;
        this->frameToString(f);
        DPRINTLN("Sending to CanBus B -> "+*this->frameToString(f));
        break;
    case CAN_BUS_C:
        setCanC();
        ledPin = CAN_C_TX_LED;
        DPRINTLN("Sending to CanBus C -> "+*this->frameToString(f));
        break;
    default:
        // Incorrect canDevice ID. Return false
        Serial.println(F("Invalid canDevice ID."));
        return false;
    }
    uint8_t attempts = 0;
    digitalWrite(ledPin, HIGH);
    MCP2515::ERROR res = this->currentCan->sendMessage(f);           
    digitalWrite(ledPin, LOW);
    return res == MCP2515::ERROR_OK;
}

can_frame CanbusComm::pollForFrame(byte canDevice) {
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
        Serial.println(F("Invalid canDevice ID."));
        return error;
    }
    digitalWrite(ledPin, HIGH);
    MCP2515::ERROR res = this->currentCan->readMessage(&read_frame);
    digitalWrite(ledPin, LOW);
    if (res == MCP2515::ERROR_OK) {
        return read_frame;
    } else {
        return error;
    }
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

char idBuffer[6];
char byteBuffer[2];
String* CanbusComm::frameToString(can_frame *f) {
    frameString = "";
    frameString += F("FRAME ID: ");
    sprintf(idBuffer,"0x%04X", f->can_id);
    frameString += idBuffer;
    frameString += F(" BYTES: ");
    for (uint8_t k = 0; k < f->can_dlc; k++)  {
        char byteBuffer[6];
        sprintf(byteBuffer,"%02X ", f->data[k]);
        frameString += byteBuffer;
    }
    if(f->can_dlc < 8) {
        for (uint8_t k = f->can_dlc; k < 8; k++) {
            frameString += F("   ");
        }
    }
    frameString += F(" CHARS: ");
    for (uint8_t k = 0; k < f->can_dlc; k++)  {
        uint8_t data = f->data[k];
        if (data >= 32 && data != 127) {
            frameString += (char) data;
        } else {
            frameString += F(".");
        }
    }
    return &frameString;
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