//
// Created by ashcon on 23/10/19.
//

#ifndef W203_CANBUS_WHEELCONTROL_H
#define W203_CANBUS_WHEELCONTROL_H

#include "mcp2515.h"
#include "can.h"
class wheelControls {
public:
    struct keyPress {
        byte volUp;
        byte volDown;
        byte PhoneUp;
        byte phoneDown;
        byte pageUp;
        byte pageDown;
    };
    enum key {
        None = 0x00,
        ArrowUp = 0x01, 
        ArrowDown = 0x02, 
        TelUp = 0x40, 
        TelDown = 0x80, 
        VolUp = 0x10, 
        VolDown = 0x20
    };
    wheelControls();
    key getPressed(MCP2515 *m);
private:
    can_frame readFrame;
    can_frame lastFrame;
};
#endif
