#include "ICDisplay.h"
#include "helper.h"

AudioDisplay* audioDisplay = nullptr;
TelDisplay* telDisplay = nullptr;

// -- Base IC stuff --

ICDisplay::ICDisplay(){
    this->sendComplete = true;
    this->isSending = false;
};

void ICDisplay::update() {
}


void ICDisplay::sendToKombi() {
    // Time to send next frame
    if (millis() >= nextUpdateTime) {
        canB->sendMessage(&frameBuffer[currFrameBufferIndex]);
        nextUpdateTime += SEPERATION_TIME_MS;
        LOG_DEBG(F_TC("Sending frame %d to Kombi\n"), currFrameBufferIndex);
        if (currFrameBufferIndex >= frameBufferSize-1) {
            LOG_DEBG(F_TC("Full payload sent to Kobi!\n"));
            // Finished sending
            isSending = false;
            sendComplete = true;
            delete[] frameBuffer;
            onSendComplete(); // Finished!
        } else {
            currFrameBufferIndex++;
        }
    }
}

void ICDisplay::putChecksum(uint8_t*buffer, uint8_t bufferSize) {
    uint8_t cs = 0xFF;
    for (uint8_t i = 0; i < bufferSize-1; i++) cs -= i + buffer[i];
    buffer[bufferSize-1] = cs; // Place checksum at end of payload
}

void ICDisplay::processIncommingFrame(can_frame *f) {
    if (f->can_id == RESP_CID) {
        if (f->data[0] != 0x30) {
            LOG_WARN(F_TC("Unknown frame from Kombi: %s\n"), Canbus::frameToString(f));
        }
    } else if (f->can_id == SEND_CID) {
        LOG_WARN(F_TC("AGW DATA: %s\n"), Canbus::frameToString(f));
    }
}

void ICDisplay::buildFrameBuffer(uint8_t* buffer, uint8_t bufferSize) {
    if (bufferSize > 55) {
        LOG_ERR(F_TC("Payload to big at %d bytes. Max is 55\n"), bufferSize);
        return;
    }
    // Single frame, just send it now and dont worry about it
    if (bufferSize <= 7) {
        can_frame temp {
            SEND_CID,
            0x08,
            buffer[0]
        };
        memset(&temp.data[1], *buffer, bufferSize);
        canB->sendMessage(&temp);
        sendComplete = true;
        isSending = false;
        onSendComplete();
    } 
    // Multi frame payload, build payload!
    else {
        int posInBuffer = 0;
        uint8_t startISO = 0x21; // Byte for ISO Sequencing, start at 0x21 and incriment by 1 at each frame
        frameBufferSize = ((bufferSize-7)/7)+2; // How many frames do we need for the payload?
        frameBuffer = new can_frame[frameBufferSize]; // Allocate buffer in memory
        frameBuffer[0] = can_frame{SEND_CID, 0x08, 0x10, bufferSize}; // Set first bytes in first frame
        memcpy(&frameBuffer[0].data[2], buffer, 6); // Copy first 6 bytes
        posInBuffer = 6;
        // Start as 1 as we already copied the first frame!
        for (int i = 1; i < frameBufferSize; i++) {
            int bytesToCopy = min(7, bufferSize-6); // Ensure not an overrun
            frameBuffer[i] = can_frame{SEND_CID, 0x08, startISO}; // new frame with sequence ID
            memcpy(&frameBuffer[i].data[1], &buffer[posInBuffer], bytesToCopy); // Copy over buffer
            startISO += 1; // Incriment sequence for next frame
            posInBuffer+=bytesToCopy;
        }

        // Send first frame to Kombi before we exit function so
        // Kombi can send us back response
        canB->sendMessage(&frameBuffer[0]);
        nextUpdateTime = millis()+10;
        currFrameBufferIndex = 1; // Next frame on CTS
        sendComplete = false;
        isSending = true;
        // Print frames - debugging
        for (int i = 0; i < frameBufferSize; i++) {
            LOG_DEBG(F_TC("BUFFER %d: %s\n"), i, Canbus::frameToString(&frameBuffer[i]));
        }
    }
}

void ICDisplay::updateHeader(uint8_t page, lineData* line) {
    uint8_t bufSize = 5+line->textLen;
    uint8_t *buf = new uint8_t[bufSize]{page, 0x29, line->fmt_args};
    memcpy(&buf[3], line->text, line->textLen);
    buf[bufSize-2] = 0x00;
    this->putChecksum(buf, bufSize);
    buildFrameBuffer(buf, bufSize);
    delete[] buf;
}

void ICDisplay::setBody(uint8_t page, lineData* line) {
    uint8_t bufSize = 9+line->textLen;
    uint8_t *buf = new uint8_t[bufSize]{page, 0x26, 0x01, 0x00, 0x01, line->textLen+2, line->fmt_args };
    memcpy(&buf[7], line->text, line->textLen);
    buf[bufSize-2] = 0x00;
    this->putChecksum(buf, bufSize);
    buildFrameBuffer(buf, bufSize);
    delete[] buf;
}

#define MAX_LINES 4
void ICDisplay::updateBodyMultipleLines(uint8_t page, lineData* lines, uint8_t numLines) {
    if (numLines > MAX_LINES) { 
        LOG_ERR(F_TC("Too many lines [%d]. Max is %d\n"), numLines, MAX_LINES);
        return;
    }
    uint8_t bufSize = 7;
    for (int i = 0; i < numLines; i++) { bufSize += lines[i].textLen + 2 ;}
    uint8_t *buf = new uint8_t[bufSize]{page, 0x26, 0x01, 0x00, 0xFF};
    uint8_t counter = 5;
    for (int i = 0; i < numLines; i++) {
        buf[counter] = lines[i].textLen+2;
        buf[counter+1] = lines[i].fmt_args;
        memcpy(&buf[counter+2], lines[i].text, lines[i].textLen);
        counter += lines[i].textLen+2;
    }
    buf[bufSize-2] = 0x00;
    this->putChecksum(buf, bufSize);
    buildFrameBuffer(buf, bufSize);
    delete[] buf;
}


// -- Audio page stuff --
void ICDisplay::test() {
    if (sendComplete) {
        //initPage();
        /*
        char* l1 = "THIS IS A LONG LINE";
        char* l2 = "HIGHLIGHTED";
        lineData* data = new lineData[2];
        data[0] = lineData { 0x10, 19, l1};
        data[1] = lineData { IC_FMT_HIGHLIGHTED | IC_FMT_CENTER, 11, l2};
        updateBodyMultipleLines(0x05, data, 2);
        */
    } else {
        LOG_WARN(F_TC("Cannot build buffer - sending hasn't finished!\n"));
    }
}

void ICDisplay::initPage() {
    uint8_t bufSize = 33;
    uint8_t *buf = new uint8_t[bufSize]{
              0x05, 0x24, 0x02, 0x60, 0x01, 0x02,
        0x00, 0x00, 0x00, 0x15, 0x00, 0x01, 0x00,
        0x02, 0x00, 0x03, 0x00, 0x04, 0x00, 0x05,
        0x10, 'T', 'E', 'S', 'T', ' ', 'H', 'E', 'A', 'D', 0x00 
    };
    buf[bufSize-2] = 0x00;
    this->putChecksum(buf, bufSize);
    buildFrameBuffer(buf, bufSize);
    delete[] buf;
}

// -- Audio page stuff --

// -- Telephone page stuff --