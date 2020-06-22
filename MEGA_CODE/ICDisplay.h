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
    void update();
    static void processIncommingFrame(can_frame *f);
    void test();
    ICDisplay();
    virtual void init(){};
    void setBody(uint8_t page, lineData* line);
protected:
    virtual void onSendComplete(){}; // Abstract
    void putChecksum(uint8_t*buffer, uint8_t bufferSize);
    void buildFrameBuffer(uint8_t* buffer, uint8_t bufferSize);
    void updateHeader(uint8_t page, lineData* line);
    void initPage();
    void updateBodyMultipleLines(uint8_t page, lineData* lines, uint8_t numLines);
private:
    void sendToKombi();
    can_frame* frameBuffer;
    uint8_t frameBufferSize;
    uint8_t currFrameBufferIndex;
    bool sendComplete;
    bool isSending;
    unsigned long nextUpdateTime = millis();
};

class AudioDisplay : public ICDisplay {
public:
    void setMusicData(Music *m);
private:
    Music* m = nullptr;
    char body_text_buffer[256]; // This can do up to 4 lines
    char head_text_buffer[64]; // Max of 1 line for header
};

class TelDisplay : public ICDisplay {

};

extern AudioDisplay* audioDisplay;
extern TelDisplay* telDisplay;
#endif