#include <SoftwareSerial.h>
#include "mcp2515.h"
#include "ic.h"
#include "phoneBluetooth.h"
#include "console_control.h"
#include "wheel_controls.h"
#include "canbuscomm.h"
#include "signals.h"
#include "debug.h"

CanbusComm *cancomm;
SignalControls *signals; 
IC_DISPLAY* d;
wheelControls* wc;
phoneBluetooth* bt;
centerConsole* cc;
struct can_frame readFrame;


String message;

void setup() {
  // LED Pins (order)
  // 1. Bluetooth TX (14)
  // 2. Bluetooth RX (15)
  // 3. CanBus  B TX (16)
  // 4. CanBus  B RX (17)
  // 5. CanBus  C TX (4)
  // 6. CanBus  C RX (5)
  // 7. Clock LED 1  (18)
  // 8. Clock LED 2  (19)
  int ledPins[8] =  { 14, 15, 16, 17, 4, 5, 18, 19 };
  cancomm = new CanbusComm(10, 8);
  wc = new wheelControls(cancomm);
  d = new IC_DISPLAY(cancomm);
  bt = new phoneBluetooth(6, 7);
  cc = new centerConsole(cancomm);
  signals = new SignalControls(cancomm);
  Serial.begin(115200);
  for (int led: ledPins) {
    pinMode(led, OUTPUT);
    delay(25);
    digitalWrite(led, HIGH);
  }
  delay(25);
  for (int led: ledPins) {
    delay(25);
    digitalWrite(led, LOW);
  }
  delay(25);
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
    DPRINTLN("Incomming BT message: '"+tmpMsg+"'");
    if (tmpMsg[0] == 'B') {
      tmpMsg.remove(0, 2);
      d->setBodyText(tmpMsg);
    } else if (tmpMsg[0] == 'S') {
      tmpMsg.remove(0, 2);
      int tmp = atoi(tmpMsg.c_str());
      if(tmp != 0){
        d->setRefreshRate(tmp);
      }
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
