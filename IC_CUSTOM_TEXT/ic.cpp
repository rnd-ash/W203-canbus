#include "ic.h"
#include "debug.h"
#include "avr/pgmspace.h"

const char * const PROGMEM BT_MSG = "NO BT!!";
const char * const PROGMEM TELE_HEAD = "TELE";
const char * const PROGMEM DIAG_TXT = "DIAG MODE";

// Width of different ASCII Codes when the IC Renders them
const uint8_t ASCII_WIDTHS[256] PROGMEM = {

    // ------------------------STANDARD ASCII----------------------//
    0, 0, 0, 0, 0, 0, 0, 0, // NUL, SOH, STX, ETX, EOT, ENQ, ACK, BEL
    0, 0, 0, 0, 0, 0, 0, 0, //  BS, TAB,  LF,  VT,  FF,  CR,  SO,  SI
    0, 0, 0, 0, 0, 0, 0, 0, // DLE, DC1, DC2, DC3, DC4, NAK, SYN, ETB
    0, 0, 0, 0, 0, 0, 0, 0, // CAN,  EM, SUB, ESC,  FS,  GS,  RS,  US
    3, 3, 4, 6, 0, 6, 6, 2, // ' ', '!', '"', '#', '$', '%', '&', '''
    5, 5, 6, 6, 3, 5, 2, 6, // '(', ')', '*', '+', '`', '-', '.', '/'
    7, 4, 7, 7, 8, 7, 7, 6, // '0', '1', '2', '3', '4', '5', '6', '7'
    7, 7, 3, 4, 0, 6, 0, 5, // '8', '9', ':', ';', '<', '=', '>', '?'
    6, 7, 7, 7, 7, 5, 5, 7, // '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G'
    7, 3, 5, 7, 5, 7, 7, 7, // 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O'
    7, 7, 7, 7, 7, 7, 7, 11, // 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W'
    7, 7, 6, 4, 6, 4, 0, 5, // 'X', 'Y', 'Z', '[', '\', ']', '^', '_'
    2, 7, 6, 6, 6, 7, 6, 7, // '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g'
    6, 3, 5, 6, 3, 9, 7, 7, // 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o'
    6, 6, 6, 6, 5, 7, 7, 9, // 'p', 'q', 'r', 's', 't', 'u', 'v', 'w'
    7, 6, 5, 5, 2, 5, 0, 0, // 'x', 'y', 'z', '{', '|', '}', '~', DEL

    // ------------------------EXTENDED ASCII----------------------//
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
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0

};



byte IC_DISPLAY::page;
IC_DISPLAY::IC_DISPLAY(CanbusComm *c, EngineData *d) {
    char displayBuffer[10];
    char charBuffer[256];
    this-> c = c;
    this->inDiagMode = false;
    this->lastUpdateMillis = millis();
    this->curr_frame.can_id = IC_DISPLAY_ID;
    this->curr_frame.can_dlc = 0x08;
    this->setBody(BT_MSG);
    this->setHeader(TELE_HEAD);
    this->isSending = false;
    this->diagPage = 0;
    this->diagBuffer.reserve(9);
    this->currFrame = 0;
    this->d = d;
    this->page = OTHER;

    diag_frame.can_dlc = 8;
    diag_frame.can_id = 0x1c;
    diag_frame.data[0] = 0x02;
    diag_frame.data[1] = 0x10;
    diag_frame.data[2] = 0x92;
    diag_frame.data[3] = 0xff;
    diag_frame.data[4] = 0xff;
    diag_frame.data[5] = 0xff;
    diag_frame.data[6] = 0xff;
    diag_frame.data[7] = 0xff;
    c->sendFrame(CAN_BUS_B, &diag_frame);
    lastKeepAliveMillis = millis();
    diag_frame.data[1] = 0x3e;
    diag_frame.data[2] = 0x02;

    isUpdating = false;
    #ifdef SIMULATION
        this->inDiagMode = true;
        this->diagPage = 4;
    #endif
}

