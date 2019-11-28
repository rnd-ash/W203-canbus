#ifndef W203_CANBUS_MUSIC_H
#define W203_CANBUS_MUSIC_H

#include <HardwareSerial.h>

class Music {
    public:
        Music();
        void setSeconds(int seconds);
        int elapsedSeconds;
        int totalSeconds;
        void pause();
        void play();
        void update();
        bool isPlaying();
        void setText(const char* track);
        char* getDisplayText();
        float progressPercent();
    private:
        unsigned long lastUpdateTime;
        bool playing;
        char trackName[32];
};

#endif