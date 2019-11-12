//
// Created by ashcon on 10/5/19.
//

#ifndef W203_CANBUS_IC_H
#define W203_CANBUS_IC_H

#include "canbuscomm.h"

#define IC_SEND_PID 0x1A4
#define MAX_STR_LENGTH 8

class IC_DISPLAY {
public:
    enum clusterPage {
        Unknown = 0x00,
        Audio = 0x03,
        Telephone = 0x05
    };
    unsigned long lastTime;
    void setRefreshRate(int rate);
    static clusterPage currentPage;
    IC_DISPLAY(CanbusComm *c);
    String textToDisplay;
    void setBodyText(String text);
    void update();
    String diagData;
    bool inDiagMode;
private:
    String resize(String s, int lb, int ub);
    void sendBody(String text);
    void sendHeader(String header);
    String shiftString();
    String currText;
    bool sendFirst;
    int staticRefreshRate;
    int scrollRefreshRate;
    int currentRefreshRate;
    uint8_t calculateHeaderCheckSum(const char text[3]);
    uint8_t calculateBodyCheckSum(String text);
    CanbusComm *c;
    can_frame curr_frame;
};


#endif //W203_CANBUS_IC_H