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
        uint8_t len = bluetooth->read();
        if (len > 0) {
            uint8_t buffer[len];
            bluetooth->readBytes(buffer, len);
            digitalWrite(14, LOW);
            String ret = "";
            for (int i = 0; i < len; i++) {
                ret += (char) buffer[i];
            }
            return ret;
        }
    }
    digitalWrite(14, LOW);
    return "";
}
