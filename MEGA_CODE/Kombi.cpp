#include "Kombi.h"
#include "helper.h"



void Kombi::processKombiFrame(can_frame *f) {
    switch (f->can_id)
    {
    case CID_KOMBI_A1:
        processFrameA1(f);
        break;
    case CID_KOMBI_A5:
        processFrameA5(f);
        break;
    default:
        // Not matching frame (not from kombi), just return
        break;
    }
}

void Kombi::processFrameA1(can_frame *f) {
    this->vSpeedKmh = f->data[1];
    // this bit is 1 when metric is in use (KMH), and 0 if imperial is being used (MPH)
    this->isUsingImperial = (f->data[4] & 0b01000000) == 0;
}

void Kombi::processFrameA5(can_frame *f) {
    if (f->data[0] == KOMBI_PAGE_AUDIO || f->data[0] == KOMBI_PAGE_OTHER || f->data[0] == KOMBI_PAGE_PHONE) {
        currPage = f->data[0];
    } else {
        LOG_WARN(F_TC("Unknown IC page. hex byte: %02X, ignoring\n"), f->data[0]);
        currPage = KOMBI_PAGE_AUDIO;
    }
    // Something being held down
    if (f->data[1] != 0x00) {
        if (!keyPressed) {
            lastPollTime = millis();
            keyPressed = true;
            keyPressDuration = 0;
        }
        currKeyPress = f->data[1];
    } else {
        keyPressDuration = millis() - lastPollTime;
        keyPressed = false;
    }
}

uint8_t Kombi::getKeyPress() {
    if (!keyPressed && currKeyPress != 0x00){
        uint8_t temp = currKeyPress;
        currKeyPress = 0x00;
        if (keyPressDuration >= TIME_TO_LONG_PRESS) {
            temp |= 0b0000100; // Convert to a long press
        }
        keyPressDuration = 0;
        return temp;
    }
    return 0x00;
}

bool Kombi::useImperial() {
    return this->isUsingImperial;
}

uint8_t Kombi::getICPage() {
    return this->currPage;
}

// -- Global Kombi --
Kombi* kombi = nullptr;