#pragma GCC optimize("-O3")
#pragma GCC optimize("-j4")

#include "can_comm.h"
#include "defines.h"
#include "ic_display.h"
#include "Bluetooth.h"
#include "Audio_Display.h"
#include "Lights.h"
#include "wheel_controls.h"
#include "Telephone_Display.h"
#include "Engine.h"
#include "Music.h"
#include "defines.h"
#include "DiagMode.h"

BLUETOOTH *bt;
IC_DISPLAY *ic;
CANBUS_COMMUNICATOR *canB;
CANBUS_COMMUNICATOR *canC;
AUDIO_DISPLAY *audio;
//TELEPHONE_DISPLAY *tel;
LIGHT_CONTROLS *lights;
WHEEL_CONTROLS *wheel_controls;
ENGINE_DATA * eng;
DIAG_MODE* diag;
MUSIC* musicdata;

const char NEXT_TRACK_CMD[1] = {0x00};
const char PREV_TRACK_CMD[1] = {0x01};


// --  DEBUG DATA FOR FREE MEMEORY -- //
#ifdef DEBUG
int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

unsigned long lastMemTime = millis();

const char * const PROGMEM MEMORY_STR_1 = "FREE SRAM: ";
const char * const PROGMEM MEMORY_STR_2 = " Bytes";
#endif
// --  DEBUG DATA FOR FREE MEMEORY -- //

bool showDiagMode = false;


void doLightShow() {
    for (int i = 0; i < 5; i++) {
        lights->flash_indicator(LIGHT_CONTROLS::LEFT, 250);
        delay(250);
        lights->flash_indicator(LIGHT_CONTROLS::RIGHT, 250);
        delay(250);
    }
    for (int i = 0; i < 4; i++) {
        lights->flash_hazard(500);
        delay(500);
        lights->flash_lights(true, false, 500);
        delay(500);
        lights->flash_lights(false, true, 500);
        delay(500);
    }
    lights->flash_lights(true, true, 2000);
    lights->flash_hazard(2000);
}

void setup() {
    pinMode(4, OUTPUT);
    pinMode(5, OUTPUT);
    Serial.begin(115200);
    SPI.begin();
    canC = new CANBUS_COMMUNICATOR(4, CAN_500KBPS, CAN_C_DEF);
    delay(100);
    canB = new CANBUS_COMMUNICATOR(5, CAN_83K3BPS, CAN_B_DEF);
    #ifdef ARDUINO_MEGA
    bt = new BLUETOOTH();
    #else
    bt = new BLUETOOTH(6, 7);
    #endif
    ic = new IC_DISPLAY(canB);
    audio = new AUDIO_DISPLAY(ic);
    musicdata = new MUSIC(audio);
    //tel = new TELEPHONE_DISPLAY(ic, bt);
    wheel_controls = new WHEEL_CONTROLS();

    #ifdef ARDUINO_MEGA
    eng = new ENGINE_DATA();
    diag = new DIAG_MODE(audio, eng);
    #endif

    Serial.println("Ready!");
    delay(900);
}

void HandleBluetoothRequest() {
    char* ptr = bt->read_message();
    uint8_t len = strlen(ptr);
    if (len > 0) {
        if (ptr[0] == 'M') { // Music data message
            if (ptr[2] == 'X' && len == 3) {
                musicdata->pause();
            } else if (ptr[2] == 'P' && len == 3) {
               musicdata->play();
            } else if (ptr[1] == '-') {
                musicdata->setTrackName(ptr+2);
            } else if (ptr[1] == ' ') {
                musicdata->setDurationSec((byte) ptr[2] * 256 + (byte) ptr[3]);
            } else if (ptr[1] == '_') {
                musicdata->setElapsedSec((byte) ptr[2] * 256 + (byte) ptr[3]);
            }
        } else if (ptr[0] == '!') { // Disconnect message
            musicdata->pause();
            musicdata->setTrackName(NULL);
        } else if (ptr[0] == 'F') {
            lights = new LIGHT_CONTROLS(canB);
            doLightShow();
            free(lights);
        } else if (ptr[0] = 'C') {
            //tel->setCarrier(ptr+1);
        }
    }
}

void handleFrameRead() {
    can_frame *readB = canB->read_frame();
    if (readB->can_dlc != 0) {
        ic->processIcResponse(readB);
        handleKeyInputs(readB);
        if (readB->can_id == 0x000C) {
            eng->readFrame(readB);
        }
    }
    can_frame *read = canC->read_frame();
    if (read->can_dlc != 0) {
        if (eng != NULL) {
            eng->readFrame(read);
        } 
    }
}

void handleKeyInputs(can_frame *f) {
    // User is in audio page
    if (ic->current_page == 0x03) {
        // Diag mode is currently being shown
        if (showDiagMode) {
            switch(wheel_controls->getPressed(f)) {
                case BUTTON_ARROW_UP:
                    diag->nextDiagPage();
                    break;
                case BUTTON_ARROW_DOWN:
                    diag->prevDiagPage();
                    break;
                case BUTTON_TEL_DEC:
                    showDiagMode = false;
                    // Mega has enough RAM to keep these in memory, uno doesn't
                    #ifndef ARDUINO_MEGA
                    free(eng);
                    free(diag);
                    #endif
                    break;
                default:
                    break;
            }
        } 
        // Normal music screen is being shown
        else {
            switch(wheel_controls->getPressed(f)) {
                case BUTTON_ARROW_UP:
                    bt->write_message(NEXT_TRACK_CMD, 1);
                    break;
                case BUTTON_ARROW_DOWN:
                    bt->write_message(PREV_TRACK_CMD, 1);
                    break;
                case BUTTON_TEL_ANS:
                    #ifndef ARDUINO_MEGA
                    eng = new ENGINE_DATA();
                    diag = new DIAG_MODE(audio, eng);
                    #endif
                    showDiagMode = true; 
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
                bt->write_message(NEXT_TRACK_CMD, 1); // Use telephone Answer button to seek track
                break;
            case BUTTON_TEL_DEC:
                bt->write_message(PREV_TRACK_CMD, 1); // Use telephone decline button to repeat track
                break;
            default:
                break;
        }
    }
}

void loop() {
    HandleBluetoothRequest();
    audio->update();
    musicdata->update();
    if (showDiagMode) {
        diag->updateUI();
    } else {
        musicdata->updateUI();
    }

    handleFrameRead();
    #ifdef DEBUG
    if (millis() - lastMemTime > 2000) {
        lastMemTime = millis();
        DPRINTLN(MEMORY_STR_1+String(freeRam())+MEMORY_STR_2);
    }
    #endif
}