#ifndef W203_CANBUS_CAR_H
#define W203_CANBUS_CAR_H
#include "canbuscomm.h"
#include "wheel_controls.h"
#include "ic.h"
#include "phoneBluetooth.h"
#include "Music.h"
#include "mirror_controls.h"

class Car {
    public:
        Car(CanbusComm *c);
        void loop();
    private:
        void processCanFrame();
        void processKeyPress(can_frame* f);
        void processBluetoothRequest();
        void updateMusic();
        void windUpWindows();
        unsigned long lastUpdateMillis;
        void drawMusicProgress();
        wheelControls *wheel;
        CanbusComm *c;
        phoneBluetooth *bluetooth;
        Music *music;
        IC_DISPLAY *ic;
        EngineData *engine;
        Mirrors *mirrors;
        bool isLocked;
        bool lockJobDone;
        bool phoneConnected = false;
};

#endif