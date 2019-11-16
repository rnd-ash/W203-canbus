//
// Created by ashcon on 23/10/19.
//

#ifndef W203_CANBUS_CONSOLECONTROL_H
#define W203_CANBUS_CONSOLECONTROL_H

#include "canbuscomm.h"
class centerConsole {
public:
    centerConsole(CanbusComm *c);
    void lockDoors();
    void unlockDoors();
    void retractHeadRest();
    void toggleESP();
private:
    CanbusComm *c;
    can_frame f;
};
#endif
