#ifndef CANBUS_H_
#define CANBUS_H_

#include <mcp2515.h>

class Canbus {
public:
    Canbus(uint8_t cspin, CAN_SPEED bitRate, CAN_CLOCK oscFreq);
    ~Canbus();
    bool sendMessage(can_frame *f);
    bool readMessage(can_frame *r);
    static const char* frameToString(can_frame *f);
private:
    static char* buffer;
    MCP2515 *mcp;
};

extern Canbus* canB;
extern Canbus* canC;

#endif