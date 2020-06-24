#include "Audio_Display.h"
#include "Bluetooth.h"

AUDIO_DISPLAY::AUDIO_DISPLAY(IC_DISPLAY *d) {
    display = d;
}

void AUDIO_DISPLAY::update() {
    if (IC_DISPLAY::current_page == IC_PAGE_AUDIO && !CAR_SLEEP) { // only update if not asleep!
        if (!isInPage) {
            display->initPage(IC_PAGE_AUDIO, this->header, headFmt, topSymbol, bottomSymbol, 1);
        }
        isInPage = true;
        if (millis() - lastUpdateHeader >= headerRefreshMsec) {
            display->setHeader(IC_PAGE_AUDIO, this->header, this->headFmt);
            lastUpdateHeader = millis();
        }

        if (scrollingRequired && millis() - lastUpdateBody >= 150) {
            display->setBody(IC_PAGE_AUDIO, this->body, this->bodyFmt);
            uint8_t len = strlen(body);
            char first = body[0];
            for (uint8_t i = 1; i < len; i++) {
                body[i-1] = body[i];
            }
            body[len-1] = first;
            lastUpdateBody = millis();
        } else if (millis() - lastUpdateBody >= bodyRefreshMsec) {
            display->setBody(IC_PAGE_AUDIO, this->body, this->bodyFmt);
            lastUpdateBody = millis();
        }
    } else {
        isInPage = false;
    }
}

void AUDIO_DISPLAY::setHeader(const char* text, uint8_t format, int refreshInterval) {
    this->headerRefreshMsec = refreshInterval;
    this->headFmt = format;
    if (strlen(text) > 0 && getCRC(text) != header_crc) {
        this->header_crc = getCRC(text);
        memset(this->header, 0x00, sizeof(this->header));
        strcpy(this->header, text);

        if (IC_DISPLAY::current_page == IC_PAGE_AUDIO) {
            display->setHeader(IC_PAGE_AUDIO, this->header, this->headFmt);
            lastUpdateHeader = millis();
        }
    }
}

void AUDIO_DISPLAY::setBody(const char* text, uint8_t format, int refreshInterval) {
    this->bodyRefreshMsec = refreshInterval;
    this->bodyFmt = format;
    if (strlen(text) > 0 && getCRC(text) != body_crc) {
        body_crc = getCRC(text);
        scrollingRequired = !display->can_fit_body_text(text);
        memset(this->body, 0x00, sizeof(this->body));
        strcpy(this->body, text);
        if (scrollingRequired) {
            uint8_t len = min(245, strlen(this->body));
            this->body[len] = ' ';
            this->body[len + 1] = ' ';
            this->body[len + 2] = ' ';
            this->body[len + 3] = ' ';
            this->bodyFmt = IC_TEXT_FMT_LEFT_JUSTIFICATION; // Override for scrolling text
        }

        if (IC_DISPLAY::current_page == IC_PAGE_AUDIO) {
            display->setBody(IC_PAGE_AUDIO, this->body, this->bodyFmt);
            lastUpdateBody = millis();
        }
    }
}


void AUDIO_DISPLAY::setSymbols(uint8_t topSymbol, uint8_t bottomSymbol) {
    if (topSymbol != this->topSymbol && bottomSymbol != this->bottomSymbol) {
        this->topSymbol = topSymbol;
        this->bottomSymbol = bottomSymbol;
        if (strlen(header) > 0) {
            display->initPage(IC_PAGE_AUDIO, this->header, headFmt, this->topSymbol, this->bottomSymbol, 1);
        } else {
            display->initPage(IC_PAGE_AUDIO, " ", headFmt, this->topSymbol, this->bottomSymbol, 1);
        }
    }
}


uint8_t AUDIO_DISPLAY::getCRC(const char* str) {
    uint8_t crc = 0xFF;
    for (uint8_t i = 0; i < strlen(str); i++) {
        crc -= i * str[i];
    }
    return crc;
}