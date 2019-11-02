#include "phoneBluetooth.h"

phoneBluetooth::phoneBluetooth(int rxPin, int txPin) {
    this->bluetooth = new SoftwareSerial(rxPin, txPin);
    bluetooth->begin(9600);
    bluetooth->clearWriteError();
    this->lastID = -1;
    this->lastMsg = "";
    this->lastSeralizedMessage = "";
    // Clear Serial buffer 
    while(bluetooth->available() > 0) {
        bluetooth->read();
    }
}

void phoneBluetooth::writeMessage(String msg) {
    digitalWrite(15, HIGH);
    Serial.println("Sending '"+msg+"' to BT");
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
        msg = bluetooth->readStringUntil('\r');
        int id = atoi(&msg[1]);
        if (id != lastID && id != 0 && msg[0] == '|') {
            this->writeMessage("OK");
            lastMsg = msg;
            lastID = id;
            String returnString = msg;
            if (lastSeralizedMessage != returnString) {
                this->lastSeralizedMessage = returnString;
                returnString.remove(0,3);
                Serial.println("Received message: "+returnString);
                digitalWrite(14, LOW);
                return returnString;
            } else {
                digitalWrite(14, LOW);
                return "";
            }
        } else {
            Serial.println("BT: Discarding corrupt message. Content is "+msg);
            this->writeMessage("FAIL");
        }
    }
    digitalWrite(14, LOW);
    return "";
}
