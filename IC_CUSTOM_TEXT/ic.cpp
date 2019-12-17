#include "ic.h"
#include "debug.h"
#include "avr/pgmspace.h"

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


bool IC_DISPLAY::textCanFit(const char* chars) { 
    uint8_t l = 0;
    for (uint8_t i = 0; i < strlen(chars); i++) {
        l += pgm_read_byte_near(ASCII_WIDTHS + (uint8_t) chars[i]);
    }
    return l-- < IC_WIDTH_PIXELS; // Minus 1 on return value as last char has no padding
}


byte IC_DISPLAY::page;
IC_DISPLAY::IC_DISPLAY(CanbusComm *c, EngineData *d) {
    this-> c = c;
}

void IC_DISPLAY::setHeader(DISPLAY_PAGE p, bool shouldCenter, const char* header) {
    uint8_t buffer[20];
    uint8_t slen = strlen(header);
    uint8_t len = slen + 3;
    if (slen > 13) slen = 12;
    buffer[0] = p == AUDIO ? 0x03 : 0x05;
    buffer[1] = 0x29;
    buffer[2] = shouldCenter ? 0x10 : 0x00;
    for (uint8_t i = 0; i < slen; i++) {
        buffer[i+3] = header[i];
    }
    buffer[len] = 0x00;
    buffer[len+1] = calculateChecksum(len, buffer);
    sendPacketsISO(len+=2, buffer);
}

void IC_DISPLAY::initPage(DISPLAY_PAGE p, IC_DISPLAY::SYMBOL upper, IC_DISPLAY::SYMBOL lower , bool shouldCenter, const char* header) {
    delay(350);
    uint8_t buffer[64];
    uint8_t slen = strlen(header);
    if (slen > 13) slen = 12;
    uint8_t len = 17 + slen + 2;
    buffer[0] = p == AUDIO ? 0x03 : 0x05;
    buffer[1] = 0x24;
    buffer[2] = 0x02;
    buffer[3] = 0x60;
    buffer[4] = 0x01;

    buffer[5] = 0x04; // Number of strs

    buffer[6] = 0x00;
    buffer[7] = 0x00;
    buffer[8] = 0x00;
    buffer[9] = 0x13;

    buffer[10] = upper; // Symbols (TOP)
    buffer[11] = 0x01;

    buffer[12] = lower; // Symbols (BOTTOM)
    buffer[13] = 0x02;

    buffer[14] = 0x00;
    buffer[15] = slen + 2;
    buffer[16] = shouldCenter ? 0x10 : 0x00;
    for (uint8_t i = 0; i < slen; i++) {
        buffer[i+17] = header[i];
    }
    buffer[len-2] = 0x00;
    buffer[len-1] = calculateChecksum(len-2, buffer);
    sendPacketsISO(len, buffer);
    delay(50);
}

