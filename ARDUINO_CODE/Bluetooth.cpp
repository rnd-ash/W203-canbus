#include "Bluetooth.h"

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
        DPRINTLN(BT_REC_STR+String(buffer));
    }
    return buffer;
}

void BLUETOOTH::write_message(const char* msg) {
    for(uint8_t i = 0; i < strlen(msg); i++) {
        Serial1.write(msg[i]);
    }
    Serial1.write('\r');
    Serial1.flush();
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
        uint8_t len = bt->read();
        bt->readBytes(buffer, len);
        DPRINTLN(BT_REC_STR+String(buffer));
    }
    return buffer;
}

void BLUETOOTH::write_message(const char* msg) {
    for(uint8_t i = 0; i < strlen(msg); i++) {
        bt->write(msg[i]);
    }
    bt->write('\r');
    bt->flush();
}
#endif
