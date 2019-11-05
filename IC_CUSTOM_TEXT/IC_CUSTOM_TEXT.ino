#include <SoftwareSerial.h>
#include "mcp2515.h"
#include "ic.h"
#include "phoneBluetooth.h"
#include "console_control.h"
#include "wheel_controls.h"
#include "canbuscomm.h"
#include "signals.h"

#define DEBUG
#ifdef DEBUG
  #define DPRINT(...) Serial.print(__VA_ARGS__)
  #define DPRINTLN(...) Serial.println(__VA_ARGS__)
#else
  #define DPRINT(...)
  #define DPRINTLN(...)
#endif


MCP2515 mcp2515(10);

CanbusComm *cancomm;
SignalControls *signals; 
IC_DISPLAY* d;
wheelControls* wc;
phoneBluetooth* bt;
centerConsole* cc;
struct can_frame readFrame;


String message;

void setup() {
  pinMode(4,  OUTPUT); // Test Button
  pinMode(14, OUTPUT); // Blue LED
  pinMode(15, OUTPUT); // Green LED
  pinMode(16, OUTPUT); // Yellow LED
  pinMode(17, OUTPUT); // Red LED
  pinMode(18, OUTPUT); // White (Clock) LED
  pinMode(19, OUTPUT); // White (Clock) LED
  cancomm = new CanbusComm(&mcp2515);
  wc = new wheelControls(cancomm);
  d = new IC_DISPLAY(cancomm);
  bt = new phoneBluetooth(5, 6);
  cc = new centerConsole(cancomm);
  signals = new SignalControls(cancomm);
  Serial.begin(115200);
  SPI.begin();
  mcp2515.reset();
  mcp2515.setBitrate(CAN_83K3BPS);
  mcp2515.setNormalMode();
  for (int i = 14; i <= 19; i++) {
    delay(10);
    digitalWrite(i, HIGH);
  }
  delay(500);
  for (int i = 14; i <= 19; i++) {
    digitalWrite(i, LOW);
  }
  Serial.println("Ready!");
}

void processButtonRequest(char x) {
  switch (x)
  {
  case '1':
    cc->toggleESP();
    break;
  case '2':
    cc->lockDoors();
    break;
  case '3':
    cc->unlockDoors();
    break;
  case '4':
    cc->retractHeadRest();
    break;
  case '5':
    signals->enableRightIndicator();
    break;
  case '6':
    signals->enableLeftIndicator();
    break;
  case '7':
    signals->enableHazards();
    break;
  case '8':
    signals->disableAll();
    break;
  default:
    Serial.println("cannot process center console request "+x);
    break;
  }
}

static long intervalDisplayBody = 140;
void bluetoothListenThread() {
  String tmpMsg = bt->readMessage();
  if (tmpMsg != "") {
    if (tmpMsg[0] == 'B') {
      tmpMsg.remove(0, 2);
      d->setBodyText(tmpMsg);
    } else if (tmpMsg[0] == 'S') {
      tmpMsg.remove(0, 2);
      int tmp = atoi(tmpMsg.c_str());
      d->refreshIntervalMS = tmp;
    } else if (tmpMsg[0] == 'C') {
      processButtonRequest(tmpMsg[2]);
    } else if (tmpMsg[0] == 'A') {
      tmpMsg.remove(0, 2);
      int tmp = atoi(tmpMsg.c_str());
      signals->intervalMS = tmp;
    } else {
      //TODO other messages
    }
  }
}

void keyPressThread() {
  wheelControls::key key = wc->getPressed();
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

bool clock = false;
void loop() {
  int clockPin = clock ? 18 : 19;
  digitalWrite(clockPin, HIGH);
  bluetoothListenThread();
  keyPressThread();
  d->update();
  signals->update();
  digitalWrite(clockPin, LOW);
  clock = !clock;
  
}
