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
    } else if (f.can_id == 0x0002) {
        engine->rpm = (f.data[2] << 8) | (f.data[3]);
        engine->isOn = f.data[2] != 0xFF;
        engine->coolantTemp = f.data[4] - 40;
    } else if (f.can_id == 0x000c) {
        engine->speed = ((int) f.data[1] * 5) / 8;
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
            ic->setBody("APP EXIT!");
            phoneConnected = false;
        } else {
            String unknown;
            for (int i = 0; i < len; i++) {
                unknown += ptr[i];
            }
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
                ic->setHeader("BLTH");
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
        if (strlen(music->getDisplayText()) != 0) {
            ic->setBody(music->getDisplayText());
        } else {
            ic->setBody("Track: ??");
        }
        DPRINTLN(String(music->getDisplayText()));
    } else {
        ic->setBody("PAUSED");
    }
}

void Car::drawMusicProgress() {
    if(millis() - lastUpdateMillis > 1000) {
        lastUpdateMillis = millis();
        if(music->isPlaying()  && music->totalSeconds > 1) {
            String txt = String(music->elapsedSeconds)+"/"+String(music->totalSeconds);
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


