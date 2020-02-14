#ifndef AUDIO_DISPLAY_H
#define AUDIO_DISPLAY_H

#include "ic_display.h"
#include "Engine.h"
#define MAX_DIAG_PAGES 3

const char * const PROGMEM STARTUP_HEADER = "No Connection";
const char * const PROGMEM STARTUP_BODY = "Open App!";

const char * const PROGMEM PAUSED_HEADER = "Paused";
const char * const PROGMEM PAUSED_BODY = "Press play";
const char * const PROGMEM DIAG_MODE_HEADER = "Scroll";
const char * const PROGMEM DIAG_MODE_BODY = "DAIG MODE";

const char * const PROGMEM MEMORY_STR_1 = "FREE SRAM: ";
const char * const PROGMEM MEMORY_STR_2 = " Bytes";

const char * const PROGMEM DIAG_HEADER_ATF = "ATF Temp (C)";
const char * const PROGMEM DIAG_HEADER_TORQUE_CONVERTER = "TC lockup";
const char * const PROGMEM DIAG_HEADER_GEARING = "Tar/Act gear";

const char * const PROGMEM DIAG_ERROR_HEAD = "INVALID";
const char * const PROGMEM DIAG_ERROR_BODY = "ERROR";

class AUDIO_DISPLAY {
    public:
        AUDIO_DISPLAY(IC_DISPLAY *d);
        void update();
        void setPlayState(bool isPlaying);
        void setDuration(int seconds);
        void setElapsed(int seconds);
        void setTrackName(const char* name);
        void enableDiagMode(ENGINE_DATA *eng);
        void disableDiagMode();
        bool getDiagModeEnabled();
        void diagNextPage();
        void diagPrevPage();
    private:
        ENGINE_DATA *eng;
        bool inDiagMode = false;
        uint8_t diagPage = 0;
        bool isInPage = true;
        IC_DISPLAY *display;
        int elapsedSeconds = 0;
        int totalSeconds = 0;
        char trackName[64] = { 0x00 };
        char headerText[15] = { 0x00 };
        bool isPlaying = false;
        bool scrollingRequired = false;
        unsigned long lastUpdateBody = 0L;
        unsigned long lastUpdateHeader = 0L;
        void createHeader();
        const char * getDiagHeader();
        const char * getDiagBody();
};
#endif