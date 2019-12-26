#ifndef WHEEL_CONTROLS_H
#define WHEEL_CONTROLS_H

#include "can_comm.h"
#include "defines.h"

/**
 * Used to decode information from KOMBI_A5 regarding steering wheel button pressses.
 * 
 * The frames from this module contain 2 pieces of relevent information:
 * 
 * Byte 1 contains information for the AGW (not sure about what though)
 * Byte 2 however contains the button press being registered from the steering wheel
 * 
 * -- Byte 2 format --
 * 10000000 (0x80) Phone End button is pressed
 * 01000000 (0x40) Phone Answer button is pressed
 * 00100000 (0x20) Volume down is pressed
 * 00010000 (0x10) Volume up is pressed
 * 00001000 (0x08) UNKNOWN
 * 00000100 (0x04) UNKNOWN
 * 00000010 (0x02) Prev page button is pressed (Arrow down)
 * 00000001 (0x01) Next page button is pressed (Arrow up)
 * 00000000 (0x00) Nothing is pressed
 * 
 * Because this is quite limited, (Only 4 buttons can freely be used),
 * I have implimented additional [WHEEL_KEY] enum objects to represent
 * long presses. These are:
 * 
 * (0x05) Arrow up long press
 * (0x06) Arrow Down long press
 * (0x41) Phone answer long press
 * (0x81) Phone end long press 
 */
class WHEEL_CONTROLS {
public:
    enum WHEEL_KEY {
        NONE = 0x00,
        ARROW_UP = 0x01,
        ARROW_DOWN = 0x02,
        TELEPHONE_ANSWER = 0x40,
        TELEPHONE_HANGUP = 0x80,
        VOLUME_UP = 0x10,
        VOLUME_DOWN = 0x20,

        // Additional long presses (Custom)
        ARROW_UP_LONG = 0x05,
        ARROW_DOWN_LONG = 0x06,
        TELEPHONE_ANSWER_LONG = 0x41,
        TELEPHONE_HANGUP_LONG = 0x81
    };

    WHEEL_KEY getPressed(can_frame *f);
private:
    WHEEL_KEY get_key(can_frame *f);
    unsigned long lastPressTimeMS = 0;
    WHEEL_KEY last_registered = WHEEL_KEY::NONE;

};

#endif