#ifndef IC_DISPLAY_H_
#define IC_DISPLAY_H_

#include "Canbus.h"
#include "Music.h"

#define SEND_CID 0x01A4
#define RESP_CID 0x01D0
#define SEPERATION_TIME_MS 5

#define DISPLAY_WIDTH 56 // In pixels

// Text formatting options
#define IC_FMT_LEFT        0b00000000
#define IC_FMT_RIGHT       0b00001000
#define IC_FMT_CENTER      0b00010000
#define IC_FMT_FLASHING    0b00100000
#define IC_FMT_HIGHLIGHTED 0b01000000

// Symbols for Audio page
#define IC_SYMB_NONE       0b00000000 
#define IC_SYMB_SKIP_TRACK 0b00000001 // |>>
#define IC_SYMB_PREV_TRACK 0b00000010 // <<|
#define IC_SYMB_FAST_FWD   0b00000011 // >>
#define IC_SYMB_FAST_REV   0b00000100 // <<
#define IC_SYMB_PLAY       0b00000101 // ▶️
#define IC_SYMB_REWIND     0b00000110 // ◀
#define IC_SYMB_UP_ARROW   0b00001001 // ↑
#define IC_SYMB_DOWN_ARROW 0b00001010 // ↓

const uint8_t CHAR_WIDTHS_BODY[256] PROGMEM = {   // Global variable so PROGMEM attribute can be used  
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 7, 6, 0, 0, 0,
    0, 6, 6, 6, 7, 7, 3, 2, 
    7, 7, 0, 0,10,10, 6, 6,
    6, 3, 4, 6, 6, 6, 6, 2,
    5, 5, 6, 6, 3, 5, 2, 6,
    7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 3, 4, 5, 6, 5, 6,

    6, 7, 7, 7, 7, 6, 6, 7,
    7, 3, 5, 7, 6, 7, 0, 0,
    7, 7, 7, 7, 7, 7, 7,11,
    7, 7, 7, 4, 6, 4, 3, 6,
    3, 6, 6, 6, 6, 7, 6, 8,
    6, 3, 5, 6, 3, 9, 7, 7,
    6, 6, 6, 6, 5, 7, 7, 9,
    7, 6, 6, 6, 2, 6,99, 0, // One marked with 99 crashes the IC!

    7, 6, 8, 9, 6, 6, 6, 6,
    7, 6, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,

    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

/**
 * Base class for Audio and Telephone page on the IC
 */

struct lineData {
    uint8_t fmt_args;
    uint8_t textLen;
    uint8_t *text;
};

class ICDisplay {
public:
    virtual void update() = 0;
    static void processIncommingFrame(can_frame *f);
    void test();
    ICDisplay();
    virtual void init(){};
    void setBody(uint8_t page, lineData* line);
    void setHeader(uint8_t page, lineData* line);
    void enablePage();
    void disablePage();
protected:
    virtual void onSendComplete(){}; // Abstract
    void putChecksum(uint8_t*buffer, uint8_t bufferSize);
    void buildFrameBuffer(uint8_t* buffer, uint8_t bufferSize);
    void initPage();
    void updateBodyMultipleLines(uint8_t page, lineData* lines, uint8_t numLines);
    bool bodyCanFit(lineData *d);
private:
    void sendToKombi();
    can_frame* frameBuffer;
    uint8_t frameBufferSize;
    uint8_t currFrameBufferIndex;
    bool sendComplete;
    bool isSending;
    unsigned long nextUpdateTime = millis();
    bool isInPage = false;
};

#define MUSIC_PROGRESS_UPDATE 1000 // Every second update the progress text
#define MUSIC_TEXT_UPDATE 400 // Every 300ms scroll the body text

const char * const PROGMEM PAUSED = "Paused";

class AudioDisplay : public ICDisplay {
public:
    void update();
    void setMusicData(Music *m);
    void removeMusic();
private:
    Music* music = nullptr;
    lineData *bodyText;
    uint8_t bodyHash = 0x00;
    lineData *headText;
    uint8_t headHash = 0x00;
    unsigned long lastProgressUpdate = 0;
    unsigned long lastTextUpdate = 0;
    uint8_t textStartPos = 0;
    bool trackNameScroll = false;
};

class TelDisplay : public ICDisplay {
public:
    void update();
};

extern AudioDisplay* audioDisplay;
extern TelDisplay* telDisplay;
#endif