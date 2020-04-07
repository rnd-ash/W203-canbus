#pragma GCC optimize("-O3")
#pragma GCC optimize("-j4")

#include "SoftwareSerial.h"
#include "can.h"
SoftwareSerial r = SoftwareSerial(9, 8);

void setup() {
    Serial.begin(115200); 
    r.begin(57600);
    Serial.println("Ready!");
    Serial.println((unsigned long) sizeof(struct can_frame));
    delay(1);
}

void loop() {
    if (r.available()) {
        Serial.write(r.read());
    }
}