#ifndef TELEPHONE_DISPLAY_H
#define TELEPHONE_DISPLAY_H

#include "ic_display.h"

class TELEPHONE_DISPLAY {
    public:
        TELEPHONE_DISPLAY(IC_DISPLAY *d);
        void update();
    private:
        bool isInPage = true;
        IC_DISPLAY *display;
        unsigned long lastUpdateBody = 0L;
        unsigned long lastUpdateHeader = 0L;
        void createHeader();
};
#endif