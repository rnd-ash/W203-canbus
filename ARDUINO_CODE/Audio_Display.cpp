#include "Audio_Display.h"

int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

AUDIO_DISPLAY::AUDIO_DISPLAY(IC_DISPLAY *d) {
    display = d;
}

void AUDIO_DISPLAY::update() {
    if (IC_DISPLAY::current_page == IC_DISPLAY::AUDIO) {
        if (!isInPage) { // Not in page so init it now (first)
            if (inDiagMode) {
                display->initPage(IC_DISPLAY::AUDIO,
                    getDiagHeader(),
                    true,
                    IC_DISPLAY::IC_SYMBOL::UP_ARROW,
                    IC_DISPLAY::IC_SYMBOL::DOWN_ARROW
                );
            } else {
                display->initPage(IC_DISPLAY::AUDIO,
                    "",
                    true,
                    IC_DISPLAY::IC_SYMBOL::SKIP_TRACK,
                    IC_DISPLAY::IC_SYMBOL::PREV_TRACK
                );
                if (!scrollingRequired) {
                    if (isPlaying) {
                        display->setBody(IC_DISPLAY::AUDIO, trackName, true);
                    } else {
                        display->setBody(IC_DISPLAY::AUDIO, PAUSED_BODY, true);
                    }
                }
            }
            isInPage = true;
        }
        if (inDiagMode) {
            if (millis() - lastUpdateBody >= DIAG_MODE_UPDATE_FREQ) {
                if (isInPage) display->setBody(IC_DISPLAY::AUDIO, getDiagBody(), true);
                lastUpdateBody = millis();
            }
        } else {
            if (millis() - lastUpdateHeader >= UPDATE_FREQ_HEADER) {
                DPRINTLN(MEMORY_STR_1+String(freeRam())+MEMORY_STR_2);
                if (isPlaying) elapsedSeconds++;
                createHeader();
                lastUpdateHeader = millis();
            }
            if (millis() - lastUpdateBody >= UPDATE_FREQ_SCROLL_BODY && scrollingRequired && isPlaying) {
                uint8_t len = strlen(trackName);
                char first = trackName[0];
                for (uint8_t i = 1; i < len; i++) {
                    trackName[i-1] = trackName[i];
                }
                trackName[len-1] = first;

                // Rotate text
                if (isInPage) display->setBody(IC_DISPLAY::AUDIO, trackName, true);
                lastUpdateBody = millis();
            }
        }
    }   else {
        isInPage = false;
    }
}

void AUDIO_DISPLAY::setPlayState(bool isPlaying) {
    this->isPlaying = isPlaying;
    createHeader();
}

void AUDIO_DISPLAY::setDuration(int seconds) {
    this->totalSeconds = seconds;
}

void AUDIO_DISPLAY::setElapsed(int seconds) {
    this->elapsedSeconds = seconds;   
}
const char PROGMEM SPACE = ' ';
void AUDIO_DISPLAY::setTrackName(const char* name) {
    memset(trackName, 0x00, sizeof(trackName));
    strcpy(trackName, name);
    scrollingRequired = !display->can_fit_body_text(trackName);
    if (scrollingRequired) {
        uint8_t len = strlen(trackName);
        trackName[len] = SPACE;
        trackName[len+1] = SPACE;
        trackName[len+2] = SPACE;
        trackName[len+3] = SPACE;
        if (isInPage) display->setBody(IC_DISPLAY::AUDIO, trackName, false);
    } else {
        if (isInPage) display->setBody(IC_DISPLAY::AUDIO, trackName, true);
    }   
}

void AUDIO_DISPLAY::createHeader() {
    if (strlen(trackName) == 0) {
        //if (isInPage) display->setHeader(IC_DISPLAY::AUDIO, STARTUP_HEADER, true);
        if (isInPage) display->setHeader(IC_DISPLAY::AUDIO, "Oil pressure", true);
        if (isInPage) display->setBody(IC_DISPLAY::AUDIO, STARTUP_BODY, true);
        return;
    }
    if (isPlaying) {
        uint8_t elapsed_mins = elapsedSeconds / 60;
        uint8_t elapsed_secs = elapsedSeconds % 60;
        uint8_t total_mins = totalSeconds / 60;
        uint8_t total_secs = totalSeconds % 60;
        sprintf(headerText, "%d:%02d/%d:%02d", elapsed_mins, elapsed_secs, total_mins, total_secs);
    } else {
        strcpy(headerText, PAUSED_HEADER);
        display->setBody(IC_DISPLAY::AUDIO, PAUSED_BODY, true);
    }
    if (isInPage) display->setHeader(IC_DISPLAY::AUDIO, headerText, false);
}

const char * AUDIO_DISPLAY::getDiagHeader() {
    switch(diagPage) {
        case 0:
            return DIAG_MODE_HEADER;
        case 1:
            return DIAG_HEADER_ATF;
        default:
            return "";
    }
}

const char * AUDIO_DISPLAY::getDiagBody() {
    switch(diagPage) {
        case 0:
            return DIAG_MODE_BODY;
        case 1:
            return "60C";
        default:
            return "";
    }
}

bool AUDIO_DISPLAY::getDiagModeEnabled() { return inDiagMode; }


void AUDIO_DISPLAY::enableDiagMode() {
    if (inDiagMode == false) {
        inDiagMode = true;
        isInPage = false;
    }
}
void AUDIO_DISPLAY::disableDiagMode() {
    if (inDiagMode == true) {
        inDiagMode = false;
        isInPage = false;
    }
}

void AUDIO_DISPLAY::diagNextPage() {
    if (diagPage == MAX_DIAG_PAGES) {
        diagPage = 0;
    } else {
        diagPage++;
    }
    display->setHeader(IC_DISPLAY::AUDIO, getDiagHeader(), true);
    display->setBody(IC_DISPLAY::AUDIO, getDiagBody(), true);
}

void AUDIO_DISPLAY::diagPrevPage() {
    if (diagPage == 0) {
        diagPage = MAX_DIAG_PAGES;
    } else {
        diagPage--;
    }
    display->setHeader(IC_DISPLAY::AUDIO, getDiagHeader(), true);
    display->setBody(IC_DISPLAY::AUDIO, getDiagBody(), true);
}
