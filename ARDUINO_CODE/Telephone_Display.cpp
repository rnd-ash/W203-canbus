#include "Telephone_Display.h"


TELEPHONE_DISPLAY::TELEPHONE_DISPLAY(IC_DISPLAY *d, BLUETOOTH* bt) {
    display = d;
    this->bluetooth = bt;
}

void TELEPHONE_DISPLAY::update() {
    if (IC_DISPLAY::current_page == IC_PAGE_TELEPHONE) {
        if (!isInPage) { // Not in page so init it now (first)
            isInPage = true;
            if (strlen(this->carrier) == 0) {
                DPRINTLN("REQUEST CARRIER");
                const char req[1] = { 0x03 };
                bluetooth->write_message(req, 1);
            } else {
                this->createHeader(carrier);
                char *text[2] = {"Test", "Phone"};
                display->setBodyTel(2,text);
            }
            lastUpdateBody = millis();
        } else {
            if (millis() - lastUpdateBody >= 1000) {
                lastUpdateBody = millis();
                if (strlen(this->carrier) == 0) {
                    DPRINTLN("REQUEST CARRIER");
                    const char req[1] = { 0x03 };
                    bluetooth->write_message(req, 1);
                } else {
                    this->createHeader(carrier);
                    char *text[2] = {"Test", "Phone"};
                    display->setBodyTel(2,text);
                }
            }
        }
    }   else {
        isInPage = false;
    }
}

void TELEPHONE_DISPLAY::setCarrier(char* c) {
    memset(carrier, 0x00, sizeof(carrier));
    strcpy(carrier, c);
    this->createHeader(carrier);
}

void TELEPHONE_DISPLAY::createHeader(const char * src) {
    if (strlen(src) > 0) {
        display->setHeader(IC_PAGE_TELEPHONE, src, IC_TEXT_FMT_LEFT_JUSTIFICATION);
    }
}
