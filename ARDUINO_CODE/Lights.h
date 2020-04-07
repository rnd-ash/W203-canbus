#include "can_comm.h"


class LIGHT_CONTROLS {
    public:
        enum INDICATOR {
            LEFT = 0x80,
            RIGHT = 0x40
        };
        LIGHT_CONTROLS(CANBUS_COMMUNICATOR *c);
        void flash_lights(bool dipped, bool fog, int duration_msec);
        void flash_indicator(INDICATOR i, int duration_msec);
        void flash_hazard(int duration_msec);
    private:
        CANBUS_COMMUNICATOR *canbus;
        can_frame x;

};