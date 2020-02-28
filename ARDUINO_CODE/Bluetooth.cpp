#include "Bluetooth.h"


bool BLUETOOTH::isConnected = false;

#ifdef ARDUINO_MEGA
BLUETOOTH::BLUETOOTH() {
    Serial1.begin(9600);
    Serial1.clearWriteError();
}

char* BLUETOOTH::read_message() {
    memset(buffer, 0x00, sizeof(buffer));
    if (Serial1.available()) {
        uint8_t len = Serial1.read();
        Serial1.readBytes(buffer, len);
        this->isConnected = true;
        DPRINTLN(BT_REC_STR+String(buffer));
    }
    return buffer;
}

void BLUETOOTH::write_message(const char* msg, uint8_t len = 0) {
    for(uint8_t i = 0; i < len; i++) {
        Serial1.write(msg[i]);
    }
}
#else
BLUETOOTH::BLUETOOTH(uint8_t rx, uint8_t tx) {
    bt = new SoftwareSerial(tx, rx);
    bt->begin(9600);
    bt->clearWriteError();
}

char* BLUETOOTH::read_message() {
    memset(buffer, 0x00, sizeof(buffer));
    if (bt->available()) {
        this->isConnected = true;
        uint8_t len = bt->read();
        bt->readBytes(buffer, len);
        DPRINTLN(BT_REC_STR+String(buffer));
    }
    return buffer;
}

void BLUETOOTH::write_message(const char* msg, uint8_t len = 0) {
    for(uint8_t i = 0; i < len; i++) {
        bt->write(msg[i]);
    }
    bt->write('\r');
    bt->flush();
}
#endif
