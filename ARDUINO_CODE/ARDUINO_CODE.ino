#include "can_comm.h"
#include "defines.h"
#include "ic_display.h"
#include "Bluetooth.h"
#include "Audio_Display.h"
#include "Lights.h"
#include "wheel_controls.h"
#include "Telephone_Display.h"
#include "Engine.h"
#include "defines.h"

BLUETOOTH *bt;
IC_DISPLAY *ic;
CANBUS_COMMUNICATOR *canB;
CANBUS_COMMUNICATOR *canC;
AUDIO_DISPLAY *audio;
TELEPHONE_DISPLAY *tel;
LIGHT_CONTROLS *lights;
WHEEL_CONTROLS *wheel_controls;
ENGINE_DATA * eng;

const char * const PROGMEM NEXT_TRACK_CMD = "N";
const char * const PROGMEM PREV_TRACK_CMD = "P";
const char * const PROGMEM SKIDDING_ACT = "S";
const char * const PROGMEM SKIDDING_DIS = "D";


void setup() {
    Serial.begin(115200);
    SPI.begin();
    bt = new BLUETOOTH(6, 7);
    canC = new CANBUS_COMMUNICATOR(9, CAN_500KBPS, MCP_8MHZ ,CAN_C_DEF);
    canB = new CANBUS_COMMUNICATOR(10, CAN_83K3BPS, CAN_B_DEF);
    ic = new IC_DISPLAY(canB);
    audio = new AUDIO_DISPLAY(ic);
    tel = new TELEPHONE_DISPLAY(ic);
    wheel_controls = new WHEEL_CONTROLS();
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
            lights = new LIGHT_CONTROLS(canB);
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
    can_frame *readB = canB->read_frame();
    if (readB->can_dlc != 0) {
        //canB->printFrame(readB);
        ic->processIcResponse(readB);
        handleKeyInputs(readB);

    }
    can_frame *read = canC->read_frame();
    if (read->can_dlc != 0) {
        if (eng != NULL) {
            eng->readFrame(read);
        } 
        #ifndef DEBUG
        canC->printFrame(read);
        #endif
    }
}

void handleKeyInputs(can_frame *f) {
    // User is in audio page
    if (ic->current_page == 0x03) {
        // Diag mode is currently being shown
        if (audio->getDiagModeEnabled()) {
            switch(wheel_controls->getPressed(f)) {
                case BUTTON_ARROW_UP:
                    audio->diagNextPage();
                    break;
                case BUTTON_ARROW_DOWN:
                    audio->diagPrevPage();
                    break;
                case BUTTON_TEL_DEC:
                    audio->disableDiagMode(); // Disable diag mode
                    free(eng);
                    break;
                default:
                    break;
            }
        } 
        // Normal music screen is being shown
        else {
            switch(wheel_controls->getPressed(f)) {
                case BUTTON_ARROW_UP:
                    bt->write_message(NEXT_TRACK_CMD);
                    break;
                case BUTTON_ARROW_DOWN:
                    bt->write_message(PREV_TRACK_CMD);
                    break;
                case BUTTON_TEL_ANS:
                    eng = new ENGINE_DATA();
                    audio->enableDiagMode(eng); // Enable diag mode 
                    break;
                default:
                    break;
            }
        }
    } 
    // Telephone screen
    else if (ic->current_page == 0x05) {

    }
    // Other screen - Here the Arrows / Page buttons are used so we can only use the telephone buttons 
    else {
        switch(wheel_controls->getPressed(f)) {
            case BUTTON_TEL_ANS:
                bt->write_message(NEXT_TRACK_CMD); // Use telephone Answer button to seek track
                break;
            case BUTTON_TEL_DEC:
                bt->write_message(PREV_TRACK_CMD); // Use telephone decline button to repeat track
                break;
            default:
                break;
        }
    }
}


void loop() {
    HandleBluetoothRequest();
    audio->update();
    tel->update();
    handleFrameRead();
}