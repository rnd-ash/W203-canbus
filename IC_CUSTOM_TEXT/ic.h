#ifndef W203_CANBUS_IC_NEW_H
#define W203_CANBUS_IC_NEW_H

#include "canbuscomm.h"
#include "EngineData.h"


#define W203 // Disable to compile for W211 IC display

#define STATIC_UPDATE_FREQ 2000
#define SCROLLING_UPDATE_FREQ 150
#define IC_DISPLAY_ID 0x1A4
#define DIAG_MODE_UPDATE_FREQ 100
#define DIAG_SCREENS 3

#ifdef W203
    #define MAX_IC_BODY_CHARS 10
    #define MAX_IC_HEAD_CHARS  4
#else
    #define MAX_IC_BODY_CHARS 11
    #define MAX_IC_HEAD_CHARS  8
#endif
#define SENSOR_UDPATE_TIME 50

class IC_DISPLAY{
    public:
        IC_DISPLAY(CanbusComm *c, EngineData *d);
        void setBody(const char* body);
        void setHeader(const char* header);
        void update();
        void nextDiagPage();
        void prevDiagPage();
        bool inDiagMode;
    private:
        unsigned long lastKeepAliveMillis;
        void setDiagText();
        bool isSending;
        uint8_t displayTextLen;
        uint8_t textLen;
        can_frame curr_frame;
        can_frame diag_frame;
        uint8_t framePayload[24] = {0x00};
        uint8_t headerFramePayload[16] = { 0x00 };
        unsigned long lastUpdateMillis;
        unsigned long nextUpdateMillis;
        String diagBuffer;
        char bodyCharBuffer[128];
        void shiftText();
        void setBody();
        void asyncSendBody();
        CanbusComm *c;
        uint8_t sendFrame();
        uint8_t calculateBodyChecksum(uint8_t len);
        uint8_t diagPage;
        EngineData *d;
        uint8_t currFrame;
        bool isUpdating;
};


#endif