#include <SoftwareSerial.h>
#include "mcp2515.h"
#include "phoneBluetooth.h"
#include "canbuscomm.h"
#include "debug.h"
#include "Car.h"
#include "Music.h"
#include "EngineData.h"
#include "ic.h"

phoneBluetooth* bt;
Car *car;

void setup() {
  // LED Pins (order)
  // 1. Bluetooth TX (14)
  // 2. Bluetooth RX (15)
  // 3. CanBus  B TX (16)
  // 4. CanBus  B RX (17)
  // 5. CanBus  C TX (4)
  // 6. CanBus  C RX (5)
  // 7. Clock LED 1  (18)
  // 8. Clock LED 2  (3)
  int ledPins[8] =  { 14, 15, 16, 17, 4, 5, 18, 19 };
  car = new Car(new CanbusComm(10, 8));
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
  Serial.println(F("Ready!"));
}

bool clock = false;
void loop() {
  int clockPin = clock ? 18 : 19;
  digitalWrite(clockPin, HIGH);
  car->loop();
  digitalWrite(clockPin, LOW);
  clock = !clock;
}
