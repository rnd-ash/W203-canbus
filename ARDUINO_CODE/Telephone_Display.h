#ifndef TELEPHONE_DISPLAY_H
#define TELEPHONE_DISPLAY_H

#include "ic_display.h"
#include "Bluetooth.h"

const char * const PROGMEM CARRIER_UNKNOWN = "Unk. Carrier";

class TELEPHONE_DISPLAY {
    public:
        TELEPHONE_DISPLAY(IC_DISPLAY *d, BLUETOOTH* bt);
        void update();
        char carrier[20] = {0x00};
        void setCarrier(char* c);
    private:
        bool isInPage = false;
        IC_DISPLAY *display;
        unsigned long lastUpdateBody = 0L;
        unsigned long lastUpdateHeader = 0L;
        void createHeader(const char * src);
        BLUETOOTH *bluetooth;
};
#endif