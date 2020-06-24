#include "ic_display.h"
#include "can.h"

IC_DISPLAY::IC_DISPLAY(CANBUS_COMMUNICATOR *can) {
    canB = can;
}

uint8_t IC_DISPLAY::current_page = IC_PAGE_AUDIO;

bool IC_DISPLAY::can_fit_body_text(const char *text) {
    int length = 0;
    for (uint8_t i = 0; i < strlen(text); i++) {
        length += pgm_read_byte_near(CHAR_WIDTHS + text[i]);
    }
    return (length - 1) <= DISPLAY_WIDTH_PX; // -1 as last char doesn't need 1 pixel padding
}

uint8_t IC_DISPLAY::getChecksum(uint8_t len, uint8_t* payload) {
    uint8_t cs = 0xFF;
    for (uint8_t i = 0; i <= len; i++) cs -= i + payload[i];
    return cs;
}

void IC_DISPLAY::setHeader(uint8_t p, const char* text, uint8_t fmt) {
    if (strlen(text) == 0) return;

    DPRINTLN(F("-- Update header --"));
    uint8_t str_len = min(strlen(text), 20);
    buffer_size = str_len + 3;
    buffer[0] = p; // Page number
    buffer[1] = 0x29; // Package 29 (Header text update)
    buffer[2] = fmt; // Text justification
    for (uint8_t i = 0; i < str_len; i++) {
        buffer[i+3] = text[i];
    }
    buffer[buffer_size] = 0x00;
    buffer[buffer_size+1] = getChecksum(buffer_size, buffer);
    buffer_size+=2;
    sendBytes(0,0);
}

void IC_DISPLAY::setBody(uint8_t p, const char* text, uint8_t fmt = IC_TEXT_FMT_CENTER_JUSTIFICATION) {
    if (strlen(text) == 0) return;

    DPRINTLN(F("-- Update body --"));
    uint8_t str_len = min(strlen(text), 32);
    buffer_size = str_len + 7; // Not including CS bit
    buffer[0] = p; // Page number
    buffer[1] = 0x26; // Package 26 (Body text update)
    buffer[2] = 0x01; 
    buffer[3] = 0x00;
    buffer[4] = 0x01; // Number of strings (1 only)
    buffer[5] = str_len + 2;
    buffer[6] = fmt;
    for (uint8_t i = 0; i < str_len; i++) {
        buffer[i+7] = text[i];
    }
    buffer[buffer_size] = 0x00; // End of string
    buffer[buffer_size+1] = getChecksum(buffer_size, buffer);
    buffer_size+=2;
    sendBytes(0,0);
}

void IC_DISPLAY::setBodyTel(uint8_t numStrs, const char* lines[]){
    uint8_t charCount = 0;
    uint8_t strsToUse = 0;
    for (uint8_t i = 0; i < numStrs; i++) {
        charCount += strlen(lines[i]) + 2;
        if (charCount < 55) {
            strsToUse++;
        }
    }

    if (strsToUse == 0) {
        return;
    }

    DPRINTLN(F("-- Update body --"));
    buffer_size = charCount + 5; // Not including CS bit
    buffer[0] = IC_PAGE_TELEPHONE; // Page number
    buffer[1] = 0x26; // Package 26 (Body text update)
    buffer[2] = 0x01; 
    buffer[3] = 0x00;
    buffer[4] = strsToUse; // Number of strings (1 only)
    uint8_t index = 5;
    for (uint8_t i = 0; i < strsToUse; i++) {
        buffer[index] = strlen(lines[i]) + 2;
        buffer[index+1] = IC_TEXT_FMT_CENTER_JUSTIFICATION | IC_TEXT_FMT_HIGHLIGHTED;
        index += 2;
        for (uint8_t x = 0; x < strlen(lines[i]); x++) {
            buffer[index] = lines[i][x];
            index++;
        }
    }
    buffer[buffer_size] = 0x00; // End of string
    buffer[buffer_size+1] = getChecksum(buffer_size, buffer);
    buffer_size+=2;
    sendBytes(0,0);
}

void IC_DISPLAY::processIcResponse(can_frame *r) {
    if (r->can_id == 0x1D0) {
        DPRINTLN(IC_TO_AGW_STR+*canB->frame_to_string(r, false));
        // Some data relating to navigation sent to AGW
        if (r->data[0] == 0x06 && r->data[2] == 0x27) {
            // Audio Page
            if (r->data[1] == 0x03 && r->data[6] == 0xC4) { // Move in
                current_page = IC_PAGE_AUDIO;
            }
            else if (r->data[1] == 0x03 && r->data[6] == 0xC3) { // Move out
                current_page = IC_PAGE_OTHER;
            }

            // Telephone page
            if (r->data[1] == 0x05 && r->data[6] == 0xC2) { // Move in
                current_page = IC_PAGE_TELEPHONE;
            }
            else if (r->data[1] == 0x05 && r->data[6] == 0xC1) { // Move out
                current_page = IC_PAGE_OTHER;
            }
        }
    }
}


