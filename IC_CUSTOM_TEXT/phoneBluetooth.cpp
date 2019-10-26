#include "phoneBluetooth.h"

phoneBluetooth::phoneBluetooth(int rxPin, int txPin, int ledPinRx, int ledPinTx) {
    this->bluetooth = new SoftwareSerial(rxPin, txPin);
    this->ledPinRx = ledPinRx;
    pinMode(ledPinRx, OUTPUT);
    pinMode(ledPinTx, OUTPUT);
    bluetooth->begin(9600);
    bluetooth->clearWriteError();
    this->lastID = -1;
    this->lastMsg = "";
}

void phoneBluetooth::writeMessage(String msg) {
    digitalWrite(ledPinTx, HIGH);
    for (char character : msg) {
        bluetooth->write((byte) character);
    }
    digitalWrite(ledPinTx, LOW);
}

String phoneBluetooth::readMessage() {
    String msg = "";
    bool isCompleteString = false;
    if (bluetooth->available()) {
        digitalWrite(ledPinRx, HIGH);
        msg = bluetooth->readStringUntil('>');
        digitalWrite(ledPinRx, LOW);
        int id = msg[0];
        if (id != lastID && msg[1] == '<') {
            lastMsg = msg;
            lastID = id;
            String returnString = msg;
            returnString.remove(0,2);
            bluetooth->write("OK");
            return returnString;
        } else {
            Serial.println("BT: Discarding corrupt message");
        }
    }
    return "";
}
