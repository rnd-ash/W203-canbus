#include <SPI.h>
#include "mcp2515.h"

struct can_frame canMsg;
MCP2515 mcp2515(9);
MCP2515 mcp2515_b(10);


void setup() {
  Serial.begin(115200);
  SPI.begin();
  
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();
  mcp2515_b.reset();
  mcp2515_b.setBitrate(CAN_83K3BPS);
  mcp2515_b.setNormalMode();
  Serial.println("PID, DATA");
}

unsigned long time = millis();
void loop() {
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
      char buffer[7];
      Serial.print("FRAME:ID=");
      Serial.print(canMsg.can_id);
      Serial.print(":LEN=");
      Serial.print(canMsg.can_dlc);
      char tmp[3];
      for (int i = 0; i<canMsg.can_dlc; i++)  {  // print the data
        char buffer[5];
        Serial.print(":");
        sprintf(buffer,"%02X", canMsg.data[i]);
        Serial.print(buffer);
      }
      Serial.println();
  }
}
