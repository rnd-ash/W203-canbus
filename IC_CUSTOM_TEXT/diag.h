
#include "canbuscomm.h"


class DIAG_DISPLAY {
public:
    int screens;
    String getSpeed();
    String getCoolantTemp();
    String getRPM();
    DIAG_DISPLAY(CanbusComm *c);
private:
    byte frameData[64];
    CanbusComm *can;
    int spd = 0;
    int rpm = 0;
    int temp = 0;
};
