//
// Created by ashcon on 23/10/19.
//


#include "wheel_controls.h"

static wheelControls::keyPress press;

wheelControls::wheelControls() {
    press.pageDown = 0x00;
    press.pageUp = 0x00;
    press.phoneDown = 0x00;
    press.PhoneUp = 0x00;
    press.volDown = 0x00;
    press.volUp = 0x00;
}

wheelControls::key wheelControls::getPressed(MCP2515 *m) {
    bool detect = false;
    for (int i = 0; i < 4000; i++) {
        if (m->readMessage(&readFrame) == MCP2515::ERROR_OK) {
            if (readFrame.can_id == 0x1CA) {
                detect = true;
                break;
            }
        }
    }
    if (detect) {
        if (readFrame.data[1] == lastFrame.data[1]) {
            return None;
        }
        lastFrame = readFrame;
        // No key press
        if (readFrame.data[1] == 0x00) {
            return None;
        }
        if (readFrame.data[0] != 0x03) {
            Serial.println("User is not in audio page!");
            return None;
        } else {
            switch (readFrame.data[1])
            {
            case 0x10:
                Serial.println("Vol UP Pressed");
                return VolUp;
            case 0x20:
                Serial.println("Vol Down pressed");
                return VolDown;
            case 0x40:
                Serial.println("Phone Answer pressed");
                return TelUp;
            case 0x80:
                Serial.println("Phone Declined pressed");
                return TelDown;
            case 0x01:
                Serial.println("Page Up pressed");
                return ArrowUp;
            case 0x02:
                Serial.println("Page Down pressed");
                return ArrowDown;
            default:
                Serial.println("Unidentified keypress!");
                return None;
            }
        }
    }
    return None;
}
