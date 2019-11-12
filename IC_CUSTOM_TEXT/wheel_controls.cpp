//
// Created by ashcon on 23/10/19.
//


#include "wheel_controls.h"
#include "debug.h"

wheelControls::wheelControls(CanbusComm *c) {
    this->c = c;
}

wheelControls::key wheelControls::getPressed() {
    can_frame r = this->c->readFrameWithID(CAN_BUS_B, 0x1CA, 10);
    bool detect = r.can_id == 0x01CA;
    if (detect) {
        readFrame = r;
        if (readFrame.data[1] == lastFrame.data[1]) {
            return None;
        }
        lastFrame = readFrame;
        // No key press
        if (readFrame.data[1] == 0x00) {
            return None;
        }
        setCurrentPage();
        // First byte of this packet indicates which page the user is in on the IC cluster
        // 0x03 -> Audio page
        // 0x05 -> Telephone page
        if (readFrame.data[0] != 0x03) {
            Serial.println("User is not in audio page!");
            return None;
        } else {
            switch (readFrame.data[1])
            {
            case 0x10:
                DPRINTLN("Vol UP Pressed");
                return VolUp;
            case 0x20:
                DPRINTLN("Vol Down pressed");
                return VolDown;
            case 0x40:
                DPRINTLN("Phone Answer pressed");
                return TelUp;
            case 0x80:
                DPRINTLN("Phone Declined pressed");
                return TelDown;
            case 0x01:
                DPRINTLN("Page Up pressed");
                return ArrowUp;
            case 0x02:
                DPRINTLN("Page Down pressed");
                return ArrowDown;
            default:
                DPRINTLN("Unidentified keypress!");
                return None;
            }
        }
    }
    return None;
}

void wheelControls::setCurrentPage() {
    switch (readFrame.data[0])
    {
    case 0x03:
        IC_DISPLAY::currentPage = IC_DISPLAY::clusterPage::Audio;
        break;
    case 0x05:
        IC_DISPLAY::currentPage = IC_DISPLAY::clusterPage::Telephone;
        break;
    default:
        IC_DISPLAY::currentPage = IC_DISPLAY::clusterPage::Unknown;
        break;
    }
}
