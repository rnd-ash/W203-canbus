#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include "SoftwareSerial.h"
#include "wiring_private.h"
#include "defines.h"

const char * const PROGMEM BT_REC_STR = "Bluetooth MSG: ";
class BLUETOOTH {
    public:
        BLUETOOTH(uint8_t rx, uint8_t tx);
        char* read_message();
        void write_message(const char* msg);
    private:
        char buffer[255];
        SoftwareSerial *bt;
};

#endif