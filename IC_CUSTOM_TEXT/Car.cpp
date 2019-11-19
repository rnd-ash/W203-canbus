#include "Car.h"


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
}

void Car::processCanFrame() {
    can_frame f = c->pollForFrame(CAN_BUS_B);
    if (f.can_id == 0x1CA) {
        processKeyPress(&f);
    } else if (f.can_id == 0x0000 && f.can_dlc > 0) {
        if (f.data[5] == 0xaa && !engine->isOn) {
            isLocked = true;
            if (!lockJobDone) {
                can_frame f;
                f.can_id = 0x0045;
                f.can_dlc = 4;
                f.data[3] = 0x44;
                f.data[4] = 0x04;
                for (int i = 0; i < 2000; i++) {
                    c->sendFrame(CAN_BUS_B, &f);
                    delay(5);
                }
                lockJobDone = true;
            }
        } else {
            isLocked = false;
            lockJobDone = false;
        }
    } else if (f.can_id == 0x0002) {
        engine->rpm = (f.data[2] << 8) | (f.data[3]);
        engine->isOn = f.data[2] == 0xFF;
    }
}

void Car::processBluetoothRequest() {
    bluetooth->readMessage();
    int len = strlen(bluetooth->message);
    char* ptr = bluetooth->message;
    if (len > 0) {
        DPRINTLN("Incomming message -> "+String(bluetooth->message));
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
            }
            updateMusic();
        } else if (ptr[0] == 'B') {
            ic->setBody(ptr+2);
        } else {
            String unknown;
            for (int i = 0; i < len; i++) {
                unknown += ptr[i];
            }
            DPRINTLN("Unknown message -> "+unknown);
        }
    }
}

void Car::processKeyPress(can_frame* f) {
    wheelControls::key k = wheel->getPressed(f);
    if(!ic->inDiagMode) {
        switch(k) {
            case wheelControls::ArrowUp:
                bluetooth->writeMessage("N");
                break;
            case wheelControls::ArrowDown:
                bluetooth->writeMessage("P");
                break;
            case wheelControls::TelUp:
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
    } else {
        switch(k) {
            case wheelControls::ArrowUp:
                ic->nextDiagPage();
                break;
            case wheelControls::ArrowDown:
                ic->prevDiagPage();
                break;
            case wheelControls::TelDown:
                ic->inDiagMode = false;
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
        ic->setBody(music->getTrack());
        DPRINTLN(String(music->getTrack()));
    } else {
        ic->setBody("--PAUSED--");
    }
}

void Car::drawMusicProgress() {
    if(millis() - lastUpdateMillis > 1000) {
        lastUpdateMillis = millis();
        if(music->isPlaying()  && music->totalSeconds > 1) {
            String txt = String(music->elapsedSeconds)+"/"+String(music->totalSeconds);
            //ic->setHeader(txt.c_str()); // Max of 8 chars
            ic->setHeader("AS3 ");
        }
    }
}


void Car::loop() {
    if(!isLocked) {
        processCanFrame();
        processBluetoothRequest();
        ic->update();
        music->update();
        drawMusicProgress();
    } else {
        processCanFrame();
    }
}


