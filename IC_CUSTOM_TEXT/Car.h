#ifndef W203_CANBUS_CAR_H
#define W203_CANBUS_CAR_H
#include "canbuscomm.h"
#include "wheel_controls.h"
#include "ic.h"
#include "phoneBluetooth.h"
#include "Music.h"
#include "mirror_controls.h"
#include "Audio_Display.h"

class Car {
    public:
        Car(CanbusComm *c);
        void loop();
    private:
        /**
         * Processes an incomming can frame
         */
        void processCanFrame();
        /**
         * Processes a key press from can frame f (ID = 0x1CA)
         */
        void processKeyPress(can_frame* f);
        /**
         * Processes an incomming message from bluetooth
         */
        void processBluetoothRequest();
        /**
         * Updates music
         */
        void updateMusic();
        void soundHorn();
        void doLightShow();
        void flash_dipped_beam(uint8_t msec);
        void flash_indicatorLights(uint8_t id, uint8_t msec);
        unsigned long lastUpdateMillis;
        void drawMusicProgress();
        wheelControls *wheel;
        CanbusComm *c;
        phoneBluetooth *bluetooth;
        Music *music;
        Audio_Page* audio;
        EngineData *engine;
        Mirrors *mirrors;
        bool isLocked;
        bool lockJobDone;
        bool phoneConnected = false;
};

#endif