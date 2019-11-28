//
// Created by ashcon on 23/10/19.
//


#include "wheel_controls.h"
#include "wiring_private.h"
#include "ic.h"

wheelControls::wheelControls() {
    this->keydown = false;
    this->lastPress = None;
    this->lastPressTime = millis();
    this->readFrame.can_id = 0x01CA;
    this->readFrame.can_dlc = 0x03;
    this->readFrame.data[0] = 0x00;
    this->readFrame.data[1] = 0x00;
    this->readFrame.data[2] = 0x00;
}

wheelControls::key getKey(__u8 k) {
    switch (k)
    {
    case 0x01:
        return wheelControls::key::ArrowUp;
    case 0x02:
        return wheelControls::key::ArrowDown;
    case 0x40:
        return wheelControls::key::TelUp;
    case 0x80:
        return wheelControls::key::TelDown;
    default:
        return wheelControls::key::None;
    }
}

wheelControls::key wheelControls::getPressed(can_frame* r) {
    bool detect = r->can_id == 0x01CA;
    // Detect if user is in the audio page before processing key presses
    if (detect) {
        setCurrentPage(r);
        // User has began to press a button
        if (r->data[1] != 0x00 && readFrame.data[1] == 0x00) {
            DPRINTLN(F("Key down detected"));
            this->keydown = true;
            readFrame = *r;
            lastPressTime = millis();
            this->lastPress = getKey(r->data[1]);
            return None;
        }
        // User is still holding down the key! 
        else if (r->data[1] != 0x00 && readFrame.data[1] != 0x00) {
            readFrame = *r;
            if ((millis() - lastPressTime) > LONG_PRESS_TIME_MS && this->keydown) {
                this->keydown = false;
                switch (this->lastPress)
                {
                case ArrowUp:
                    DPRINTLN(F("Long arrow up pressed!"));
                    return ArrowUpLong;
                case ArrowDown:
                    DPRINTLN(F("Long arrow down pressed!"));
                    return ArrowDownLong;
                case TelDown:
                    DPRINTLN(F("Long tel down pressed!"));
                    return TelDownLong;
                case TelUp:
                    DPRINTLN(F("Long tel up pressed!"));
                    return TelUpLong;
                default:
                    DPRINTLN(F("Unknown long keypress"));
                    return None;
                }
            }
        }
        // User has let go of the key 
        else {
            readFrame = *r;
            if (this->keydown) {
                DPRINTLN(F("Simple key tap detected"));
                this->keydown = false;
                return this->lastPress;
            }
        }
    }
    // Default return if not in Audio page or if CanFrame not found
    return None;
}

void wheelControls::setCurrentPage(can_frame* r) {
    switch (r->data[0])
    {
    case 0x03:
        IC_DISPLAY::page = IC_DISPLAY::PAGES::AUDIO;
        break;
    case 0x05:
        IC_DISPLAY::page = IC_DISPLAY::PAGES::TELEPHONE;
        break;
    default:
        IC_DISPLAY::page = IC_DISPLAY::PAGES::OTHER;
        break;
    }
}
