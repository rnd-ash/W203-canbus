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
    this->diagData = "NO DATA!";
    this->inDiagMode = false;
    this->sendFirst = false;
    this->c = c;
    this->staticRefreshRate = 1000;
    this->scrollRefreshRate = 150;
    this->setBodyText("BT = NA!");
    this->lastTime = millis();
}

/**
 * Displays the 3 character header text on the IC page
 * @param text Chars to display as the header
 */
void IC_DISPLAY::sendHeader(String header) {
    String msg = resize(header, 4, 8);
    
    curr_frame.can_id = IC_SEND_PID;
    curr_frame.can_dlc = 0x08;
    uint8_t checkSumBit = calculateBodyCheckSum(header);
    uint8_t buffer[14] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    buffer[0] = msg.length() + 5;
    buffer[1] = msg.length() - 1;
    buffer[2] = 0x29;
    buffer[3] = 0x00;
    for (int i = 0; i < msg.length(); i++) {
        buffer[i+4] = msg[i];
    }
    buffer[msg.length()+4] = 0x00;
    buffer[msg.length()+5] = checkSumBit;

    curr_frame.data[0] = 0x10;
    for (int i = 1; i < 7; i++) {
        curr_frame.data[i+1] = buffer[i];
    }
    c->sendFrame(CAN_BUS_B, &curr_frame);
    delay(7);

    curr_frame.data[0] = 0x21;
    for (int i = 7; i < 14; i++) {
        curr_frame.data[i-6] = buffer[i];
    }
    c->sendFrame(CAN_BUS_B, &curr_frame);
    delay(2);
}
String IC_DISPLAY::resize(String s, int lb, int ub) {
    String ret = "";
    if (s.length() < lb) {
        ret = s;
        for (int i = 0; i < lb - s.length(); i++) {
            ret += " ";
        }
    }
    // If text is longer than MAX_STR_LENGTH chars, crop it to MAX_STR_LENGTH chars long 
    else if (s.length() > ub) {
        for (int i = 0; i < ub; i++) {
            ret += s[i];
        }
    } else {
        ret = s;
    }
    return ret;
}

/**
 * Sends body packets to the IC display to display a maximum of MAX_STR_LENGTH characters at a lastTime
 * @param text Text to be displayed. If longer than 8 characters, it is cropped to be 8 characters long
 */
void IC_DISPLAY::sendBody(String text) {
    curr_frame.can_id = IC_SEND_PID;
    curr_frame.can_dlc = 0x08;
    String msg = resize(text, 5, MAX_STR_LENGTH);
    // Stores the Sum of all ASCII Chars in text. Needed for validation packet
    int asciiTotal = 0;
    // Stores the number of bytes for actual data for the IC to process. Message length (ASCII) + Validation byte + null termination byte
    uint8_t totalMsgLength = msg.length() + 2;
    // Number of bytes to send across multiple packets to the IC
    uint8_t numberOfBytes = 7 + totalMsgLength;
    
    // Buffer to hold datawe will send to the IC across 2 packets
    uint8_t bodyData[14] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    // Start of string
    bodyData[0] = 0x10;
    // fAdd in all the body text
    for (int i = 0; i < msg.length(); i++) {
        bodyData[i+1] = msg[i];
        asciiTotal += msg[i];
    }
    // Add null termination to the bodydata
    bodyData[msg.length()+1] = 0x00;
    // Finally, add the validation byte to the bodyData
    bodyData[msg.length() + 2] = calculateBodyCheckSum(msg);

    // Pre-text packet for body text
    curr_frame.data[0] = 0x10;
    curr_frame.data[1] = numberOfBytes;
    curr_frame.data[2] = 0x03;
    curr_frame.data[3] = 0x26;
    curr_frame.data[4] = 0x01;
    curr_frame.data[5] = 0x00;
    curr_frame.data[6] = 0x01;
    curr_frame.data[7] = totalMsgLength;
    c->sendFrame(CAN_BUS_B, &curr_frame);
    delay(7); // Wait 5 MS for IC to process request (+2ms for time taken for the IC to receive packet)

    curr_frame.data[0] = 0x21;
    for (int i = 0; i < 7; i++) {
        curr_frame.data[i+1] = bodyData[i];
    }
    c->sendFrame(CAN_BUS_B, &curr_frame);
    delay(2);
    curr_frame.data[0] = 0x22;
    for (int i = 7; i < 14; i++) {
        curr_frame.data[i-6] = bodyData[i];
    }
    c->sendFrame(CAN_BUS_B, &curr_frame);
    delay(2);
}

