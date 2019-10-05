#include "ic.h"

IC_DISPLAY* dptr;

MCP2515 mcp2515(10);
void setup() {
  dptr = new IC_DISPLAY(80, CAN_83K3BPS);
  Serial.begin(115200);
  SPI.begin();
  mcp2515.reset();
  mcp2515.setBitrate(CAN_83K3BPS);
  mcp2515.setNormalMode();
  Serial.println("Setup!");
}
void gen_random(char *s, const int len) {
  static const char alphanum[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

  for (int i = 0; i < len; ++i) {
      s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
  }

  s[len] = 0;
}

void loop() {
  // put your main code here, to run repeatedly:
  char c[3];
  gen_random(c,3);
  can_frame x = dptr->sendHeaderText(c, mcp2515);
  for (int i = 0; i < 8;i++){
    Serial.print(" ");
    Serial.print(x.data[i], HEX);
  }
  Serial.println();
  delay(100);
}
