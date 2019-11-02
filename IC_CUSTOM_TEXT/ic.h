//
// Created by ashcon on 10/5/19.
//

#ifndef W203_CANBUS_IC_H
#define W203_CANBUS_IC_H

#include "canbuscomm.h"

#define IC_SEND_PID 0x1A4
#define MAX_STR_LENGTH 9

class IC_DISPLAY {
public:
    enum clusterPage {
        Unknown = 0x00,
        Audio = 0x03,
        Telephone = 0x05
    };
    unsigned long lastTime;
    int refreshIntervalMS;
    static clusterPage currentPage;
    IC_DISPLAY(CanbusComm *c);
    String textToDisplay;
    void setBodyText(String text);
    void update();
private:
    void sendBody(String text);
    void sendHeader(const char text[3]);
    String shiftString();
    bool needsRotation;
    String currText;
    bool sendFirst;
    uint8_t calculateHeaderCheckSum(const char text[3]);
    uint8_t calculateBodyCheckSum(String text);
    CanbusComm *c;
    can_frame curr_frame;
};


#endif //W203_CANBUS_IC_H