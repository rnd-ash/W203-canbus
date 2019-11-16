//
// Created by ashcon on 23/10/19.
//

#ifndef W203_CANBUS_PHONEBLUETOOTH_H
#define W203_CANBUS_PHONEBLUETOOTH_H

#include <wiring_private.h>
#include <SoftwareSerial.h>
class phoneBluetooth {
public:
    char message[256];
    phoneBluetooth(int rxPin, int txPin);
    void writeMessage(const char msg[]);
    void readMessage();
private:
    SoftwareSerial *bluetooth;
};


#endif //W203_CANBUS_ICPACKETBATCH_H