void IC_DISPLAY::sendPacketsISO(uint8_t byteCount, uint8_t* bytes) {
    can_frame x;
    x.can_dlc = 8;
    x.can_id = IC_DISPLAY_ID;
    // Too many Bytes check
    if (byteCount > 55) {
        Serial.println(F("IC PAYLOAD TOO BIG (> 55 Bytes)"));
    } 
    else if (byteCount == 0) {
        Serial.println(F("IC PAYLOAD IS EMPTY!"));
    }
    // Send 1 frame as 7 or less bytes 
    else if (byteCount < 7) {
        x.data[0] = byteCount;
        for (uint8_t i = 1; i < 8; i++) {
            x.data[i] = bytes[i-1];
        }
        c->sendFrame(CAN_BUS_B, &x);
        delay(7);
    } else {
        x.data[0] = 0x10;
        x.data[1] = byteCount;
        for (uint8_t i = 2; i < 8; i++) x.data[i] = bytes[i-2];
        c->sendFrame(CAN_BUS_B, &x);
        delay(7);
        x.data[0] = 0x21;
        for (uint8_t i = 1; i < 8; i++) x.data[i] = bytes[i+5];
        c->sendFrame(CAN_BUS_B, &x);
        delay(2);
        if (byteCount > 13) {
            x.data[0] = 0x22;
            for (uint8_t i = 1; i < 8; i++) x.data[i] = bytes[i+12];
            c->sendFrame(CAN_BUS_B, &x);
            delay(2);
        }
        if (byteCount > 20) {
            x.data[0] = 0x23;
            for (uint8_t i = 1; i < 8; i++) x.data[i] = bytes[i+19];
            c->sendFrame(CAN_BUS_B, &x);
            delay(2);
        }
        if (byteCount > 27) {
            x.data[0] = 0x24;
            for (uint8_t i = 1; i < 8; i++) x.data[i] = bytes[i+26];
            c->sendFrame(CAN_BUS_B, &x);
            delay(2);
        }
        if (byteCount > 34) {
            x.data[0] = 0x25;
            for (uint8_t i = 1; i < 8; i++) x.data[i] = bytes[i+33];
            c->sendFrame(CAN_BUS_B, &x);
            delay(2);
        }
        if (byteCount > 41) {
            x.data[0] = 0x26;
            for (uint8_t i = 1; i < 8; i++) x.data[i] = bytes[i+40];
            c->sendFrame(CAN_BUS_B, &x);
            delay(2);
        }
        if (byteCount > 48) {
            x.data[0] = 0x27;
            for (uint8_t i = 1; i < 8; i++) x.data[i] = bytes[i+47];
            c->sendFrame(CAN_BUS_B, &x);
            delay(2);
        }
        delay(10);
    }
}

uint8_t IC_DISPLAY::calculateChecksum(uint8_t size, uint8_t* bytes) {
    uint8_t hash = 0xFF;
    for (uint8_t i = 0; i <= size; i++) hash -= i + bytes[i];
    return hash;
}

void IC_DISPLAY::setbodyText(DISPLAY_PAGE p, bool centerText, const char* line1, const char* line2, const char* line3, const char* line4) {
    uint8_t bytes[64];
    uint8_t len1 = strlen(line1);
    uint8_t len2 = strlen(line2);
    uint8_t len3 = strlen(line3);
    uint8_t len4 = strlen(line4);
    uint8_t arrLen = 7;
    bool has1 = len1 > 1;
    bool has2 = len2 > 1;
    bool has3 = len3 > 1;
    bool has4 = len4 > 1;
    // Break early if no text
    if (!has1) {
        return;
    }
    bytes[0] = p == AUDIO ? 0x03 : 0x05;
    bytes[1] = 0x26;
    bytes[2] = 0x01;
    bytes[3] = 0x00;
    bytes[4] = 1 + has2 + has3 + has4;
    bytes[5] = len1 + 2;
    bytes[6] = centerText ? 0x10 : 0x00;
    for (uint8_t i = 0; i < len1; i++) bytes[i+7] = line1[i];
    arrLen += len1;
    // IC Does not accept more than 1 line on Audio page
    if (has2 && p == TELEPHONE) {
        bytes[arrLen] = len2+2;
        bytes[arrLen+1] = centerText ? 0x10 : 0x00;
        arrLen+=2;
        for (uint8_t i = 0; i < len2; i++) {
            bytes[arrLen+i] = line2[i];
        }
        arrLen += len2;
        bytes[arrLen] = 0x02;
        bytes[arrLen+1] = 0x10;
        bytes[arrLen+2] = 0x02;
        bytes[arrLen+3] = 0x10;
        arrLen += 3;
    }
    bytes[arrLen] = 0x00;
    bytes[arrLen + 1] = calculateChecksum(arrLen, bytes);
    arrLen+=2;
    sendPacketsISO(arrLen, bytes);
}

void IC_DISPLAY::setSymbols(DISPLAY_PAGE p, SYMBOL top, SYMBOL bottom){
    // For now, only AUDIO page supports symbols
    if (p == AUDIO) {
        uint8_t buffer[8] = { 0x03, 0x28, 0x01, top, 0x01, bottom, 0x02 };
        buffer[7] = calculateChecksum(7, buffer);
        sendPacketsISO(7, buffer);
    }
}