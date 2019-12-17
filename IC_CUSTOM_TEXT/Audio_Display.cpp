#include "Audio_Display.h"
#include "ic.h"


Audio_Page::Audio_Page(IC_DISPLAY *ic) {
    this->lastUpdatetime = millis();
    this->display = ic;
}


void Audio_Page::update() {
    // Copy this text into header buffer if it is empty
    if (strlen(headerText) <= 1) {
        strcpy(headerText, "BLUETOOTH");
    }
    // Stop sending useless packets if not in audio page
    if (IC_DISPLAY::page == IC_DISPLAY::DISPLAY_PAGE::AUDIO) {
        if (isInPage == false) {
            // User just moved into the Audio page. We have to set it up first with icons and header text or else the 
            // audio unit will override our display!
            DPRINTLN(F("INIT AUDIO PAGE"));
            isInPage = true;
            display->initPage(IC_DISPLAY::DISPLAY_PAGE::AUDIO, IC_DISPLAY::SYMBOL::UP_ARROW, IC_DISPLAY::SYMBOL::DOWN_ARROW, true, headerText);
        }
        if (this->isScrolling) {
            if (millis() - lastUpdatetime > SCROLL_UPDATE_FREQ) {
                lastUpdatetime = millis();
                // Copy 10 chars from the body Buffer into roatating buffer (which will be used to display a section of the longer text)
                memset(rotatingText, 0x00, sizeof(rotatingText));
                for (uint8_t i = 0; i < 10; i++) {
                    rotatingText[i] = bodyText[i];
                }
                display->setbodyText(IC_DISPLAY::DISPLAY_PAGE::AUDIO, false, rotatingText, NULL, NULL, NULL);
                // Rotate the text by 1 character
                rotateText();
            }
        } else {
            if (millis() - lastUpdatetime > STATIC_UPDATE_FREQ) {
                lastUpdatetime = millis();
                display->setbodyText(IC_DISPLAY::DISPLAY_PAGE::AUDIO, true, bodyText, NULL, NULL, NULL);
            }
        }
    } else {
        isInPage = false;
    }
}

void Audio_Page::rotateText() {
    int len = strlen(bodyText);
    char first = bodyText[0];
    for (int i = 1; i < len; i++) {
        bodyText[i-1] = bodyText[i];

    }
    bodyText[len-1] = first;
}

void Audio_Page::setText(const char* body) {
    memset(bodyText, 0x00, sizeof(bodyText));
    strcpy(bodyText, body);
    this->isScrolling = false;
    if (strlen(bodyText) > ABSOLUTE_IC_MAX_BODY_CHARS || !IC_DISPLAY::textCanFit(body)) {
        this->isScrolling = true;
        uint8_t len = strlen(bodyText);
        bodyText[len] = ' ';
        bodyText[len+1] = ' ';
        bodyText[len+2] = ' ';
        bodyText[len+3] = ' ';
    }
    lastUpdatetime = 0;
}

void Audio_Page::setSymbols(IC_DISPLAY::SYMBOL upper, IC_DISPLAY::SYMBOL lower) {
    display->setSymbols(IC_DISPLAY::DISPLAY_PAGE::AUDIO, upper, lower);
}

void Audio_Page::setHeader(const char* header) {
    display->setHeader(IC_DISPLAY::DISPLAY_PAGE::AUDIO, false, header);
}
        