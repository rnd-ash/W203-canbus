#include <SPI.h>
#include <mcp2515.h>

struct can_frame canMsg;
MCP2515 mcp2515(10);


void setup() {
  Serial.begin(115200);
  SPI.begin();
  
  mcp2515.reset();
  mcp2515.setBitrate(CAN_83K3BPS);
  mcp2515.setNormalMode();
  Serial.println("PID, DATA");
}

void loop() {
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    char buffer[7];
    // REMOVE THIS IF to see everything
    if(canMsg.can_id == 0x01A4 || canMsg.can_id == 0x01D0){
      sprintf(buffer,"0x%04X", canMsg.can_id);
      Serial.print(buffer);
      Serial.print(", ");
      char tmp[2];
      for (int i = 0; i<canMsg.can_dlc; i++)  {  // print the data
        char buffer[5];
        sprintf(buffer,"%02X", canMsg.data[i]);
        Serial.print(buffer);
        Serial.print(" ");
      }
      Serial.println();
    }
  }
}
