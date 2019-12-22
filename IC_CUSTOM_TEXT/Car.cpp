#include "Car.h"

const char * const PROGMEM APP_EXIT_TXT = "APP EXIT!";
const char * const PROGMEM UNKNOWN_TRACK = "Track: ??";
const char * const PROGMEM TRACK_PAUSED = "AUX ";
const char * const PROGMEM TRACK_PAUSED_HEAD = "----";
Car::Car(CanbusComm *c) {
    this->engine = new EngineData();
    this->audio = new Audio_Page(new IC_DISPLAY(c, engine));
    this->wheel = new wheelControls();
    this->bluetooth = new phoneBluetooth(6, 7);
    this->c = c;
    this->music = new Music();
    this->lastUpdateMillis = millis();
    this->isLocked = false;
    this->lockJobDone = false;
    this->mirrors = new Mirrors(c);
    #ifdef SIMULATION
        ser = new SoftwareSerial(8,3);
        DPRINTLN("SERIAL READY");
    #endif


}

uint8_t count = 0;
bool reverseJobDone = false;
void Car::processCanFrame() {
    // Poll for a can frame on bus B (Interior Can)
    can_frame f = c->pollForFrame(CAN_BUS_B);
    // Key press related frame
    if (f.can_id == 0x1CA || f.can_id == 0x1D0) {
        processKeyPress(&f);
    } 
    // Frame from EZS_A1
    else if (f.can_id == 0x0000 && f.can_dlc > 0) {
        
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
    else if (f.can_id == 0x0030) {
        engine->internal_temp_c = (int) f.data[7] - 40;
        engine->ac_fan_percent = (int) f.data[3];
    } else if (f.can_id == 0x0004) {
        if (f.data[1] == 0x04 && f.data[2] == 0x40) {
            isLocked = true;
            if (!lockJobDone) {
                lockJobDone = true;

            }
        } else {
            isLocked = false;
            lockJobDone = false;
        }
    } else if (f.can_id == 0x01D0) {
        DPRINTLN("IC >> AGW: "+*c->frameToString(&f));
    } else if (f.can_id == 0x01A0) {
        DPRINTLN("IC << AGW: "+*c->frameToString(&f));
    }
}

void Car::processBluetoothRequest() {
    bluetooth->readMessage();
    char* ptr = bluetooth->message;
    int len = strlen(bluetooth->message);
    if (len > 0) {
        phoneConnected = true;
        DPRINTLN("Incomming message -> " + String(ptr));
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
                drawMusicProgress();
            } else if (ptr[1] == '_'){
                int seek = (byte) ptr[2] * 256 + (byte)ptr[3];
                music->setElapsed(seek);
                drawMusicProgress();
            } else {}
            updateMusic();
        } else if (ptr[0] == 'B') {
            // ptr + 2
        } else if (ptr[0] == '!') {
            music->pause();
            phoneConnected = false;
        } else if (ptr[0] == 'D') {
            #ifdef SIMULATION
            ser->begin(9600);
            #endif
            doLightShow();
            #ifdef SIMULATION
            this->bluetooth->debugInit();
            #endif
        } else if (ptr[0] == 'F') {
            #ifdef SIMULATION
            ser->begin(9600);
            #endif
            flash_dipped_beam(1000);
            #ifdef SIMULATION
            this->bluetooth->debugInit();
            #endif
        } else if (ptr[0] == 'G') {
            soundHorn();
        }
    }
}

void Car::processKeyPress(can_frame* f) {
    wheelControls::key k = wheel->getPressed(f);
    if(IC_DISPLAY::page == IC_DISPLAY::DISPLAY_PAGE::AUDIO) {
        switch(k) {
            case wheelControls::ArrowUp:
                if (phoneConnected) bluetooth->writeMessage("N");
                break;
            case wheelControls::ArrowDown:
                if (phoneConnected) bluetooth->writeMessage("P");
                break;
            case wheelControls::TelUp:
            case wheelControls::TelDown:
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
            audio->setText(music->getDisplayText());
        } else {
            audio->setText(UNKNOWN_TRACK);
        }
    } else {
        audio->setHeader("Not Playing ");
        delay(20);
        audio->setText("PAUSED");
    }
}

int lastProg = 0;
void Car::drawMusicProgress() {
    if(millis() - lastUpdateMillis > 1000) {
        lastUpdateMillis = millis();
        if(music->isPlaying() && lastProg != music->progressPercent) {
            char txt[14];
            sprintf(txt, "Playing %d%%", music->progressPercent);
            lastProg = music->progressPercent;
            audio->setHeader(txt);
            delay(50);
        }
    }
}

void Car::loop() {
    audio->update();
    if(!isLocked) {
        processBluetoothRequest();
        if (phoneConnected) {
            music->update();
            drawMusicProgress();
        }
        processCanFrame();
    } else {
        processCanFrame();
    }
}

void Car::soundHorn() {
    can_frame x;
    x.can_id = 0x0006;
    x.can_dlc = 5;
    x.data[0] = 0xA0;
    x.data[1] = 0x00;
    x.data[2] = 0x00;
    x.data[3] = 0x00;
    x.data[4] = 0x09;
    for (int i = 0; i < 2; i++) {
        c->sendFrame(CAN_BUS_B, &x);
        delay(10);
    }
    delay(300);
    for (int i = 0; i < 2; i++) {
        c->sendFrame(CAN_BUS_B, &x);
        delay(10);
    }
}

void Car::flash_indicatorLights(uint8_t id, uint8_t msec) {
    can_frame x;
    x.can_id = 0x000E;
    x.can_dlc = 2;
    if (id == 0) {
        x.data[0] = 0xE0;
        #ifdef SIMULATION
        char snd[3] = {'H', ':', msec};
        writeToSim(snd);
    #endif
    } else if (id == 1) {
        x.data[0] = 0x80;
        #ifdef SIMULATION
       char snd[3] = {'R', ':', msec};
        writeToSim(snd);
    #endif
    } else {
        x.data[0] = 0x40;
        #ifdef SIMULATION
        char snd[3] = {'L', ':', msec};
        writeToSim(snd);
    #endif
    }
    x.data[1] = msec;
    c->sendFrame(CAN_BUS_B, &x);
}

void Car::flash_dipped_beam(uint8_t msec) {
    /*
    can_frame x;
    x.can_id = 0x0230;
    x.can_dlc = 2;
    x.data[0] = 0b01000000;
    x.data[1] = msec;
    c->sendFrame(CAN_BUS_B, &x);
    #ifdef SIMULATION
        char snd[3] = {'D', ':', msec};
        writeToSim(snd);
    #endif
    */
   audio->setSymbols(IC_DISPLAY::SYMBOL::PLAY, IC_DISPLAY::SYMBOL::NEXT_TRACK);
}

#ifdef SIMULATION
void Car::writeToSim(const char *s) {
    DPRINTLN("Sending");
    DPRINTLN((uint8_t) s[0]);
    for (int i = 0; i < 3; i++) {
        ser->write(s[i]);
    }
    ser->flush();
}
#endif

// Wizards of Winter
// 148 BPM
// 1 note -> 405.4MS
// 1/2 Note -> 203 MS
void Car::doLightShow() {
    delay(250);
    for (int i = 0; i < 12; i++) {
        flash_indicatorLights(1,100);
        delay(203);
        flash_indicatorLights(2,100);
        delay(203);
    }
    flash_dipped_beam(255);
    flash_indicatorLights(0, 100);
    delay(100);
    flash_dipped_beam(255);
}