#include "Car.h"

const char * const PROGMEM APP_EXIT_TXT = "APP EXIT!";
const char * const PROGMEM UNKNOWN_TRACK = "Track: ??";
const char * const PROGMEM TRACK_PAUSED = "PAUSED";
const char * const PROGMEM TRACK_PAUSED_HEAD = "----";

Car::Car(CanbusComm *c) {
    this->engine = new EngineData();
    this->ic = new IC_DISPLAY(c, engine);
    this->wheel = new wheelControls();
    this->bluetooth = new phoneBluetooth(6, 7);
    this->c = c;
    this->music = new Music();
    this->lastUpdateMillis = millis();
    this->isLocked = false;
    this->lockJobDone = false;
    this->mirrors = new Mirrors(c);
}

uint8_t count = 0;
bool reverseJobDone = false;
void Car::processCanFrame() {
    // Poll for a can frame on bus B (Interior Can)
    can_frame f = c->pollForFrame(CAN_BUS_B);
    // Key press related frame
    if (f.can_id == 0x1CA) {
        processKeyPress(&f);
    } 
    // Frame from EZS_A1
    else if (f.can_id == 0x0000 && f.can_dlc > 0) {
        if (f.data[5] == 0xaa && !engine->isOn) {
            isLocked = true;
            if (!lockJobDone) {
                can_frame f;
                f.can_id = 0x0045;
                f.can_dlc = 4;
                f.data[3] = 0x44;
                f.data[4] = 0x04;
                for (int i = 0; i < 1000; i++) {
                    c->sendFrame(CAN_BUS_B, &f);
                    delay(10);
                }
                lockJobDone = true;
            }
        } else {
            isLocked = false;
            lockJobDone = false;
        }
    }
    // Frame from GW_C_B1 
    else if (f.can_id == 0x0002) {
        engine->rpm = (f.data[2] << 8) | (f.data[3]);
        engine->isOn = f.data[2] != 0xFF;
        engine->coolantTemp = f.data[4] - 40;
    }
    // Frame from GW_C_B2 
    else if (f.can_id == 0x0003) {
        // frames 8th bit indicates if car is in reverse or not
        bool isReverse = f.data[1] >> 7 == 1;
        if (isReverse) {
            count++;
            if (count == 5 && !reverseJobDone) {
                mirrors->lowerMirror(7, false, true);
                reverseJobDone = true;
            }
        } else {
            if (reverseJobDone) {
                mirrors->raiseMirror(7, false, true);
                reverseJobDone = false;
            }
            count = 0;
        }
    }
    // Frame from KOMBI_A1 
    else if (f.can_id == 0x000c) {
        // Read speed frpm V_SIGNAL (Source is in KM/H so convert to MPH)
        engine->speed = ((int) f.data[1] * 5) / 8;
    } else if (f.can_id == 0x009e) {
        engine->odometer_milage = (float) (((long) f.data[3] << 16) + ((int) f.data[4] << 8) + f.data[5]) / 8 * 5;
    }
}

void Car::processBluetoothRequest() {
    bluetooth->readMessage();
    int len = strlen(bluetooth->message);
    char* ptr = bluetooth->message;
    if (len > 0) {
        phoneConnected = true;
        DPRINTLN("Incomming message -> " + String(bluetooth->message));
        // Track data
        if (ptr[0] == 'M') {
            if (ptr[2] == 'X' && len == 3) {
                music->pause();
            } else if (ptr[2] == 'P' && len == 3) {
                music->play();
            } else if (ptr[1] == '-') {
                music->setText(ptr+2);
            } else if (ptr[1] == ' '){
                int duration = (byte) ptr[2] * 256 + (byte)ptr[3];
                music->setSeconds(duration);
            } else {
            }
            updateMusic();
        } else if (ptr[0] == 'B') {
            ic->setBody(ptr+2);
        } else if (ptr[0] == '!') {
            music->pause();
            ic->setBody(APP_EXIT_TXT);
            phoneConnected = false;
        }
    }
}

void Car::processKeyPress(can_frame* f) {
    wheelControls::key k = wheel->getPressed(f);
    if(!ic->inDiagMode && IC_DISPLAY::page == IC_DISPLAY::PAGES::AUDIO) {
        switch(k) {
            case wheelControls::ArrowUp:
                if (phoneConnected) bluetooth->writeMessage("N");
                break;
            case wheelControls::ArrowDown:
                if (phoneConnected) bluetooth->writeMessage("P");
                break;
            case wheelControls::TelUp:
                ic->diagSetHeader();
                ic->inDiagMode = true;
                break;
            case wheelControls::TelDown:
            case wheelControls::ArrowUpLong:
            case wheelControls::ArrowDownLong:
            case wheelControls::TelUpLong:
            case wheelControls::TelDownLong:
            default:
                break;
        }
    } else if (IC_DISPLAY::page == IC_DISPLAY::PAGES::AUDIO){
        switch(k) {
            case wheelControls::ArrowUp:
                ic->nextDiagPage();
                break;
            case wheelControls::ArrowDown:
                ic->prevDiagPage();
                break;
            case wheelControls::TelDown:
                ic->inDiagMode = false;
                updateMusic();
                break;
            case wheelControls::TelUp:
            case wheelControls::ArrowUpLong:
            case wheelControls::ArrowDownLong:
            case wheelControls::TelUpLong:
            case wheelControls::TelDownLong:
            default:
                break;
        }
    }
}

void Car::updateMusic() {
    if (music->isPlaying()) {
        if (strlen(music->getDisplayText()) != 0) {
            ic->setBody(music->getDisplayText());
        } else {
            ic->setBody(UNKNOWN_TRACK);
        }
    } else {
        ic->setHeader(TRACK_PAUSED_HEAD);
        ic->setBody(TRACK_PAUSED);
    }
}

int lastProg = 0;
void Car::drawMusicProgress() {
    if(millis() - lastUpdateMillis > 1000) {
        lastUpdateMillis = millis();
        if (!ic->inDiagMode) {
            if(music->isPlaying() && lastProg != music->progressPercent) {
                char txt[4];
                sprintf(txt, "%d%%", music->progressPercent);
                lastProg = music->progressPercent;
                ic->setHeader(txt);
            }
        }
    }
}


void Car::loop() {
    if(!isLocked) {
        processBluetoothRequest();
        ic->update();
        if (phoneConnected) {
            music->update();
            drawMusicProgress();
        }
        processCanFrame();
    } else {
        processCanFrame();
    }
}


