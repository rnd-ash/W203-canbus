//
// Created by ashcon on 10/5/19.
//

#include "ic.h"
/*
 * Sending header text
 * Send packet 1
 * Receive response
 * Send packet 2
 * Receive response
 */

void IC_DISPLAY::createHeaderPackets(const char text[3], icPacketBatch *b) {
    int checkSumTotal = 0;
    for (int i = 0; i < 3; i++) {
        checkSumTotal += text[i];
    }
    int checkSumBit = 407 - checkSumTotal;
    b->frames[0].can_id = IC_SEND_PID;
    b->frames[0].can_dlc = 0x08;
    b->frames[0].data[0] = 0x10;
    b->frames[0].data[1] = 0x09;
    b->frames[0].data[2] = 0x03;
    b->frames[0].data[3] = 0x29;
    b->frames[0].data[4] = 0x00;
    b->frames[0].data[5] = text[0];
    b->frames[0].data[6] = text[1];
    b->frames[0].data[7] = text[2];

    b->frames[1].can_id = IC_SEND_PID;
    b->frames[1].can_dlc = 0x08;
    b->frames[1].data[0] = 0x21;
    b->frames[1].data[1] = 0x20;
    b->frames[1].data[2] = 0x00;
    b->frames[1].data[3] = checkSumBit;
    b->frames[1].data[4] = 0x05;
    b->frames[1].data[5] = 0x26;
    b->frames[1].data[6] = 0x01;
    b->frames[1].data[7] = 0x00;

    b->numberOfFrames = 2;
}

/*
 * Sending body text packets
 * Send PAC 1
 * Receive Response
 * Send Pac 2
 * Send Pac 3
 * Receive response
 */
void IC_DISPLAY::createBodyPackets(int charCount, const char *text, icPacketBatch *b) {
    char temp[9];
    if (charCount > 9) {
        charCount = 9;
    }
    for (int i = 0; i < charCount; i++) {
        temp[i] = text[i];
    }
    if (charCount < 5) {
        for (int k = charCount; k <= 5; k++) {
            temp[k] = 0x20;
        }
        charCount = 5;
    }
    int asciiTotal = 0;
    int checkSumB = charCount + 2;
    int checkSumA = 7 + checkSumB;
    // Placeholder - Need to work out how to calculate this
    
    uint8_t bodyData[11] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    for (int i = 0; i < charCount; i++) {
        bodyData[i] = temp[i];
        asciiTotal += temp[i];
    }
    uint8_t x = calculateBodyCheckSum(charCount, temp);
    bodyData[charCount] = 0x00;
    bodyData[charCount+1] = int(x);

    b->frames[0].can_id = IC_SEND_PID;
    b->frames[0].can_dlc = 0x08;
    b->frames[0].data[0] = 0x10;
    b->frames[0].data[1] = checkSumA;
    b->frames[0].data[2] = 0x03;
    b->frames[0].data[3] = 0x26;
    b->frames[0].data[4] = 0x01;
    b->frames[0].data[5] = 0x00;
    b->frames[0].data[6] = 0x01;
    b->frames[0].data[7] = checkSumB;
    b->numberOfFrames = 1;

    b->frames[1].can_id = IC_SEND_PID;
    b->frames[1].can_dlc = 0x08;
    b->frames[1].data[0] = 0x21;
    b->frames[1].data[1] = 0x10;
    for (int i = 2; i <=7; i++) {
        b->frames[1].data[i] = bodyData[i-2];
    }

    b->frames[2].can_id = IC_SEND_PID;
    b->frames[2].can_dlc = 0x08;
    b->frames[2].data[0] = 0x22;
    for (int i = 1; i <= 5; i++) {
        b->frames[2].data[i] = bodyData[i+5];
    }

    b->frames[2].data[6] = 0x20;
    b->frames[2].data[7] = 0x00;

    b->numberOfFrames = 3;
}

uint8_t IC_DISPLAY::calculateHeaderCheckSum(const char *text) {
    return 0;
}

uint8_t IC_DISPLAY::calculateBodyCheckSum(int charCount, const char *text){
    // Lookup table valid checksum + ASCII Total values
    int NINE_CHAR_TOTAL_LOOKUP[] =  {1073, 817, 561, 305};
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
            if(k - strTotal >= 256) {
                return (k-strTotal);
            }
        }
    } else if (charCount == 7) {
        for(int k : SEVEN_CHAR_TOTAL_LOOKUP) {
            if(k - strTotal >= 256) {
                return (k-strTotal);
            }
        }
    } else if (charCount == 6) {
        for(int k : SIX_CHAR_TOTAL_LOOKUP) {
            if(k - strTotal >= 256) {
                return (k-strTotal);
            }
        }
    } else if (charCount == 5) {
        for(int k : FIVE_CHAR_TOTAL_LOOKUP) {
            if(k - strTotal >= 256) {
                return (k-strTotal);
            }
        }
    }
    return 0;
}




