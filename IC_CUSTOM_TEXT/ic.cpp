#include "ic.h"
#include "debug.h"



byte IC_DISPLAY::page;
IC_DISPLAY::IC_DISPLAY(CanbusComm *c, EngineData *d) {
    char displayBuffer[10];
    char charBuffer[256];
    this-> c = c;
    this->inDiagMode = false;
    this->lastUpdateMillis = millis();
    this->curr_frame.can_id = IC_DISPLAY_ID;
    this->curr_frame.can_dlc = 0x08;
    this->setBody("NO BT!!");
    this->setHeader("TELE");
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
    for (int i = 0; i < strlen(body); i++) {
        bodyCharBuffer[i] = body[i];
    }
    int bufferLen = strlen(bodyCharBuffer);
    textLen = bufferLen;
    if (bufferLen > MAX_IC_BODY_CHARS) {
        bodyCharBuffer[bufferLen]  = ' ';
        bodyCharBuffer[bufferLen+1] = ' ';
        bodyCharBuffer[bufferLen+2] = ' ';
        bodyCharBuffer[bufferLen+3] = ' ';
        textLen+=4;
    }
    this->lastUpdateMillis = 0L;
}

void IC_DISPLAY::setHeader(const char* header) {

    memset(headerFramePayload, 0x00, sizeof(headerFramePayload));
    int slen = min(8, strlen(header));
    int len = slen + 5;
    headerFramePayload[0] = 0x10;
    headerFramePayload[1] = len;
    headerFramePayload[2] = 0x03;
    headerFramePayload[3] = 0x29;
    headerFramePayload[4] = 0x00;
    headerFramePayload[8] = 0x21;
    for (int i = 0; i < min(3, slen); i++) headerFramePayload[5+i] = header[i];
    for (int i = 0; i < min(8, slen); i++) headerFramePayload[9+i] = header[i+3];
    uint8_t hash = 439 - (header[0] + header[1] + header[2] + header[3]);
    headerFramePayload[11] = hash;
    Serial.println(len);
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
        diagBuffer = F("DIAG MODE");
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
    else if (textLen > MAX_IC_BODY_CHARS) {
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
    for (uint8_t i = 1; i < textLen; i++) {
        bodyCharBuffer[i-1] = bodyCharBuffer[i];
    }
    bodyCharBuffer[textLen-1] = first;
}

void IC_DISPLAY::setBody() {
    uint8_t strLen = 0;
    char displayBuffer[12];
    memset(displayBuffer, 0x00, sizeof(displayBuffer));
    if (inDiagMode) {
        strLen = min(MAX_IC_BODY_CHARS, diagBuffer.length());
        for (int i = 0; i < strLen; i++) {
            displayBuffer[i] = diagBuffer[i];
        }
    } else {
        strLen = min(MAX_IC_BODY_CHARS, strlen(bodyCharBuffer));
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
    for(uint8_t i = 0; i < min(MAX_IC_BODY_CHARS, strLen) - 6; i++) framePayload[17+i] = displayBuffer[i+6];

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

void IC_DISPLAY::diagSetHeader() {
    switch (diagPage)
    {
    case 0:
        setHeader("MAIN");
        break;
    case 1:
        setHeader("SPD ");
        break;
    case 2:
        setHeader("RPM ");
        break;
    case 3:
        setHeader("CLNT");
        break;
    case 4:
        setHeader("BHP");
        break;
    case 5:
        setHeader("TORQ");
        break;
    default:
        break;
    }
}