//
// Created by ashcon on 23/10/19.
//

#ifndef W203_CANBUS_CONSOLECONTROL_H
#define W203_CANBUS_CONSOLECONTROL_H

#include "mcp2515.h"
#include "can.h"
class centerConsole {
public:
    void lockDoors(MCP2515* m);
    void unlockDoors(MCP2515* m);
    void retractHeadRest(MCP2515* m);
    void toggleESP(MCP2515* m);
};
#endif