void IC_DISPLAY::setBody(const char* body) {
    memset(bodyCharBuffer, 0x00, sizeof(bodyCharBuffer));
    int text_width=0;
    for (int i = 0; i < strlen(body); i++) {
        // Remove ~ from string as that causes the IC to freak out and display garbage

        // On a side note, sending '~ Test' to the IC will make it render the text 'Test' with left justification!
        if (body[i] == '~') {
            bodyCharBuffer[i] = ' ';
            text_width += ASCII_WIDTHS[20];
        } else {
            bodyCharBuffer[i] = body[i];
            text_width += pgm_read_byte_near(ASCII_WIDTHS + (uint8_t) body[i]);
        }
    }
    text_width--; // minus 1 because last character has no padding
    DPRINTLN("Text width: "+String(text_width)+" pixels");
    // Text too wide, scroll, or too many chars to send in 2 frames
    if (text_width > IC_WIDTH_PIXELS || strlen(bodyCharBuffer) > ABSOLUTE_IC_MAX_BODY_CHARS) {
        int bufferLen = strlen(bodyCharBuffer);
        bodyCharBuffer[bufferLen]  = ' ';
        bodyCharBuffer[bufferLen+1] = ' ';
        bodyCharBuffer[bufferLen+2] = ' ';
        bodyCharBuffer[bufferLen+3] = ' ';
        this->shouldScrollText = true;
    } else {
        this->shouldScrollText = false;
    }
    this->lastUpdateMillis = 0L;
}

void IC_DISPLAY::setHeader(const char* header) {
    char text[MAX_IC_HEAD_CHARS+1];
    memset(text, 0x00, sizeof(text));
    if (strlen(header) < MAX_IC_HEAD_CHARS) {
        for (uint8_t i = 0; i < strlen(header); i++) {
            text[i] = header[i];
        }
        for (uint8_t i = strlen(header); i < MAX_IC_HEAD_CHARS; i++) {
            text[i] = ' ';
        }
    } else {
        for (uint8_t i = 0; i < MAX_IC_HEAD_CHARS; i++) {
            text[i] = header[i];
        }
    }

    memset(headerFramePayload, 0x00, sizeof(headerFramePayload));
    int slen = min(8, strlen(text));
    int len = slen + 5;
    headerFramePayload[0] = 0x10;
    headerFramePayload[1] = len;
    headerFramePayload[2] = 0x03;
    headerFramePayload[3] = 0x29;
    headerFramePayload[4] = 0x00;
    headerFramePayload[8] = 0x21;
    for (int i = 0; i < min(3, slen); i++) headerFramePayload[5+i] = text[i];
    for (int i = 0; i < min(8, slen); i++) headerFramePayload[9+i] = text[i+3];
    uint8_t hash = 439 - (text[0] + text[1] + text[2] + text[3]);
    headerFramePayload[11] = hash;
    for (uint8_t i = 0; i <= 7; i++) {
        curr_frame.data[i] = headerFramePayload[i];
    }
    c->sendFrame(CAN_BUS_B, &curr_frame);
    delay(7);
    for (uint8_t i = 8; i <= 15; i++) {
        curr_frame.data[i-8] = headerFramePayload[i];
    }
    c->sendFrame(CAN_BUS_B, &curr_frame);
    delay(7);
}


void IC_DISPLAY::setDiagText() {
    switch (diagPage)
    {
    case 0:
        diagBuffer = DIAG_TXT;
        break;
    case 1:
        diagBuffer = d->getSpeed();
        break;
    case 2:
        diagBuffer = d->getRpm();
        break;
    case 3:
        diagBuffer = d->getCoolantTemp();
        break;
    case 4:
        diagBuffer = d->getBhp();
        break;
    case 5:
        diagBuffer = d->getTorque();
        break;
    default:
        break;
    }
}

uint8_t x = 0;
void IC_DISPLAY::update() {
    if (isUpdating) {
        asyncSendBody();
        return;
    }
    if (inDiagMode) {
        if (millis() - lastUpdateMillis > DIAG_MODE_UPDATE_FREQ) {
            setDiagText();
            setBody();
            lastUpdateMillis = millis();
            nextUpdateMillis = millis();
            #ifdef SIMULATION
                delay(100);
                d->isOn = true;
                d->speed += random(2);
            #endif
        }
    }
    else if (shouldScrollText) {
        if (millis() - lastUpdateMillis > SCROLLING_UPDATE_FREQ) {
            lastUpdateMillis = millis();
            nextUpdateMillis = millis();
            setBody();
            this->shiftText();
        }
    } else {
        if (millis() - lastUpdateMillis > STATIC_UPDATE_FREQ) {
            lastUpdateMillis = millis();
            nextUpdateMillis = millis();
            setBody();
        }
    }
}

void IC_DISPLAY::shiftText() {
    char first = bodyCharBuffer[0];
    uint8_t len = strlen(bodyCharBuffer);
    for (uint8_t i = 1; i < len; i++) {
        bodyCharBuffer[i-1] = bodyCharBuffer[i];
    }
    bodyCharBuffer[len-1] = first;
}

