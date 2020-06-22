#ifndef LIGHTS_H_
#define LIGHTS_H_

#include "Canbus.h"

namespace Lights {
    /**
     * Sends a command to SAM A3
     * Bit 0 - Right turn signal
     * Bit 1 - Left turn signal
     * Bit 2 - Hazards (Both)
     * Bit 8-15 - Time on measures as value*10 ms
     */
    void triggerSAMA3(uint8_t* args);

    /**
     * Sends a command to SAM A3
     * Bit 0 - Fog lights
     * Bit 1 - Dipped beam
     * Bit 8-15 - Time on measures as value*10 ms
     */
    void triggerSAMA5(uint8_t* args);
}

#endif