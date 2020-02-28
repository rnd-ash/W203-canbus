#include "Music.h"


MUSIC::MUSIC(AUDIO_DISPLAY *disp) {
    this->playing = false;
    this->lastUpdate = millis();
    this->lastUIUpdate = millis();
    this->displayPage = disp;
}

const char* MUSIC::getTrackName() {
    return this->track;
}

void MUSIC::setTrackName(const char * track) {
    if (strlen(track) > 0) {
        strcpy(this->track, track);
    } else {
        this->playing = false;
    }
}

void MUSIC::setDurationSec(int sec) {
    this->totalMillis = sec * 1000L;
    this->elapsedMillis = 0L;
}

void MUSIC::setElapsedSec(int elapsed) {
    this->elapsedMillis = elapsed * 1000L;
}

void MUSIC::play() {
    DPRINTLN(F("Music playing"));
    this->playing = true;
    this->lastUpdate = millis();
}

void MUSIC::pause() {
    DPRINTLN(F("Music paused"));
    this->playing = false;
}

bool MUSIC::isPlaying() {
    return this->playing;
}

int MUSIC::getDurationSec() {
    return this->totalMillis / 1000;
}

int MUSIC::getElapsedSec() {
    return this->elapsedMillis / 1000;
}

void MUSIC::update() {
    if (millis() - lastUpdate > 5) {
        if (this->playing) { 
            this->elapsedMillis += millis() - lastUpdate;
        }
        lastUpdate = millis();
    }
}


void MUSIC::updateUI() {
    if (millis() - lastUIUpdate >= 1000) {
        lastUIUpdate = millis();

        if (strlen(track) == 0) {
            displayPage->setSymbols(IC_SYMB_NONE, IC_SYMB_NONE);
            displayPage->setHeader("No Connect.", IC_TEXT_FMT_CENTER_JUSTIFICATION, 5000);
            displayPage->setBody("Open app", IC_TEXT_FMT_HIGHLIGHTED | IC_TEXT_FMT_CENTER_JUSTIFICATION, 5000);
            return;
        }


        if (this->playing) {
            displayPage->setSymbols(IC_SYMB_SKIP_TRACK, IC_SYMB_PREV_TRACK);
            displayPage->setBody(track, IC_TEXT_FMT_CENTER_JUSTIFICATION, 5000);
            if (totalMillis > 0) {
                char prog_buffer[12];
                uint8_t elapsed_mins = getElapsedSec() / 60;
                uint8_t elapsed_secs = getElapsedSec() % 60;
                uint8_t total_mins = getDurationSec() / 60;
                uint8_t total_secs = getDurationSec() % 60;
                sprintf(prog_buffer, "%d:%02d/%d:%02d", elapsed_mins, elapsed_secs, total_mins, total_secs);
                displayPage->setHeader(prog_buffer, IC_TEXT_FMT_LEFT_JUSTIFICATION, 1000);
            } else {
                displayPage->setHeader(unknown_duration, IC_TEXT_FMT_HIGHLIGHTED, 5000);
            }
        } else {
            displayPage->setSymbols(IC_SYMB_PLAY, IC_SYMB_NONE);
            displayPage->setHeader(paused_text, IC_TEXT_FMT_CENTER_JUSTIFICATION | IC_TEXT_FMT_FLASHING, 5000);
        }
    }
}