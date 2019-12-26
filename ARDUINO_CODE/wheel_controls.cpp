#include "wheel_controls.h"


WHEEL_CONTROLS::WHEEL_KEY WHEEL_CONTROLS::getPressed(can_frame *f) {
    // Frame is not from the steering wheel KOMBI_A5 module
    if (f->can_id != 0x01CA) {
        return NONE;
    }

    WHEEL_KEY tmp = get_key(f);
    // Let go of button / Not pressed
    if (tmp == WHEEL_KEY::NONE) {
        // User was holding down a key
        if (last_registered != WHEEL_KEY::NONE) {
            // User has held down the key for some time now
            if (millis() - lastPressTimeMS >= WHEEL_BUTTON_LONG_PRESS_MS) {
                switch(last_registered) {
                    case ARROW_DOWN:
                        return ARROW_DOWN_LONG;
                    case ARROW_UP:
                        return ARROW_UP_LONG;
                    case TELEPHONE_ANSWER:
                        return TELEPHONE_ANSWER_LONG;
                    case TELEPHONE_HANGUP_LONG:
                        return TELEPHONE_HANGUP_LONG;
                    default:
                        // Key was held down that doesn't have a long press companion enum
                        return last_registered;
                }
            }
            // Short key tap 
            else {
                tmp = last_registered;
                last_registered = NONE;
                return tmp;
            }
        }
        // User wasn't holding anything down so return none
        return WHEEL_KEY::NONE;
    }

    // New key tap! Start the clock
    if (tmp != last_registered) {
        lastPressTimeMS = millis();
        last_registered = tmp;
    } 

    return WHEEL_KEY::NONE;
}

WHEEL_CONTROLS::WHEEL_KEY WHEEL_CONTROLS::get_key(can_frame *f) {
    switch(f->data[1]) {
        case 0x01:
            return ARROW_UP;
        case 0x02:
            return ARROW_DOWN;
        case 0x40:
            return TELEPHONE_ANSWER;
        case 0x80:
            return TELEPHONE_HANGUP;
        default:
            return NONE;
    }
}
