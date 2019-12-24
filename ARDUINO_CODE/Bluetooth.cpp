#include "Bluetooth.h"

BLUETOOTH::BLUETOOTH(uint8_t rx, uint8_t tx) {
    bt = new SoftwareSerial(rx, tx);
    bt->begin(9600);
    bt->clearWriteError();
    pinMode(BT_RX_LED, OUTPUT);
    pinMode(BT_TX_LED, OUTPUT);
}

void BLUETOOTH::read_message() {
    memset(buffer, 0x00, sizeof(buffer));
    if (bt->available()) {
        digitalWrite(BT_RX_LED, HIGH);
        uint8_t len = bt->read();
        bt->readBytes(buffer, len);
        DPRINTLN("Bluetooth MSG: "+String(buffer));
        digitalWrite(BT_RX_LED, LOW);
    }
}

void BLUETOOTH::write_message(char* msg) {
    digitalWrite(BT_TX_LED, HIGH);
    for(uint8_t i = 0; i < strlen(msg); i++) {
        bt->write(msg[i]);
    }
    bt->write('\r');
    bt->flush();
    digitalWrite(BT_TX_LED, LOW);
}
