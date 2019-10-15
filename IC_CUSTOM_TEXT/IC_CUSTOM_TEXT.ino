#include "ic.h"
#include "icPacketBatch.h"
#include "mcp2515.h"
#include <stdbool.h>

MCP2515 mcp2515(10);
IC_DISPLAY* d;
icPacketBatch* x;
struct can_frame readFrame;

void setup() {
  x = new icPacketBatch();
  d = new IC_DISPLAY();
  Serial.begin(115200);
  SPI.begin();
  mcp2515.reset();
  mcp2515.setBitrate(CAN_83K3BPS);
  mcp2515.setNormalMode();
  Serial.println("Setup!");
}

void sendFrame(can_frame *frame) {
  int attempts = 0;
  while(true) {
    if (mcp2515.sendMessage(frame) == MCP2515::ERROR_OK) {
      break;
    } else {
      attempts++;
    }
    if (attempts == 100) {
      Serial.println("ERROR SENDING FRAME!");
      break;
    }
  }
}

bool getICResponse() {
  int attempts = 0;
  while(mcp2515.readMessage(&readFrame) == MCP2515::ERROR_OK && attempts <= 100) {
    if(readFrame.can_id == 0x01D0)  {
      Serial.print("Get response after ");
      Serial.print(attempts);
      Serial.print(" !\n");
      return true;
    }
  }
  return false;
}

void sendBody(int chars, const char *str) {
  Serial.println("BODY!");
  Serial.print("Body text: ");
  Serial.print(&str[0]);
  Serial.println();
  d->createBodyPackets(chars, str, x);
  bool gotresponse = false;
  while(!gotresponse) {
    for (int i = 0; i < x->numberOfFrames; i++) {
      char buffer[7];
      sprintf(buffer,"0x%04X", x->frames[i].can_id);
      Serial.print(buffer);
      Serial.print(", ");
      char tmp[2];
      for (int k = 0; k < x->frames[i].can_dlc; k++)  {  // print the data
        char buffer[5];
        sprintf(buffer,"%02X", x->frames[i].data[k]);
        Serial.print(buffer);
        Serial.print(" ");
      }
      Serial.println();
      sendFrame(&x->frames[i]);
    }
    gotresponse = getICResponse();
  }
}

void sendHeading(const char *str) {
  Serial.println("HEADING!");
  d->createHeaderPackets(str, x);
  for (int i = 0; i < x->numberOfFrames; i++) {
    char buffer[7];
    sprintf(buffer,"0x%04X", x->frames[i].can_id);
    Serial.print(buffer);
    Serial.print(", ");
    char tmp[2];
    for (int k = 0; k < x->frames[i].can_dlc; k++)  {  // print the data
      char buffer[5];
      sprintf(buffer,"%02X", x->frames[i].data[k]);
      Serial.print(buffer);
      Serial.print(" ");
    }
    Serial.println();
    sendFrame(&x->frames[i]);
  }
}


void loop() {
  sendBody(9, "Testing! ");
  delay(100L);
  sendBody(9, "esting! T");
  delay(100L);
  sendBody(9, "sting! Te");
  delay(100L);
  sendBody(9, "ting! Tes");
  delay(100L);
  sendBody(9, "ing! Test");
  delay(100L);
  sendBody(9, "ng! Testi");
  delay(100L);
  sendBody(9, "g! Testin");
  delay(100L);
  sendBody(9, "! Testing");
  delay(100L);
}
