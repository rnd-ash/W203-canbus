//
// Created by ashcon on 23/10/19.
//

#ifndef W203_CANBUS_PHONEBLUETOOTH_H
#define W203_CANBUS_PHONEBLUETOOTH_H

#include <wiring_private.h>
#include <SoftwareSerial.h>
class phoneBluetooth {
public:
    phoneBluetooth(int rxPin, int txPin);
    void writeMessage(String msg);
    String readMessage();
private:
    String lastMsg;
    int lastID;
    SoftwareSerial *bluetooth;
    String lastSeralizedMessage;
};


#endif //W203_CANBUS_ICPACKETBATCH_H
