#include "Telephone_Display.h"


TELEPHONE_DISPLAY::TELEPHONE_DISPLAY(IC_DISPLAY *d) {
    display = d;
}

void TELEPHONE_DISPLAY::update() {
    if (IC_DISPLAY::current_page == IC_DISPLAY::TELEPHONE) {
        if (!isInPage) { // Not in page so init it now (first)
            DPRINTLN("INIT PHONE");
            display->initPage(IC_DISPLAY::TELEPHONE,
                "Long message",
                true,
                IC_DISPLAY::IC_SYMBOL::NONE,
                IC_DISPLAY::IC_SYMBOL::NONE
            );
            display->setBody(IC_DISPLAY::TELEPHONE, "Someone is calling me!", true);
            isInPage = true;
        } else {
            if (millis() - lastUpdateHeader >= UPDATE_FREQ_HEADER) {
                createHeader();
                lastUpdateHeader = millis();
            }
        }
    }   else {
        isInPage = false;
    }
}

void TELEPHONE_DISPLAY::createHeader() {
    //display->setHeader(IC_DISPLAY::TELEPHONE, "TEST PHONE", false);
}
