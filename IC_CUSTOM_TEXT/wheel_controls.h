//
// Created by ashcon on 23/10/19.
//

#ifndef W203_CANBUS_WHEELCONTROL_H
#define W203_CANBUS_WHEELCONTROL_H

#include "can.h"
#include "debug.h"
#include "HardwareSerial.h"

#define LONG_PRESS_TIME_MS 2000

class wheelControls {
public:
    enum key {
        None = 0x00,
        ArrowUp = 0x01,
        ArrowDown = 0x02, 
        TelUp = 0x40, 
        TelDown = 0x80, 
        VolUp = 0x10, 
        VolDown = 0x20,

        // Additional key presses for long detection
        ArrowUpLong = 0x03,
        ArrowDownLong = 0x04,
        TelUpLong = 0x41,
        TelDownLong = 0x81
    };
    wheelControls();
    key getPressed(can_frame* r);
private:
    unsigned long lastPressTime;
    key lastPress;
    bool keydown;
    void setCurrentPage();
    can_frame readFrame;
    can_frame lastFrame;
};
#endif
