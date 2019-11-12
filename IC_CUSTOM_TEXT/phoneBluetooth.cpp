#include "phoneBluetooth.h"
#include "debug.h"

phoneBluetooth::phoneBluetooth(int rxPin, int txPin) {
    this->bluetooth = new SoftwareSerial(rxPin, txPin);
    bluetooth->begin(9600);
    bluetooth->clearWriteError();
    // Clear Serial buffer 
    while(bluetooth->available() > 0) {
        bluetooth->read();
    }
}

void phoneBluetooth::writeMessage(String msg) {
    digitalWrite(15, HIGH);
    DPRINTLN("Sending '"+msg+"' to BT");
    for(char i : msg) {
        bluetooth->write((byte) i);
    }
    bluetooth->write('\r');
    bluetooth->flush();
    digitalWrite(15, LOW);
}

String phoneBluetooth::readMessage() {
    String msg = "";
    bool isCompleteString = false;
    if (bluetooth->available()) {
        digitalWrite(14, HIGH);
        uint8_t buffer[128];
        bluetooth->readBytes(buffer, 128);
        digitalWrite(14, LOW);
        uint8_t length = buffer[0];
        if (length > 0) {
            String ret = "";
            for (int i = 1; i < length+1; i++) {
                ret += (char) buffer[i];
            }
            return ret;
        } else {
            return "";
        }
    }
    return "";
}
