#ifndef MUSIC_H_
#define MUSIC_H_

#include "Bluetooth.h"
#include "Arduino.h"

class Music {
    public:
        Music(char* trackName, uint8_t trackNameLen);
        void setArtist(char* artistName, uint8_t artistNameLen);
        int getDuration();
        void setDuration(int secs);
        bool isPlaying();
        void setPlayStatus(bool playing);
        int getPosition();
        void setPosition(int secs);
        void update();
        const char* getArtistName();
        uint8_t getArtistNameLen();
        const char* getTrackName();
        uint8_t getTrackNameLen();
        ~Music();
    private:
        void updateSeek(long elapsed);
        unsigned long lastUpdate = millis();
        char *trackName = nullptr;
        int trackNameLen = 0;
        char *trackArtist = nullptr;
        int artistNameLen = 0;
        long duration_ms = 0;
        long position_ms = 0;
        bool playing = false;
};

#endif