#ifndef DIAG_MODE_H

#define DIAG_MODE_H

#include "defines.h"
#include "can_comm.h"
#include <stdio.h>
#include "Audio_Display.h"
#include "Engine.h"

#define MAX_DIAG_PAGES 8

class DIAG_MODE {

    public:
        DIAG_MODE(AUDIO_DISPLAY *disp, ENGINE_DATA *eng);
        void updateUI();
        void nextDiagPage();
        void prevDiagPage();
    private:
        ENGINE_DATA *engine;
        AUDIO_DISPLAY *display;
        uint8_t displayPage = 1;
};

#endif