void IC_DISPLAY::setBody() {
    uint8_t strLen = 0;
    char displayBuffer[12];
    memset(displayBuffer, 0x00, sizeof(displayBuffer));
    if (inDiagMode) {
        strLen = min(ABSOLUTE_IC_MAX_BODY_CHARS, diagBuffer.length());
        for (int i = 0; i < strLen; i++) {
            displayBuffer[i] = diagBuffer[i];
        }
    } else {
        if (shouldScrollText) {
            strLen = min(SCROLL_CHARS, strlen(bodyCharBuffer));
        } else {
            strLen = min(ABSOLUTE_IC_MAX_BODY_CHARS, strlen(bodyCharBuffer));
        }
        for (int i = 0; i < strLen; i++) {
            displayBuffer[i] = bodyCharBuffer[i];
        }
    }
    uint8_t len = strLen + 9;
    memset(framePayload, 0x00, sizeof(framePayload));
    framePayload[0] = 0x10;
    framePayload[1] = len;
    framePayload[2] = 0x03;
    framePayload[3] = 0x26;
    framePayload[4] = 0x01;
    framePayload[5] = 0x00;
    framePayload[6] = 0x01;
    framePayload[7] = strLen + 2;
    framePayload[8] = 0x21;
    framePayload[9] = 0x10;
    framePayload[16] = 0x22;

    for(uint8_t i = 0; i < min(6,strLen); i++) framePayload[10+i] = displayBuffer[i];
    for(uint8_t i = 0; i < min(SCROLL_CHARS, strLen) - 6; i++) framePayload[17+i] = displayBuffer[i+6];

    uint8_t hash = 0xCA;
    for(uint8_t i = 0; i < len; i++) hash -= i;
    for (uint8_t i = 10; i < 16; i++) hash -=framePayload[i];
    for (uint8_t i = 17; i < 23; i++) hash -=framePayload[i];
    framePayload[(len > 13) ? (len + 3) : (len + 2)] = hash;
    isUpdating = true;
}

uint8_t IC_DISPLAY::sendFrame() {
    if (currFrame == 0) {
        for (uint8_t i = 0; i <= 7; i++) {
            curr_frame.data[i] = framePayload[i];
        }
        c->sendFrame(CAN_BUS_B, &curr_frame);
        currFrame++;
        return 7;
    } else if (currFrame == 1) {
        for (uint8_t i = 8; i <= 15; i++) {
            curr_frame.data[i-8] = framePayload[i];
        }
        c->sendFrame(CAN_BUS_B, &curr_frame);
        currFrame++;
        return 2;
    } else if (currFrame == 2) {
        for (uint8_t i = 16; i <= 23; i++) {
            curr_frame.data[i-16] = framePayload[i];
        }
        c->sendFrame(CAN_BUS_B, &curr_frame);
        currFrame = 0;
        isUpdating = false;
        return 2;
    }
}


void IC_DISPLAY::asyncSendBody() {
    if (millis() >= nextUpdateMillis) {
        nextUpdateMillis = lastUpdateMillis + sendFrame();
    }
}

void IC_DISPLAY::nextDiagPage() {
    if(diagPage == DIAG_SCREENS) {
        diagPage = 0;
    } else {
        diagPage++;
    }
    diagSetHeader();
}

void IC_DISPLAY::prevDiagPage(){
   if(diagPage == 0) {
       diagPage = DIAG_SCREENS;
   } else {
       diagPage--;
   }
   diagSetHeader();
}

const char * const PROGMEM DIAG_HEAD_MAIN    = "MAIN";
const char * const PROGMEM DIAG_HEAD_SPEED   = "SPD";
const char * const PROGMEM DIAG_HEAD_RPM     = "RPM";
const char * const PROGMEM DIAG_HEAD_COOLENT = "CLNT";
const char * const PROGMEM DIAG_HEAD_POWER   = "BHP";
const char * const PROGMEM DIAG_HEAD_TORQUE  = "TORQ";

void IC_DISPLAY::diagSetHeader() {
    switch (diagPage)
    {
    case 0:
        setHeader(DIAG_HEAD_MAIN);
        break;
    case 1:
        setHeader(DIAG_HEAD_SPEED);
        break;
    case 2:
        setHeader(DIAG_HEAD_RPM);
        break;
    case 3:
        setHeader(DIAG_HEAD_COOLENT);
        break;
    case 4:
        setHeader(DIAG_HEAD_POWER);
        break;
    case 5:
        setHeader(DIAG_HEAD_TORQUE);
        break;
    default:
        break;
    }
}