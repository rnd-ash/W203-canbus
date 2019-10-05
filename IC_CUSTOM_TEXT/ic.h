//
// Created by ashcon on 10/5/19.
//

#ifndef W203_CANBUS_IC_H
#define W203_CANBUS_IC_H

#include "can.h"
#include "mcp2515.h"
#define IC_SEND_PIC 0x1A4
#define IC_RECV_PIC 0x0D0

class IC_DISPLAY {
public:
    enum RESULT{
        SEND_OK,
        SEND_FAIL
    };
    IC_DISPLAY(int pin, const CAN_SPEED spd);
    can_frame sendHeaderText(const char text[3], MCP2515 mcp);
private:
    can_frame c;
    can_frame read_ic_response();
};


#endif //W203_CANBUS_IC_H
