//
// Created by ashcon on 23/10/19.
//

#ifndef W203_CANBUS_WHEELCONTROL_H
#define W203_CANBUS_WHEELCONTROL_H

#include "canbuscomm.h"
#include "ic.h"

class wheelControls {
public:
    enum key {
        None = 0x00,
        ArrowUp = 0x01, 
        ArrowDown = 0x02, 
        TelUp = 0x40, 
        TelDown = 0x80, 
        VolUp = 0x10, 
        VolDown = 0x20
    };
    wheelControls(CanbusComm *c);
    key getPressed();
private:
    void setCurrentPage();
    can_frame readFrame;
    can_frame lastFrame;
    CanbusComm *c;
};
#endif