/**
 * calculates the header text validation byte
 */
uint8_t IC_DISPLAY::calculateHeaderCheckSum(const char *text) {
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        sum += int(text[i]);
    }
    return 407 - (sum);
}

/**
 * Calculates the body text validation byte
 */
uint8_t IC_DISPLAY::calculateBodyCheckSum(String text){
    uint8_t charCount = text.length();
    // Lookup table valid checksum + ASCII Total values
    uint16_t NINE_CHAR_TOTAL_LOOKUP[] =  {1073, 817, 561, 561};
    uint16_t EIGHT_CHAR_TOTAL_LOOKUP[] = {1090, 834, 578, 322};
    uint16_t SEVEN_CHAR_TOTAL_LOOKUP[] = {1136, 880, 624, 368};
    uint16_t SIX_CHAR_TOTAL_LOOKUP[] =   {1121, 865, 609, 353};
    uint16_t FIVE_CHAR_TOTAL_LOOKUP[] =  {1135, 879, 623, 367};
    uint16_t FOUR_CHAR_TOTAL_LOOKUP[] =  {439, 439, 439, 439};

    int strTotal = 0;
    for(int i = 0; i < charCount; i++) {
        strTotal += text[i];
    }
    if(charCount == 9) {
        for(uint16_t k : NINE_CHAR_TOTAL_LOOKUP) {
            if(k - strTotal <= 256) {
                DPRINTLN("K: "+String(k)+" SUM: "+String(strTotal)+" Num Chars: 9");
                return (k-strTotal);
            }
        }
    }else if(charCount == 8) {
        for(uint16_t k : EIGHT_CHAR_TOTAL_LOOKUP) {
            if(k - strTotal <= 256) {
                DPRINTLN("K: "+String(k)+" SUM: "+String(strTotal)+" Num Chars: 8");
                return (k-strTotal);
            }
        }
    } else if (charCount == 7) {
        for(uint16_t k : SEVEN_CHAR_TOTAL_LOOKUP) {
            if(k - strTotal <= 256) {
                DPRINTLN("K: "+String(k)+" SUM: "+String(strTotal)+" Num Chars: 7");
                return (k-strTotal);
            }
        }
    } else if (charCount == 6) {
        for(uint16_t k : SIX_CHAR_TOTAL_LOOKUP) {
            if(k - strTotal <= 256) {
                DPRINTLN("K: "+String(k)+" SUM: "+String(strTotal)+" Num Chars: 6");
                return (k-strTotal);
            }
        }
    } else if (charCount == 5) {
        for(uint16_t k : FIVE_CHAR_TOTAL_LOOKUP) {
            if(k - strTotal <= 256) {
                DPRINTLN("K: "+String(k)+" SUM: "+String(strTotal)+" Num Chars: 5");
                return (k-strTotal);
            }
        }
    } else if (charCount == 4) {
        for(uint16_t k : FOUR_CHAR_TOTAL_LOOKUP) {
            if(k - strTotal <= 256) {
                DPRINTLN("K: "+String(k)+" SUM: "+String(strTotal)+" Num Chars: 4");
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
    if (!inDiagMode) {
        if (millis() - lastTime > currentRefreshRate) {
            lastTime = millis();
            if(currentPage == clusterPage::Audio) {
                sendBody(currText);
                if(this->currText.length() > MAX_STR_LENGTH) {
                    this->currText = shiftString();
                }
            } else if (!sendFirst) {
                sendBody(currText);
                this->sendFirst = true;
            }
        }
    } else {
        this->sendBody(diagData);
        delay(5);
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
