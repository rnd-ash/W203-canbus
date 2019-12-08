#include "Music.h"
#include "debug.h"
#include "wiring_private.h"

/**
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
    this->playing = true;
}

void Music::pause() {
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
}

void Music::setElapsed(int seconds) {
    DPRINTLN("Seeking track to "+String(seconds));
    this->elapsedSeconds = seconds;
    lastUpdateTime = millis();
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
