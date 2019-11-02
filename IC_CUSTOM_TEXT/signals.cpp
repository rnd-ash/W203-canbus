#include "signals.h"

SignalControls::SignalControls(CanbusComm *c) {
    this->c = c;
    this->f.can_dlc = 0x00;
    this->f.can_id = 0x0E;
    this->lastTime = millis();
    this->intervalMS = 500;
}

void SignalControls::enableRightIndicator() {
    disableAll();
    f.can_dlc = 0x02;
    f.data[0] = 0x80;
    f.data[1] = 0x22;
}

void SignalControls::enableLeftIndicator() {
    disableAll();
    f.can_dlc = 0x02;
    f.data[0] = 0x40;
    f.data[1] = 0x22;
}

void SignalControls::enableHazards() {
    disableAll();
    f.can_dlc = 0x02;
    f.data[0] = 0xE0;
    f.data[1] = 0x22;
}

void SignalControls::update() {
    if (millis() - lastTime > this->intervalMS) {
        lastTime = millis();
        if (f.can_dlc != 0x00) {
            if(!c->sendFrame(&f)) {
                Serial.println("Error sending signal frame: "+c->frameToString(f));
            }
        }
    }
}
void SignalControls::disableAll() {
    f.can_dlc = 0x00;
}

