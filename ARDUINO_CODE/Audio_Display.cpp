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
    if (millis() - lastUpdateHeader >= UPDATE_FREQ_HEADER) {
        elapsedSeconds++;
        DPRINTLN("FREE MEM: "+String(freeRam()) + " Bytes");
        createHeader();
        lastUpdateHeader = millis();
    }
}

void AUDIO_DISPLAY::setPlayState(bool isPlaying) {
    this->isPlaying = isPlaying;
}

void AUDIO_DISPLAY::setDuration(int seconds) {
    this->totalSeconds = seconds;
}

void AUDIO_DISPLAY::setElapsed(int seconds) {
    this->elapsedSeconds = seconds;   
}

void AUDIO_DISPLAY::setTrackName(const char* name) {
    
}

void AUDIO_DISPLAY::createHeader() {
    if (isPlaying) {
        uint8_t elapsed_mins = elapsedSeconds / 60;
        uint8_t elapsed_secs = elapsedSeconds % 60;
        uint8_t total_mins = totalSeconds / 60;
        uint8_t total_secs = totalSeconds % 60;
        sprintf(headerText, "%d:%02d/%d:%02d", elapsed_mins, elapsed_secs, total_mins, total_secs);
    } else {
        strcpy(headerText, PAUSED_HEADER);
    }
    display->setHeader(IC_DISPLAY::AUDIO, headerText, false);
}
