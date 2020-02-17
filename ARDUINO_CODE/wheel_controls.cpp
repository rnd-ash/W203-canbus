#include "wheel_controls.h"


WHEEL_CONTROLS::WHEEL_CONTROLS() {
    this->last_registered = BUTTON_NONE;
}

uint8_t WHEEL_CONTROLS::getPressed(can_frame *f) {
    // Frame is not from the steering wheel KOMBI_A5 module
    if (f->can_id != 0x01CA) {
        return BUTTON_NONE;
    }

    uint8_t tmp = get_key(f);
    // Let go of button / Not pressed
    if (tmp == BUTTON_NONE) {
        // User was holding down a key
        if (last_registered != BUTTON_NONE) {
            // User has held down the key for some time now
            if (millis() - lastPressTimeMS >= WHEEL_BUTTON_LONG_PRESS_MS) {
                switch(last_registered) {
                    case BUTTON_ARROW_DOWN:
                        return BUTTON_ARROW_DOWN_LONG;
                    case BUTTON_ARROW_UP:
                        return BUTTON_ARROW_UP_LONG;
                    case BUTTON_TEL_ANS:
                        return BUTTON_TEL_ANS_LONG;
                    case BUTTON_TEL_DEC:
                        return BUTTON_TEL_DEC_LONG;
                    default:
                        // Key was held down that doesn't have a long press companion enum
                        return last_registered;
                }
            }
            // Short key tap 
            else {
                tmp = last_registered;
                last_registered = BUTTON_NONE;
                return tmp;
            }
        }
        // User wasn't holding anything down so return none
        return BUTTON_NONE;
    }

    // New key tap! Start the clock
    if (tmp != last_registered) {
        lastPressTimeMS = millis();
        last_registered = tmp;
    } 

    return BUTTON_NONE;
}

uint8_t WHEEL_CONTROLS::get_key(can_frame *f) {
    switch(f->data[1]) {
        case 0x01:
            return BUTTON_ARROW_UP;
        case 0x02:
            return BUTTON_ARROW_DOWN;
        case 0x40:
            return BUTTON_TEL_ANS;
        case 0x80:
            return BUTTON_TEL_DEC;
        default:
            return BUTTON_NONE;
    }
}
