#ifndef WHEEL_CONTROLS_H
#define WHEEL_CONTROLS_H

#include "can_comm.h"
#include "defines.h"


// Standard button presses directly from wheel
#define BUTTON_NONE 0x00
#define BUTTON_ARROW_UP 0x01
#define BUTTON_ARROW_DOWN 0x02
#define BUTTON_TEL_ANS 0x40
#define BUTTON_TEL_DEC 0x80
#define BUTTON_VOL_UP 0x10
#define BUTTON_VOL_DOWN 0x20

// Additional buttons via software (detecting long presses)
#define BUTTON_ARROW_UP_LONG 0x05
#define BUTTON_ARROW_DOWN_LONG 0x06
#define BUTTON_TEL_ANS_LONG 0x41
#define BUTTON_TEL_DEC_LONG 0x81

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
    WHEEL_CONTROLS();
    uint8_t getPressed(can_frame *f);
private:
    uint8_t get_key(can_frame *f);
    unsigned long lastPressTimeMS = 0;
    uint8_t last_registered;

};

#endif