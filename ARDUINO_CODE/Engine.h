#ifndef ENGINE_H

#define ENGINE_H

#include "defines.h"
#include "can_comm.h"



const char * const PROGMEM ENGINE_OFF = "Engine off";
const char * const PROGMEM UNKNOWN_VAL = "Unknown";


const char * const PROGMEM TORQUE_C_SLIP = "Slipping";
const char * const PROGMEM TORQUE_C_OPEN = "Open";
const char * const PROGMEM TORQUE_C_CLOSED = "Closed";

const char * const PROGMEM GEAR_PARK = "Park";
const char * const PROGMEM GEAR_REVERSE = "Reverse";
const char * const PROGMEM GEAR_REVERSE_2 = "Reverse 2";
class ENGINE_DATA{

    public:
        ENGINE_DATA();
        void readFrame(can_frame *f);
        const char* getTransmissionTemp();
        const char* getTorqueConverterStatus();
        const char* getGearing();
    private:
        char buffer[20] = {0x00};
        boolean engineOn = false;
        uint8_t transmission_temp = 0xFF;
        uint8_t torque_converter_state = 0x00;
        uint8_t targetGear = 0x00;
        uint8_t actualGear = 0x00;
};

#endif

