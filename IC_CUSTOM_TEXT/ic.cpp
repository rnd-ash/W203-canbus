//
// Created by ashcon on 10/5/19.
//

#include "ic.h"
#include "debug.h"

/**
 * Sets the refresh rate for scrolling text. If text is static (8 chars),
 * refresh rate is locked to every 2 seconds
 * 
 * @param rate Desired rate to scroll across text
 */
void IC_DISPLAY::setRefreshRate(int rate) {
    this->scrollRefreshRate = rate;
    // If text is shorter than MAX_STR_LENGTH, then leave the refresh rate as static interval
    if (currText.length() > MAX_STR_LENGTH) {
        this->currentRefreshRate = this->scrollRefreshRate;
    } else {
        this->currentRefreshRate = this->staticRefreshRate;
    }
}

/**
 * Static value to hold the current page displayed on the IC display
 */
IC_DISPLAY::clusterPage IC_DISPLAY::currentPage = Audio;


IC_DISPLAY::IC_DISPLAY(CanbusComm *c) {
    this->sendFirst = false;
    this->c = c;
    this->staticRefreshRate = 2000;
    this->scrollRefreshRate = 125;
    this->setBodyText("NO BLUETOOTH CONNECTION!");
    this->lastTime = millis();
}

/**
 * Displays the 3 character header text on the IC page
 * @param text Chars to display as the header
 */
void IC_DISPLAY::sendHeader(const char text[3]) {
    curr_frame.can_id = IC_SEND_PID;
    curr_frame.can_dlc = 0x08;
    int checkSumBit = calculateHeaderCheckSum(text);
    
    curr_frame.data[0] = 0x10;
    curr_frame.data[1] = 0x09;
    curr_frame.data[2] = 0x03;
    curr_frame.data[3] = 0x29;
    curr_frame.data[4] = 0x00;
    curr_frame.data[5] = text[0];
    curr_frame.data[6] = text[1];
    curr_frame.data[7] = text[2];
    c->sendFrame(CAN_BUS_B, &curr_frame);
    delay(7);

    curr_frame.data[0] = 0x21;
    curr_frame.data[1] = 0x20;
    curr_frame.data[2] = 0x00;
    curr_frame.data[3] = checkSumBit;
    curr_frame.data[4] = 0x00;
    curr_frame.data[5] = 0x01;
    curr_frame.data[6] = 0x12;
    curr_frame.data[7] = 0xC0;
    c->sendFrame(CAN_BUS_B, &curr_frame);
    delay(7);
}

/**
 * Sends body packets to the IC display to display a maximum of MAX_STR_LENGTH characters at a lastTime
 * @param text Text to be displayed. If longer than 8 characters, it is cropped to be 8 characters long
 */
void IC_DISPLAY::sendBody(String text) {
    curr_frame.can_id = IC_SEND_PID;
    curr_frame.can_dlc = 0x08;
    String msg = "";
    // If string is less than 5 chars, pad it out with spaces to 5 characters
    if (text.length() < 5) {
        msg = text;
        for (int i = 0; i < 5 - text.length(); i++) {
            msg += " ";
        }
    }
    // If text is longer than MAX_STR_LENGTH chars, crop it to MAX_STR_LENGTH chars long 
    else if (text.length() > MAX_STR_LENGTH) {
        for (int i = 0; i < MAX_STR_LENGTH; i++) {
            msg += text[i];
        }
    }
    // String is a valid length. No modification needed 
    else {
        msg = text;
    }
    // Stores the Sum of all ASCII Chars in text. Needed for validation packet
    int asciiTotal = 0;
    // Stores the length of the string data. Needed in the pre-text packet
    int checkSumB = msg.length() + 2;
    int checkSumA = 7 + checkSumB;
    
    // Buffer to hold data for validation packet and text packet
    uint8_t bodyData[11] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    // fill up bodyData with text data
    for (int i = 0; i < msg.length(); i++) {
        bodyData[i] = msg[i];
        asciiTotal += msg[i];
    }
    // Add null termination to the bodydata
    bodyData[msg.length()] = 0x00;
    // Finally, add the validation byte to the bodyData
    bodyData[msg.length() + 1] = calculateBodyCheckSum(msg);

    // Pre-text packet for body text
    curr_frame.data[0] = 0x10;
    curr_frame.data[1] = checkSumA;
    curr_frame.data[2] = 0x03;
    curr_frame.data[3] = 0x26;
    curr_frame.data[4] = 0x01;
    curr_frame.data[5] = 0x00;
    curr_frame.data[6] = 0x01;
    curr_frame.data[7] = checkSumB;
    c->sendFrame(CAN_BUS_B, &curr_frame);
    delay(7); // Wait 5 MS for IC to process request (+2ms for time taken for the IC to receive packet)

    curr_frame.data[0] = 0x21;
    curr_frame.data[1] = 0x10;
    for (int i = 2; i <=7; i++) {
        curr_frame.data[i] = bodyData[i-2];
    }
    c->sendFrame(CAN_BUS_B, &curr_frame);
    delay(2);

    curr_frame.data[0] = 0x22;
    for (int i = 1; i <= 5; i++) {
        curr_frame.data[i] = bodyData[i+5];
    }

    curr_frame.data[6] = 0x01;
    curr_frame.data[7] = 0x08;
    c->sendFrame(CAN_BUS_B, &curr_frame);
    delay(2);
}

