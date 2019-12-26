#include "ic_display.h"
#include "can.h"

IC_DISPLAY::IC_DISPLAY(CANBUS_COMMUNICATOR *can) {
    canB = can;
}

IC_DISPLAY::PAGE IC_DISPLAY::current_page = IC_DISPLAY::AUDIO;

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

void IC_DISPLAY::setHeader(PAGE p, const char* text, bool should_center) {
    DPRINTLN(F("-- Update header --"));
    uint8_t str_len = min(strlen(text), 20);
    buffer_size = str_len + 3;
    buffer[0] = p; // Page number
    buffer[1] = 0x29; // Package 29 (Header text update)
    buffer[2] = should_center ? 0x10 : 0x00; // Text justification
    for (uint8_t i = 0; i < str_len; i++) {
        buffer[i+3] = text[i];
    }
    buffer[buffer_size] = 0x00;
    buffer[buffer_size+1] = getChecksum(buffer_size, buffer);
    buffer_size+=2;
    sendBytes(0,0);
}

void IC_DISPLAY::setBody(PAGE p, const char* text, bool should_center = true) {
    DPRINTLN(F("-- Update body --"));
    uint8_t str_len = min(strlen(text), 12);
    buffer_size = str_len + 7; // Not including CS bit
    buffer[0] = p; // Page number
    buffer[1] = 0x26; // Package 26 (Body text update)
    buffer[2] = 0x01; 
    buffer[3] = 0x00;
    buffer[4] = 0x01; // Number of strings (1 only)
    buffer[5] = str_len + 2;
    buffer[6] = should_center ? 0x10 : 0x00; // Text justification
    for (uint8_t i = 0; i < str_len; i++) {
        buffer[i+7] = text[i];
    }
    buffer[buffer_size] = 0x00; // End of string
    buffer[buffer_size+1] = getChecksum(buffer_size, buffer);
    buffer_size+=2;
    sendBytes(0,0);
}

void IC_DISPLAY::processIcResponse(can_frame *r) {
    // Some data relating to navigation sent to AGW
    if (r->can_id == 0x1D0 && r->data[0] == 0x06 && r->data[2] == 0x27) {
        // Audio Page
        if (r->data[1] == 0x03 && r->data[6] == 0xC4) { // Move in
            current_page = AUDIO;
        }
        else if (r->data[1] == 0x03 && r->data[6] == 0xC3) { // Move out
            current_page = OTHER;
        }

        // Telephone page
        if (r->data[1] == 0x05 && r->data[6] == 0xC2) { // Move in
            current_page = TELEPHONE;
        }
        else if (r->data[1] == 0x05 && r->data[6] == 0xC1) { // Move out
            current_page = OTHER;
        }
    }
}


void IC_DISPLAY::initPage(PAGE p, const char* header, bool should_center, IC_SYMBOL upper_Symbol, IC_SYMBOL lower_Symbol) {
    DPRINTLN(F("-- Init page --"));
    uint8_t str_len = min(strlen(header), 20);
    buffer_size = str_len + 17; // Not including CS bit
    buffer[0] = p; // Page number
    buffer[1] = 0x24; // Package 24 (Init page)
    buffer[2] = 0x02;
    buffer[3] = 0x60;
    buffer[4] = 0x01;
    buffer[5] = 0x01;
    buffer[6] = 0x00;
    buffer[7] = 0x00;
    buffer[8] = 0x00;
    buffer[9] = 0x13;
    buffer[10] = upper_Symbol; // Upper symbol (Above text)
    buffer[11] = 0x01;
    buffer[12] = lower_Symbol; // Lower symbol (Under text)
    buffer[13] = 0x02;
    buffer[14] = 0x00;
    buffer[15] = str_len;
    buffer[16] = should_center ? 0x10 : 0x00;
    for (uint8_t i = 0; i < str_len; i++) {
        buffer[i+17] = header[i];
    }
    buffer[buffer_size] = 0x00; // End of string
    buffer[buffer_size+1] = getChecksum(buffer_size, buffer);
    buffer_size+=2;
    sendBytes(300,300);
}

void IC_DISPLAY::sendBytes(int pre_delay, int post_delay) {
    delay(post_delay);
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
        delay(post_delay);
        return;
    } else {
        x.data[0] = 0x10;
        x.data[1] = buffer_size;
        for (uint8_t i = 2; i < 8; i++) x.data[i] = buffer[i-2];
        DPRINTLN(AGW_TO_IC_STR+*canB->frame_to_string(&x, false));
        canB->sendToBus(&x);
        delay(5);
        x.data[0] = 0x21;
        for (uint8_t i = 1; i < 8; i++) x.data[i] = buffer[i+5];
        DPRINTLN(AGW_TO_IC_STR+*canB->frame_to_string(&x, false));
        canB->sendToBus(&x);
        delay(2);
        if (buffer_size > 13) {
            x.data[0] = 0x22;
            for (uint8_t i = 1; i < 8; i++) x.data[i] = buffer[i+12];
            DPRINTLN(AGW_TO_IC_STR+*canB->frame_to_string(&x, false));
            canB->sendToBus(&x);
            delay(2);
        }
        if (buffer_size > 20) {
            x.data[0] = 0x23;
            for (uint8_t i = 1; i < 8; i++) x.data[i] = buffer[i+19];
            DPRINTLN(AGW_TO_IC_STR+*canB->frame_to_string(&x, false));
            canB->sendToBus(&x);
            delay(2);
        }
        if (buffer_size > 27) {
            x.data[0] = 0x24;
            for (uint8_t i = 1; i < 8; i++) x.data[i] = buffer[i+26];
            DPRINTLN(AGW_TO_IC_STR+*canB->frame_to_string(&x, false));
            canB->sendToBus(&x);
            delay(2);
        }
        if (buffer_size > 34) {
            x.data[0] = 0x25;
            for (uint8_t i = 1; i < 8; i++) x.data[i] = buffer[i+33];
            DPRINTLN(AGW_TO_IC_STR+*canB->frame_to_string(&x, false));
            canB->sendToBus(&x);
            delay(2);
        }
        if (buffer_size > 41) {
            x.data[0] = 0x26;
            for (uint8_t i = 1; i < 8; i++) x.data[i] = buffer[i+40];
            DPRINTLN(AGW_TO_IC_STR+*canB->frame_to_string(&x, false));
            canB->sendToBus(&x);
            delay(2);
        }
        if (buffer_size > 48) {
            x.data[0] = 0x27;
            for (uint8_t i = 1; i < 8; i++) x.data[i] = buffer[i+47];
            DPRINTLN(AGW_TO_IC_STR+*canB->frame_to_string(&x, false));
            canB->sendToBus(&x);
            delay(2);
        }
        delay(post_delay+10);
    }
}