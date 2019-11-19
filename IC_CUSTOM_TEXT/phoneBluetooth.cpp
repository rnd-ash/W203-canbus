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

void phoneBluetooth::writeMessage(const char msg[]) {
    digitalWrite(15, HIGH);
    DPRINTLN("Sending '"+String(msg)+"' to BT");
    for(uint8_t i = 0; i < strlen(msg); i++) {
        bluetooth->write(msg[i]);
    }
    bluetooth->write('\r');
    bluetooth->flush();
    digitalWrite(15, LOW);
}

void phoneBluetooth::readMessage() {
    bool isCompleteString = false;
    memset(message, 0x00, sizeof(message));
    if (bluetooth->available()) {
        digitalWrite(14, HIGH);
        uint8_t len = bluetooth->read();
        if (len > 0) {
            uint8_t buffer[len];
            bluetooth->readBytes(buffer, len);
            digitalWrite(14, LOW);
            for (int i = 0; i < len; i++) {
                this->message[i] = (char) buffer[i];
            }
            return;
        }
    }
    digitalWrite(14, LOW);
}