void IC_DISPLAY::initPage(uint8_t p, const char* header, uint8_t fmt, uint8_t upper_Symbol, uint8_t lower_Symbol, uint8_t numLines=1) {
    DPRINTLN(F("-- Init page --"));
    uint8_t str_len = min(strlen(header), 20);
    buffer_size = str_len + 17; // Not including CS bit
    buffer[0] = p; // Page number
    buffer[1] = 0x24; // Package 24 (Init page)
    buffer[2] = 0x02;
    buffer[3] = 0x60;
    buffer[4] = 0x00;
    buffer[5] = numLines;
    buffer[6] = 0x00;
    buffer[7] = 0x00;
    buffer[8] = 0x00;
    buffer[9] = 0x13;
    buffer[10] = IC_SYMB_UP_ARROW; // Upper symbol (Above text)
    buffer[11] = 0x01;
    buffer[12] = IC_SYMB_UP_ARROW; // Lower symbol (Under text)
    buffer[13] = 0x02;
    buffer[14] = 0x00;
    buffer[15] = str_len;   
    buffer[16] = fmt;
    for (uint8_t i = 0; i < str_len; i++) {
        buffer[i+17] = header[i];
    }
    buffer[buffer_size] = 0x00; // End of string
    buffer[buffer_size+1] = getChecksum(buffer_size, buffer);
    buffer_size+=2;
    sendBytes(400,400);
}

void IC_DISPLAY::sendBytes(int pre_delay, int post_delay) {
    wait(pre_delay);
    x.can_id = SEND_CAN_ID;
    x.can_dlc = 0x08;
    if (buffer_size == 0) {
        return;
    } else if (buffer_size >= 55) {
        Serial.println(F("Payload too big!"));
        return;
    }
    if (buffer_size < 8) {
        x.data[0] = buffer_size;
        for (uint8_t i = 0; i < buffer_size; i++) x.data[i+1] = buffer[i];
        DPRINTLN(AGW_TO_IC_STR+*canB->frame_to_string(&x, false));
        canB->sendToBus(&x);
        wait(post_delay);
        return;
    } else {
        x.data[0] = 0x10;
        x.data[1] = buffer_size;
        for (uint8_t i = 2; i < 8; i++) x.data[i] = buffer[i-2];
        DPRINTLN(AGW_TO_IC_STR+*canB->frame_to_string(&x, false));
        canB->sendToBus(&x);
        wait(5);
        x.data[0] = 0x21;
        for (uint8_t i = 1; i < 8; i++) x.data[i] = buffer[i+5];
        DPRINTLN(AGW_TO_IC_STR+*canB->frame_to_string(&x, false));
        canB->sendToBus(&x);
        wait(2);
        if (buffer_size > 13) {
            x.data[0] = 0x22;
            for (uint8_t i = 1; i < 8; i++) x.data[i] = buffer[i+12];
            DPRINTLN(AGW_TO_IC_STR+*canB->frame_to_string(&x, false));
            canB->sendToBus(&x);
            wait(2);
        }
        if (buffer_size > 20) {
            x.data[0] = 0x23;
            for (uint8_t i = 1; i < 8; i++) x.data[i] = buffer[i+19];
            DPRINTLN(AGW_TO_IC_STR+*canB->frame_to_string(&x, false));
            canB->sendToBus(&x);
            wait(2);
        }
        if (buffer_size > 27) {
            x.data[0] = 0x24;
            for (uint8_t i = 1; i < 8; i++) x.data[i] = buffer[i+26];
            DPRINTLN(AGW_TO_IC_STR+*canB->frame_to_string(&x, false));
            canB->sendToBus(&x);
            wait(2);
        }
        if (buffer_size > 34) {
            x.data[0] = 0x25;
            for (uint8_t i = 1; i < 8; i++) x.data[i] = buffer[i+33];
            DPRINTLN(AGW_TO_IC_STR+*canB->frame_to_string(&x, false));
            canB->sendToBus(&x);
            wait(2);
        }
        if (buffer_size > 41) {
            x.data[0] = 0x26;
            for (uint8_t i = 1; i < 8; i++) x.data[i] = buffer[i+40];
            DPRINTLN(AGW_TO_IC_STR+*canB->frame_to_string(&x, false));
            canB->sendToBus(&x);
            wait(2);
        }
        if (buffer_size > 48) {
            x.data[0] = 0x27;
            for (uint8_t i = 1; i < 8; i++) x.data[i] = buffer[i+47];
            DPRINTLN(AGW_TO_IC_STR+*canB->frame_to_string(&x, false));
            canB->sendToBus(&x);
            wait(2);
        }
        delay(post_delay+10);
    }
}

void IC_DISPLAY::wait(uint8_t msec) {
    unsigned long endTime = millis() + msec;
    while (millis() <= endTime) {
        processIcResponse(canB->read_frame());
    }
}