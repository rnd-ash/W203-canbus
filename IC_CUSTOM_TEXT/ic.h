#ifndef W203_CANBUS_IC_NEW_H
#define W203_CANBUS_IC_NEW_H

#include "canbuscomm.h"
#include "EngineData.h"


#define W203 // Disable to compile for W211 IC display
#define IC_DISPLAY_ID 0x1A4

#define ABSOLUTE_IC_MAX_BODY_CHARS 11 // Absolute maximum number of ASCII Chars allowed in 2 frames
#define SCROLL_CHARS 12
#ifdef W203
    #define IC_WIDTH_PIXELS 56
    #define MAX_IC_HEAD_CHARS  8
#else
    #define IC_WIDTH_PIXELS 140
    #define MAX_IC_HEAD_CHARS  8
#endif

class IC_DISPLAY{
    public:
        enum DISPLAY_PAGE {
            AUDIO = 0x03, /// Audio page
            TELEPHONE = 0x05, /// Telephone page
            OTHER = 0x00 /// Any other page that we don't want
        };
        
        /**
         * Used to represent a symbol that the IC can display on a page
         */
        enum SYMBOL {
            NONE = 0x00, /// No symbol to be displayed
            NEXT_TRACK = 0x01, /// Next track icon 
            PREV_TRACK = 0x02, /// Previous track icon
            FAST_FWD   = 0x03, /// Fast forward icon
            FAST_REV   = 0x04, /// Fast reverse icon
            PLAY       = 0x05, /// Play icon
            REWIND     = 0x06, /// Rewind icon
            UP_ARROW   = 0x09, /// Up arrow
            DOWN_ARROW = 0x0A /// Down arrow
        };
        static byte page;
        static uint8_t MAX_DISPLAY_WIDTH_PIXELS;
        IC_DISPLAY(CanbusComm *c, EngineData *d);
        void initPage(DISPLAY_PAGE p, IC_DISPLAY::SYMBOL upper, IC_DISPLAY::SYMBOL lower , bool shouldCenter, const char* header);
        void setSymbols(DISPLAY_PAGE p, SYMBOL top, SYMBOL bottom);
        void setbodyText(DISPLAY_PAGE p, bool centerText, const char* line1, const char* line2, const char* line3, const char* line4);
        void setHeader(DISPLAY_PAGE p, bool shouldCenter, const char* header);
        static bool textCanFit(const char* chars);
    private:
        void sendPacketsISO(uint8_t byteCount, uint8_t* bytes);
        uint8_t calculateChecksum(uint8_t size, uint8_t* bytes);
        CanbusComm *c;
        uint8_t sendFrame();
};
#endif