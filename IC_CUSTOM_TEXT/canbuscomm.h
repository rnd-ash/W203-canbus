#ifndef W203_CANBUS_CANBUSCOMM_H
#define W203_CANBUS_CANBUSCOMM_H

#include "mcp2515.h"
#include "can.h"

class CanbusComm {
public:
    static String frameToString(can_frame frame);
    CanbusComm(MCP2515 *mcp);
    bool sendFrame(can_frame *f);
    can_frame readFrameWithID(int id, int maxTimeMillis);
private:
    can_frame read_frame;
    MCP2515 *m;
};
#endif