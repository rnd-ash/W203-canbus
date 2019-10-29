#include "phoneBluetooth.h"

phoneBluetooth::phoneBluetooth(int rxPin, int txPin) {
    this->bluetooth = new SoftwareSerial(rxPin, txPin);
    bluetooth->begin(9600);
    bluetooth->clearWriteError();
    this->lastID = -1;
    this->lastMsg = "";
}

void phoneBluetooth::writeMessage(String msg) {
    Serial.println("Sending '"+msg+"' to BT");
    for(char i : msg) {
        bluetooth->write((byte) i);
    }
}

String phoneBluetooth::readMessage() {
    String msg = "";
    bool isCompleteString = false;
    if (bluetooth->available()) {
        msg = bluetooth->readStringUntil('>');
        int id = msg[0];
        if (id != lastID && msg[1] == '<') {
            lastMsg = msg;
            lastID = id;
            String returnString = msg;
            returnString.remove(0,2);
            bluetooth->write("OK");
            Serial.println("Received message: "+returnString);
            return returnString;
        } else {
            Serial.println("BT: Discarding corrupt message");
        }
    }
    return "";
}
