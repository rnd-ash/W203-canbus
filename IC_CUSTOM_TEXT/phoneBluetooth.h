//
// Created by ashcon on 23/10/19.
//

#ifndef W203_CANBUS_PHONEBLUETOOTH_H
#define W203_CANBUS_PHONEBLUETOOTH_H

#include "wiring_private.h"
#include "SoftwareSerial.h"
#include "debug.h"
class phoneBluetooth {
public:
    phoneBluetooth(int rxPin, int txPin);
    void writeMessage(const char msg[]);
    void readMessage();
    char message[128];
    #ifdef SIMULATION
        void debugInit();
    #endif

private:
    SoftwareSerial *bluetooth;
};


#endif //W203_CANBUS_ICPACKETBATCH_H
