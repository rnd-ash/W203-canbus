#include "Music.h"
#include "debug.h"
#include "wiring_private.h"

/**
 * 
 * Music stuff
 * 
 */

Music::Music() {
    this->trackName[128] = { 0x00 };
    this->elapsedSeconds = 0;
    this->totalSeconds = 0;
    this->playing = false;
    this->lastUpdateTime = millis();
}

void Music::play() {
    DPRINTLN(F("MUSIC PLAYING"));
    this->playing = true;
}

void Music::pause() {
    DPRINTLN(F("MUSIC PAUSED"));
    this->playing = false;
}

void Music::update() {
    if (millis() - lastUpdateTime >= 1000) {
        lastUpdateTime = millis();
        if(this->playing) {
            elapsedSeconds++;
            if (totalSeconds != 0) {
                progressPercent = ((float) elapsedSeconds / (float) totalSeconds) * 100;
            } else {
                progressPercent = 0;
            }
        }
    }
}

void Music::setSeconds(int seconds) {
    this->elapsedSeconds = 0;
    this->totalSeconds = seconds;
    DPRINTLN("Track is of length "+String(totalSeconds));
}

bool Music::isPlaying() {
    return this->playing;
}


void Music::setText(const char* track) {
    memset(trackName, 0x00, sizeof(trackName));
    int textLen = strlen(track);
    for (uint8_t i = 0; i < textLen; i++) {
        trackName[i] = track[i];
    }
}

char* Music::getDisplayText() {
    return this->trackName;
}
