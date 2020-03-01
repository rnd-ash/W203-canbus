#ifndef ENGINE_H

#define ENGINE_H

#include "defines.h"
#include "can_comm.h"
#include <stdio.h>



const char * const PROGMEM ENGINE_OFF = "Engine off";
const char * const PROGMEM UNKNOWN_VAL = "Unknown";

const char * const PROGMEM TORQUE_C_IDLE = "Idle";
const char * const PROGMEM TORQUE_C_SLIP = "Slipping";
const char * const PROGMEM TORQUE_C_OPEN = "Open";
const char * const PROGMEM TORQUE_C_CLOSED = "Closed";

const char * const PROGMEM GEAR_PARK = "Park";
const char * const PROGMEM GEAR_NEUTRAL = "Neutral";
const char * const PROGMEM GEAR_REVERSE = "Reverse";
const char * const PROGMEM GEAR_REVERSE_2 = "Reverse 2";

const char * const PROGMEM FUEL_CONSUMP_INF = "Inf";
const char * const PROGMEM FUEL_CONSUMP_0 = "0.00";

class ENGINE_DATA{

    public:
        ENGINE_DATA();
        void readFrame(can_frame *f);
        const char* getTransmissionTemp();
        const char* getTorqueConverterStatus();
        const char* getGearing();
        const char* getIntakeTemp();
        const char* getCoolantTemp();
        const char* getConsumption();
        const char* getOilTemp();
        const char* getMPG();
    private:
        int samples_uls = 0;
        int samples_spd = 0;
        char buffer[20] = {0x00};
        boolean engineOn = false;
        uint8_t transmission_temp = 0xFF;
        uint8_t speed_km = 0;
        uint8_t torque_converter_state = 0xFF;
        uint8_t targetGear = 0xFF;
        uint8_t actualGear = 0xFF;
        uint8_t coolant_temp = 0xFF;
        uint8_t intake_temp = 0xFF;
        uint8_t oil_temp = 0xFF;
        float engine_torque = 0xFF;
        unsigned long lastMpgTime = 0;
        long consumption = 0xFFFF;
        bool returnValidMPG = false;
};

#endif

