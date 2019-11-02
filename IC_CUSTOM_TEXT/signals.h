
#include "canbuscomm.h"

#ifndef W203_CANBUS_SIGNALCONTROLS_H
#define W203_CANBUS_SIGNALCONTROLS_H
class SignalControls {
    public:
        int intervalMS;
        SignalControls(CanbusComm *c);
        void enableRightIndicator();
        void enableLeftIndicator();
        void enableHazards();
        void update();
        void disableAll();
    private:
        unsigned long lastTime;
        can_frame f;
        CanbusComm *c;
};
#endif