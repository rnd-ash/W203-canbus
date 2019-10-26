//
// Created by ashcon on 10/5/19.
//

#ifndef W203_CANBUS_IC_H
#define W203_CANBUS_IC_H

#include "can.h"
#include "icPacketBatch.h"
//#include "mcp2515.h"
#define IC_SEND_PID 0x1A4
#define IC_RECV_PID 0x0D0

class IC_DISPLAY {
public:
    enum RESULT {
        SEND_OK,
        SEND_FAIL
    };
    //RESULT sendHeaderText(const char text[3], MCP2515 mcp);
    //RESULT sendBodyText(int charCount, char text[9], MCP2515 mcp);
    void createHeaderPackets(const char text[3], icPacketBatch *b);
    void createBodyPackets(String text, icPacketBatch *b);
private:
    //can_frame read_ic_response(MCP2515 mcp);
    uint8_t calculateHeaderCheckSum(const char text[3]);
    static uint8_t calculateBodyCheckSum(String text);
};


#endif //W203_CANBUS_IC_H