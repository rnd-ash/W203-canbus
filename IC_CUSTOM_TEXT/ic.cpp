#include "ic.h"
#include "debug.h"

IC_DISPLAY::IC_DISPLAY(CanbusComm *c) {
    char displayBuffer[10];
    char charBuffer[256];
    this-> c = c;
    this->inDiagMode = false;
    this->lastUpdateMillis = millis();
    this->curr_frame.can_id = IC_DISPLAY_ID;
    this->curr_frame.can_dlc = 0x08;
    this->setBody("NO BT");
    this->isSending = false;
    this->diagPage = 0;
    this->d = new DIAG_DISPLAY(this->c);
    this->diagBuffer.reserve(9);

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
    sendBody();
}
void IC_DISPLAY::setDiagText() {
    switch (diagPage)
    {
    case 0:
        diagBuffer = "DIAG MODE";
        break;
    case 1:
        diagBuffer = d->getSpeed();
        break;
    case 2:
        diagBuffer = d->getRPM();
        break;
    case 3:
        diagBuffer = d->getCoolantTemp();
        break;
    default:
        break;
    }
}


void IC_DISPLAY::update() {
    if (millis() - lastKeepAliveMillis >= KWP2000_KEEP_ALIVE_FREQ) {
        lastKeepAliveMillis = millis();
        DPRINTLN(F("Keeping KWP2000 diagnostic mode alive!"));
        c->sendFrame(CAN_BUS_B, &diag_frame);
    }
    if (inDiagMode) {
        if (millis() - lastUpdateMillis > DIAG_MODE_UPDATE_FREQ) {
            setDiagText();
            lastUpdateMillis = millis();
            sendBody();
        }
    }
    else if (textLen > MAX_IC_BODY_CHARS) {
        if (millis() - lastUpdateMillis > SCROLLING_UPDATE_FREQ) {
            lastUpdateMillis = millis();
            sendBody();
            this->shiftText();
        }
    } else {
        if (millis() - lastUpdateMillis > STATIC_UPDATE_FREQ) {
            lastUpdateMillis = millis();
            sendBody();
            //sendHeader();
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

void IC_DISPLAY::sendBody() {
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
    uint8_t framePayload[24] = {0x00};
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

    for (uint8_t i = 0; i <= 7; i++) {
        curr_frame.data[i] = framePayload[i];
    }
    c->sendFrame(CAN_BUS_B, &curr_frame);
    delay(7);
    for (uint8_t i = 8; i <= 15; i++) {
        curr_frame.data[i-8] = framePayload[i];
    }
    c->sendFrame(CAN_BUS_B, &curr_frame);
    delay(2);

    for (uint8_t i = 16; i <= 23; i++) {
        curr_frame.data[i-16] = framePayload[i];
    }
    c->sendFrame(CAN_BUS_B, &curr_frame);
    delay(2);

    DPRINTLN("SENT "+String(displayBuffer)+" TO IC");
}

void IC_DISPLAY::nextDiagPage() {
    if (diagPage == d->screens) {
        diagPage = 0;
    } else {
        diagPage++;
    }
}

void IC_DISPLAY::prevDiagPage(){
    if (diagPage == 0) {
        diagPage = d->screens;
    } else {
        diagPage--;
    }
}