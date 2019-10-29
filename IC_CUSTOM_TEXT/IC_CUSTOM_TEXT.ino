#include <SoftwareSerial.h>
#include "mcp2515.h"
#include "ic.h"
#include "icPacketBatch.h"
#include "phoneBluetooth.h"
#include "console_control.h"
#include "wheel_controls.h"


MCP2515 mcp2515(10);

IC_DISPLAY* d;
wheelControls* wc;
icPacketBatch* pb;
phoneBluetooth* bt;
centerConsole* cc;
struct can_frame readFrame;


String message;

void setup() {
  wc = new wheelControls();
  pb = new icPacketBatch();
  d = new IC_DISPLAY();
  bt = new phoneBluetooth(5, 6);
  cc = new centerConsole();
  Serial.begin(115200);
  SPI.begin();
  mcp2515.reset();
  mcp2515.setBitrate(CAN_83K3BPS);
  mcp2515.setNormalMode();
  Serial.println("SETUP! COMPLETED");
}

void sendFrame(can_frame *frame) {
  int attempts = 0;
  while(mcp2515.sendMessage(frame) != MCP2515::ERROR_OK) {
    if (attempts == 100) {
      break;
    }
    attempts++;
  }
}

void sendHeader(boolean display) {
  sendFrame(&pb->frames[0]);
  delay(7);
  sendFrame(&pb->frames[1]);
  delay(3);
}  

void sendBody(boolean display) {
  sendFrame(&pb->frames[0]);
  delay(7);
  sendFrame(&pb->frames[1]);
  delay(2);
  sendFrame(&pb->frames[2]);
  delay(2);
}  

void processButtonRequest(char x) {
  switch (x)
  {
  case '1':
    cc->toggleESP(&mcp2515);
    break;
  case '2':
    cc->lockDoors(&mcp2515);
    break;
  case '3':
    cc->unlockDoors(&mcp2515);
    break;
  case '4':
    cc->retractHeadRest(&mcp2515);
    break;
  default:
    Serial.println("cannot process center console request "+x);
    break;
  }
}

String shiftStringBy1(String msg) {
  char x = msg[0];
  String tmp;
  for (int i = 1; i <msg.length(); i++) {
    tmp += msg[i];
  }
  tmp += x;
  return tmp;
  
}

String bodyText = "WAITING!";
bool updated = false;

static long intervalDisplayBody = 300;
void bluetoothListenThread() {
  String tmpMsg = bt->readMessage();
  if (tmpMsg != "") {
    if (tmpMsg[0] == 'B') {
      tmpMsg.remove(0, 2);
      bodyText = tmpMsg;
      if (bodyText.length() > 8) {
        bodyText += "    ";
      }
      updated = false;
    } else if (tmpMsg[0] == 'S') {
      tmpMsg.remove(0, 2);
      int tmp = atoi(tmpMsg.c_str());
      if (tmp > 50) {
        intervalDisplayBody = tmp - 10;
      }
    } else if (tmpMsg[0] == 'C') {
      processButtonRequest(tmpMsg[2]);
    } else {
      //TODO other messages
    }
  }
}

unsigned long millisDisplay = millis();
void displayBodyThread() {
  if (millis() - millisDisplay > intervalDisplayBody) {
    millisDisplay = millis();
    if (!updated) {
      d->createBodyPackets(bodyText, pb);
      sendBody(pb);
      if (bodyText.length() > 8) {
        bodyText = shiftStringBy1(bodyText);
      } else {
        updated = true;
      }
    }
  }
}

void keyPressThread() {
  wheelControls::key key = wc->getPressed(&mcp2515);
  switch (key)
  {
  case 0x01:
    bt->writeMessage("N");
    break;
  case 0x02:
    bt->writeMessage("P");
  default:
    break;
  }
}

void loop() {
  bluetoothListenThread();
  displayBodyThread();
  keyPressThread();
}
