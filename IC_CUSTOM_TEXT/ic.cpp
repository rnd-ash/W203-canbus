//
// Created by ashcon on 10/5/19.
//

#include "ic.h"

void IC_DISPLAY::setRefreshRate(int rate) {
    this->scrollRefreshRate = rate;
    if (currText.length() > ) {
        this->currentRefreshRate = this->scrollRefreshRate;
    } else {
        this->currentRefreshRate = this->staticRefreshRate;
    }
}

IC_DISPLAY::clusterPage IC_DISPLAY::currentPage = Audio;

IC_DISPLAY::IC_DISPLAY(CanbusComm *c) {
    this->sendFirst = false;
    this->c = c;
    this->staticRefreshRate = 2000;
    this->scrollRefreshRate = 125;
    this->setBodyText("NO BLUETOOTH CONNECTION!");
    this->lastTime = millis();
}

void IC_DISPLAY::sendHeader(const char text[3]) {
    curr_frame.can_id = IC_SEND_PID;
    curr_frame.can_dlc = 0x08;

    int checkSumTotal = 0;
    for (int i = 0; i < 3; i++) {
        checkSumTotal += text[i];
    }
    int checkSumBit = 407 - checkSumTotal;
    
    curr_frame.data[0] = 0x10;
    curr_frame.data[1] = 0x09;
    curr_frame.data[2] = 0x03;
    curr_frame.data[3] = 0x29;
    curr_frame.data[4] = 0x00;
    curr_frame.data[5] = text[0];
    curr_frame.data[6] = text[1];
    curr_frame.data[7] = text[2];
    c->sendFrame(&curr_frame);
    delay(7);

    curr_frame.data[0] = 0x21;
    curr_frame.data[1] = 0x20;
    curr_frame.data[2] = 0x00;
    curr_frame.data[3] = checkSumBit;
    curr_frame.data[4] = 0x00;
    curr_frame.data[5] = 0x01;
    curr_frame.data[6] = 0x12;
    curr_frame.data[7] = 0xC0;
    c->sendFrame(&curr_frame);
    delay(7);
}

/*
 * Sending body text packets
 * Send PAC 1
 * Receive Response
 * Send Pac 2
 * Send Pac 3
 * Receive response
 */
void IC_DISPLAY::sendBody(String text) {
    curr_frame.can_id = IC_SEND_PID;
    curr_frame.can_dlc = 0x08;
    String msg = "";
    if (text.length() < 5) {
        msg = text;
        for (int i = 0; i < 5 - text.length(); i++) {
            msg += " ";
        }
    } else if (text.length() > MAX_STR_LENGTH) {
        for (int i = 0; i < MAX_STR_LENGTH; i++) {
            msg += text[i];
        }
    } else {
        msg = text;
    }
    int asciiTotal = 0;
    int checkSumB = msg.length() + 2;
    int checkSumA = 7 + checkSumB;
    // Placeholder - Need to work out how to calculate this
    
    uint8_t bodyData[11] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    for (int i = 0; i < msg.length(); i++) {
        bodyData[i] = msg[i];
        asciiTotal += msg[i];
    }
    uint8_t x = calculateBodyCheckSum(msg);
    bodyData[msg.length()] = 0x00;
    bodyData[msg.length() + 1] = int(x);

    curr_frame.data[0] = 0x10;
    curr_frame.data[1] = checkSumA;
    curr_frame.data[2] = 0x03;
    curr_frame.data[3] = 0x26;
    curr_frame.data[4] = 0x01;
    curr_frame.data[5] = 0x00;
    curr_frame.data[6] = 0x01;
    curr_frame.data[7] = checkSumB;
    c->sendFrame(&curr_frame);
    delay(7);

    curr_frame.data[0] = 0x21;
    curr_frame.data[1] = 0x10;
    for (int i = 2; i <=7; i++) {
        curr_frame.data[i] = bodyData[i-2];
    }
    c->sendFrame(&curr_frame);
    delay(2);

    curr_frame.data[0] = 0x22;
    for (int i = 1; i <= 5; i++) {
        curr_frame.data[i] = bodyData[i+5];
    }

    curr_frame.data[6] = 0x01;
    curr_frame.data[7] = 0x08;
    c->sendFrame(&curr_frame);
    delay(2);
}

uint8_t IC_DISPLAY::calculateHeaderCheckSum(const char *text) {
    return 0;
}

uint8_t IC_DISPLAY::calculateBodyCheckSum(String text){
    int charCount = text.length();
    // Lookup table valid checksum + ASCII Total values
    int NINE_CHAR_TOTAL_LOOKUP[] =  {1073, 817, 561, 561};
    int EIGHT_CHAR_TOTAL_LOOKUP[] = {1090, 834, 578, 322};
    int SEVEN_CHAR_TOTAL_LOOKUP[] = {1136, 880, 624, 368};
    int SIX_CHAR_TOTAL_LOOKUP[] =   {1121, 865, 609, 353};
    int FIVE_CHAR_TOTAL_LOOKUP[] =  {1135, 879, 623, 367};

    int strTotal = 0;
    for(int i = 0; i < charCount; i++) {
        strTotal += text[i];
    }

    if(charCount == 9) {
        for(int k : NINE_CHAR_TOTAL_LOOKUP) {
            if(k - strTotal <= 256) {
                return (k-strTotal);
            }
        }
    }else if(charCount == 8) {
        for(int k : EIGHT_CHAR_TOTAL_LOOKUP) {
            if(k - strTotal <= 256) {
                return (k-strTotal);
            }
        }
    } else if (charCount == 7) {
        for(int k : SEVEN_CHAR_TOTAL_LOOKUP) {
            if(k - strTotal <= 256) {
                return (k-strTotal);
            }
        }
    } else if (charCount == 6) {
        for(int k : SIX_CHAR_TOTAL_LOOKUP) {
            if(k - strTotal <= 256) {
                return (k-strTotal);
            }
        }
    } else if (charCount == 5) {
        for(int k : FIVE_CHAR_TOTAL_LOOKUP) {
            if(k - strTotal <= 256) {
                return (k-strTotal);
            }
        }
    }
    return 0;
}



void IC_DISPLAY::update() {
    if (millis() - lastTime >= currentRefreshRate) {
        lastTime = millis();
        if(currentPage == clusterPage::Audio) {
            sendBody(currText);
            this->currText = shiftString();
        } else if (!sendFirst) {
            this->sendHeader("EXP");
            sendBody(currText);
            this->sendFirst = true;
        }
    }
}

void IC_DISPLAY::setBodyText(String text) {
    this->currText = text;
    if (text.length() > MAX_STR_LENGTH) {
        this->currText += "   ";
        this->currentRefreshRate = scrollRefreshRate;
    } else {
        this->currentRefreshRate = staticRefreshRate;
    }
    this->sendFirst = false;
}

String IC_DISPLAY::shiftString() {
    char x = currText[0];
    String tmp;
    for (int i = 1; i < currText.length(); i++) {
        tmp += currText[i];
    }
    tmp += x;
    return tmp;
}
