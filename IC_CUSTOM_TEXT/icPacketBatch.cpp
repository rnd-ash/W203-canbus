#include "icPacketBatch.h"

void icPacketBatch::printBatch() {
    for (int i = 0; i < this->numberOfFrames; i++) {
        Serial.print("FRAME ID: ");
        char buffer[7];
        sprintf(buffer,"0x%04X", this->frames[i].can_id);
        Serial.print(buffer);
        char tmp[2];
        Serial.print("  BYTES: ");
        for (int k = 0; k < this->frames[i].can_dlc; k++)  {  // print the data
            char buffer[5];
            sprintf(buffer,"%02X", this->frames[i].data[k]);
            Serial.print(buffer);
            Serial.print(" ");
        }
        Serial.print("  CHARS: ");
        for (int k = 0; k < this->frames[i].can_dlc; k++)  {  // print the data
            int data = this->frames[i].data[k];
            if (data >= 32 && data <= 126) {
                Serial.print((char) data);
            } else {
                Serial.print(".");
            }
        }
        Serial.print("\n");
    }
}

String icPacketBatch::frameToString(can_frame frame) {
    String msg = "FRAME ID: ";
    char buffer[7];
    sprintf(buffer,"0x%04X", frame.can_id);
    msg += buffer;
    msg += " BYTES: ";
    char tmp[2];
    for (int k = 0; k < frame.can_dlc; k++)  {  // print the data
        char buffer[5];
        sprintf(buffer,"%02X", frame.data[k]);
        msg += buffer;
        msg += " ";
    }
    msg += " CHARS: ";
    for (int k = 0; k < frame.can_dlc; k++)  {  // print the data
        int data = frame.data[k];
        if (data >= 32 && data <= 126) {
            msg += (char) data;
        } else {
            msg += ".";
        }
    }
    return msg;
}