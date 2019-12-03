#include "canbuscomm.h"

/**
 * Controls the car mirrors
 */
class Mirrors {
    public:
        Mirrors(CanbusComm *c);
        /**
         * Lowers any, or both of the 2 mirrors on the car
         * @param passengers Should lower passengers (left) mirror)
         * @param drivers Should lower drivers (right) mirror?
         * @param degrees approx number of degrees to lower the mirror
         */
        void lowerMirror(uint8_t degrees, bool drivers, bool passengers);
        /**
         * Raises any, or both of the 2 mirrors on the car
         * @param passengers Should raise passengers (left) mirror)
         * @param drivers Should raise drivers (right) mirror?
         * @param degrees approx number of degrees to raise the mirror
         */
        void raiseMirror(uint8_t degrees, bool drivers, bool passengers);
    private:
        CanbusComm *c;
    
};