#ifndef CANCOM_H

#define CANCOM_H

#include "can.h"
#include "mcp2515.h"
#include "wiring_private.h"

#define RX_LED 16
#define TX_LED 17

/**
 * Class to handle reading and writing to Canbus
 */
class CANBUS_COMMUNICATOR {
    public:
        /**
         * Clock default is 16Mhz
         * @param cs_pin CS Pin of MCP2515 module
         * @param speed Speed of Canbus
         */
        CANBUS_COMMUNICATOR(uint8_t cs_pin, CAN_SPEED speed);
        /**
         * @param cs_pin CS Pin of MCP2515 module
         * @param speed Speed of Canbus
         * @param clock (Default 16Mhz) Clock frequency of MCP2515 module
         */
        CANBUS_COMMUNICATOR(uint8_t cs_pin, CAN_SPEED speed, CAN_CLOCK clock);
        /**
         * Sends frame to bus
         */
        void sendToBus(can_frame *send);
        /**
         * Attempts to read a frame from canbus.
         * If no frame is found during the read cycle, then a frame with DLC of 0 is returned
         */
        can_frame *read_frame();

        /**
         * Returns a pointer to a string of the can_frame parsed.
         * 
         * @param f Can Frame to print as a String
         * @param includeBinary If true, the binary string of the frame is also included in the string
         */
        String *frame_to_string(can_frame *f, bool includeBinary);
        String frame_string;
    private:
        can_frame read;
        MCP2515 *mcp;
};
    

#endif