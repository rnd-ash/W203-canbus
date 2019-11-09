#ifndef W203_CANBUS_CANBUSCOMM_H
#define W203_CANBUS_CANBUSCOMM_H

#include "mcp2515.h"
#include "can.h"

#define CAN_BUS_C 0
#define CAN_BUS_B 1

#define CAN_C_RX_LED 4
#define CAN_C_TX_LED 5
#define CAN_B_RX_LED 16
#define CAN_B_TX_LED 17

class CanbusComm {
public:
    static String frameToString(can_frame *f);
    CanbusComm(int pinCanB, int pinCanC);
    bool sendFrame(int canDevice, can_frame *f);
    can_frame readFrameWithID(int canDevice, int id, int maxTimeMillis);
private:
    void setCanB();
    void setCanC();
    can_frame read_frame;
    MCP2515 *canB;
    MCP2515 *canC;
    MCP2515 *currentCan;
    int canBPin;
    int canCPin;
};
#endif