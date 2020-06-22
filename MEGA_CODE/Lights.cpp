#include "Lights.h"

namespace Lights {
    void triggerSAMA3(uint8_t* args) {
        can_frame s = can_frame {
            0x000E,
            0x02,
            args[0],
            args[1]
        };
        canB->sendMessage(&s);
    }

    void triggerSAMA5(uint8_t* args) {
        can_frame s = can_frame {
            0x0230,
            0x02,
            args[0],
            args[1]
        };
        canB->sendMessage(&s);
    }
}