/**
 * calculates the header text validation byte
 */
uint8_t IC_DISPLAY::calculateHeaderCheckSum(const char *text) {
    int sum = 0;
    for (int i = 0; i < sum; i++) {
        sum += int(text[i]);
    }
    return 407 - sum;
}

/**
 * Calculates the body text validation byte
 */
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
                DPRINTLN("K: "+String(k)+" SUM: "+String(strTotal)+" Num Chars: 9");
                return (k-strTotal);
            }
        }
    }else if(charCount == 8) {
        for(int k : EIGHT_CHAR_TOTAL_LOOKUP) {
            if(k - strTotal <= 256) {
                DPRINTLN("K: "+String(k)+" SUM: "+String(strTotal)+" Num Chars: 8");
                return (k-strTotal);
            }
        }
    } else if (charCount == 7) {
        for(int k : SEVEN_CHAR_TOTAL_LOOKUP) {
            if(k - strTotal <= 256) {
                DPRINTLN("K: "+String(k)+" SUM: "+String(strTotal)+" Num Chars: 7");
                return (k-strTotal);
            }
        }
    } else if (charCount == 6) {
        for(int k : SIX_CHAR_TOTAL_LOOKUP) {
            if(k - strTotal <= 256) {
                DPRINTLN("K: "+String(k)+" SUM: "+String(strTotal)+" Num Chars: 6");
                return (k-strTotal);
            }
        }
    } else if (charCount == 5) {
        for(int k : FIVE_CHAR_TOTAL_LOOKUP) {
            if(k - strTotal <= 256) {
                DPRINTLN("K: "+String(k)+" SUM: "+String(strTotal)+" Num Chars: 5");
                return (k-strTotal);
            }
        }
    }
    return 0;
}


/**
 * Method to update the contents of the IC based on data within the class
 */
void IC_DISPLAY::update() {
    if (millis() - lastTime >= currentRefreshRate) {
        lastTime = millis();
        if(currentPage == clusterPage::Audio) {
            sendBody(currText);
            if(this->currText.length() > MAX_STR_LENGTH) {
                this->currText = shiftString();
            }
        } else if (!sendFirst) {
            this->sendHeader("EXP");
            sendBody(currText);
            this->sendFirst = true;
        }
    }
}

/**
 * Sets body text for the IC
 */
void IC_DISPLAY::setBodyText(String text) {
    this->currText = text;
    if (text.length() > MAX_STR_LENGTH) {
        this->currText += "   ";
        this->currentRefreshRate = scrollRefreshRate;
    } else {
        this->sendBody(currText);
        this->currentRefreshRate = staticRefreshRate;
    }
    this->sendFirst = false;
}

/**
 * Shifts string by 1 to the left
 */
String IC_DISPLAY::shiftString() {
    char x = currText[0];
    String tmp;
    for (int i = 1; i < currText.length(); i++) {
        tmp += currText[i];
    }
    tmp += x;
    return tmp;
}
