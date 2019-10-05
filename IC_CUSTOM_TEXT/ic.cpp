//
// Created by ashcon on 10/5/19.
//

#include "ic.h"
#include "mcp2515.h"

IC_DISPLAY::IC_DISPLAY(int pin, const CAN_SPEED spd) : c(){
    this->c = can_frame();
}

can_frame IC_DISPLAY::sendHeaderText(const char text[3], MCP2515 mcp) {
    int checkSumTotal = 0;
    for (int i = 0; i < 3; i++) {
        checkSumTotal += text[i];
    }
    int checkSumBit = 407 - checkSumTotal;
    c.can_id = IC_SEND_PIC;
    c.can_dlc = 8;
    c.data[0] = 0x10;
    c.data[1] = 0x09;
    c.data[2] = 0x03;
    c.data[3] = 0x29;
    c.data[4] = 0x00;
    c.data[5] = text[0];
    c.data[6] = text[1];
    c.data[7] = text[2];
    mcp.sendMessage(&c);

    c.can_id = IC_SEND_PIC;
    c.can_dlc = 8;
    c.data[0] = 0x21;
    c.data[1] = 0x20;
    c.data[2] = 0x00;
    c.data[3] = checkSumBit;
    c.data[4] = 0x05;
    c.data[5] = 0x26;
    c.data[6] = 0x01;
    c.data[7] = 0x00;
    mcp.sendMessage(&c);
    return c;

}
/*
can_frame IC_DISPLAY::read_ic_response() {
    int attempts = 0;
    while(c.can_id != IC_RECV_PIC || attempts < 100) {
        this->readMessage(&c);
        attempts++;
    }
    return c;
}
*/
