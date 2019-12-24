#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include "SoftwareSerial.h"
#include "wiring_private.h"
#include "defines.h"

#define BT_RX_LED 14
#define BT_TX_LED 15

class BLUETOOTH {
    public:
        BLUETOOTH(uint8_t rx, uint8_t tx);
        void read_message();
        void write_message(char* msg);
        char buffer[255];
    private:
        SoftwareSerial *bt;
};

#endif