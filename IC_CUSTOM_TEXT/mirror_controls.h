#include "canbuscomm.h"

/**
 * Controls the car mirrors
 */
class Mirrors {
    public:
        Mirrors(CanbusComm *c);
        /**
         * 
         * @param passengers Should lower passengers (left) window)
         * @param drivers Should lower drivers (right) window?
         * @param degrees approx number of degrees to lower the mirror
         */
        void lowerMirror(uint8_t degrees, bool drivers, bool passengers);
        void raiseMirror(uint8_t degrees, bool drivers, bool passengers);
    private:
        CanbusComm *c;
    
};