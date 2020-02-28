#ifndef MUSIC_H
#define MUSIC_H

#include "Audio_Display.h"
#include "defines.h"

const char * const PROGMEM paused_text = "Paused";
const char * const PROGMEM unknown_duration = "Unk. Duration";

class MUSIC {
    public:
        MUSIC(AUDIO_DISPLAY *disp);
        const char* getTrackName();
        void setTrackName(const char * track);
        void setDurationSec(int sec);
        void setElapsedSec(int elapsed);
        void play();
        void pause();
        bool isPlaying();
        void updateUI();
        void update();
        int getDurationSec();
        int getElapsedSec();
    private:
        long elapsedMillis = 0L;
        long totalMillis = 0L;
        bool playing = false;
        char track[255] = {0x00};
        unsigned long lastUpdate = millis();
        unsigned long lastUIUpdate = millis();
        AUDIO_DISPLAY* displayPage;
};

#endif