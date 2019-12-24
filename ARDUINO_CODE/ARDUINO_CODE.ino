#include "can_comm.h"
#include "ic_display.h"
#include "Bluetooth.h"
#include "Audio_Display.h"

#define CLOCK_A_PIN 18
#define CLOCK_B_PIN 19
bool useClockALED = false;

BLUETOOTH *bt;
IC_DISPLAY *ic;
CANBUS_COMMUNICATOR *can;
AUDIO_DISPLAY *audio;


void setup() {
    Serial.begin(115200);
    Serial.println("READY");
    
    bt = new BLUETOOTH(6, 7);
    can = new CANBUS_COMMUNICATOR(9, CAN_83K3BPS);
    ic = new IC_DISPLAY(can);
    audio = new AUDIO_DISPLAY(ic);


    pinMode(CLOCK_A_PIN, OUTPUT);
    pinMode(CLOCK_B_PIN, OUTPUT);
}

void HandleBluetoothRequest() {
    bt->read_message();
    char* ptr = bt->buffer;
    uint8_t len = strlen(ptr);
    if (len > 0) {
        if (ptr[0] == 'M') {
            if (ptr[2] == 'X' && len == 3) {
                audio->setPlayState(false);
            } else if (ptr[2] == 'P' && len == 3) {
                audio->setPlayState(true);
            } else if (ptr[1] == '-') {
                audio->setTrackName(ptr+2);
            } else if (ptr[1] == ' ') {
                audio->setDuration((byte) ptr[2] * 256 + (byte) ptr[3]);
            } else if (ptr[1] == '_') {
                audio->setElapsed((byte) ptr[2] * 256 + (byte) ptr[3]);
            }
        }
    }
}



void loop() {
    uint8_t led = useClockALED ? CLOCK_A_PIN : CLOCK_B_PIN;
    useClockALED = !useClockALED;
    digitalWrite(led, HIGH);
    HandleBluetoothRequest();
    audio->update();
    digitalWrite(led, LOW);
}