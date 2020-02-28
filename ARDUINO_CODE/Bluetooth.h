#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include "HardwareSerial.h"
#include "SoftwareSerial.h"
#include "wiring_private.h"
#include "defines.h"

const char * const PROGMEM BT_REC_STR = "Bluetooth MSG: ";
class BLUETOOTH {
    public:
        static bool isConnected;
        #ifdef ARDUINO_MEGA
        BLUETOOTH();
        #else
        BLUETOOTH(uint8_t rx, uint8_t tx);
        #endif
        char* read_message();
        void write_message(const char* msg, uint8_t len = 0);
    private:
        char buffer[255];
        #ifndef ARDUINO_MEGA
        SoftwareSerial *bt;
        #endif
};

#endif