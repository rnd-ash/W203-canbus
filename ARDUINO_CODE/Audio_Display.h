#ifndef AUDIO_DISPLAY_H
#define AUDIO_DISPLAY_H

#include "ic_display.h"


#define UPDATE_FREQ_HEADER 1000
#define UPDATE_FREQ_SCROLL_BODY 150


const char * const PROGMEM PAUSED_HEADER = "Paused";
class AUDIO_DISPLAY {
    public:
        AUDIO_DISPLAY(IC_DISPLAY *d);
        void update();
        void setPlayState(bool isPlaying);
        void setDuration(int seconds);
        void setElapsed(int seconds);
        void setTrackName(const char* name);
    private:
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

};
#endif