#include "can_comm.h"
#include "defines.h"
#include "ic_display.h"
#include "Bluetooth.h"
#include "Audio_Display.h"
#include "Lights.h"
#include "wheel_controls.h"

bool useClockALED = false;

BLUETOOTH *bt;
IC_DISPLAY *ic;
CANBUS_COMMUNICATOR *can;
AUDIO_DISPLAY *audio;
LIGHT_CONTROLS *lights;
WHEEL_CONTROLS *wheel_controls;

const char * const PROGMEM NEXT_TRACK_CMD = "N";
const char * const PROGMEM PREV_TRACK_CMD = "P";

void setup() {
    Serial.begin(115200);
    Serial.println(F("READY"));
    
    bt = new BLUETOOTH(6, 7);
    can = new CANBUS_COMMUNICATOR(10, CAN_83K3BPS);
    ic = new IC_DISPLAY(can);
    audio = new AUDIO_DISPLAY(ic);
    wheel_controls = new WHEEL_CONTROLS();


    pinMode(CLOCK_A_PIN, OUTPUT);
    pinMode(CLOCK_B_PIN, OUTPUT);
}

void HandleBluetoothRequest() {
    char* ptr = bt->read_message();
    uint8_t len = strlen(ptr);
    if (len > 0) {
        if (ptr[0] == 'M') { // Music data message
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
        } else if (ptr[0] == '!') { // Disconnect message
            audio->setPlayState(false);
        } else if (ptr[0] == 'F') {
            lights = new LIGHT_CONTROLS(can);
            for (uint8_t i = 0; i < 10; i++) {
                lights->flash_lights(true, true, 200);
                delay(50);
                lights->flash_hazard(200);
                delay(100);
            }
            free(lights);
        }
    }
}

void handleFrameRead() {
    can_frame *read = can->read_frame();
    if (read->can_dlc != 0) {
        ic->processIcResponse(read);
        handleKeyInputs(read);
    }
}

void handleKeyInputs(can_frame *f) {
    // User is in audio page
    if (ic->current_page == IC_DISPLAY::AUDIO) {
        // Diag mode is currently being shown
        if (audio->getDiagModeEnabled()) {
            switch(wheel_controls->getPressed(f)) {
                case WHEEL_CONTROLS::ARROW_UP:
                    //bt->write_message("N");
                    break;
                case WHEEL_CONTROLS::ARROW_DOWN:
                    //bt->write_message("P");
                    break;
                case WHEEL_CONTROLS::TELEPHONE_ANSWER:
                    //audio->enableDiagMode();
                    break;
                case WHEEL_CONTROLS::TELEPHONE_HANGUP:
                    audio->disableDiagMode(); // Disable diag mode
                    break;
                default:
                    break;
            }
        } 
        // Normal music screen is being shown
        else {
            switch(wheel_controls->getPressed(f)) {
                case WHEEL_CONTROLS::ARROW_UP:
                    bt->write_message(NEXT_TRACK_CMD);
                    break;
                case WHEEL_CONTROLS::ARROW_DOWN:
                    bt->write_message(PREV_TRACK_CMD);
                    break;
                case WHEEL_CONTROLS::TELEPHONE_ANSWER:
                    audio->enableDiagMode(); // Enable diag mode 
                    break;
                case WHEEL_CONTROLS::TELEPHONE_HANGUP:
                    break;
                default:
                    break;
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
    handleFrameRead();
    digitalWrite(led, LOW);
}