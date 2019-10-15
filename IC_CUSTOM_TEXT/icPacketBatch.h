//
// Created by ashcon on 10/9/19.
//

#ifndef W203_CANBUS_ICPACKETBATCH_H
#define W203_CANBUS_ICPACKETBATCH_H

#include "can.h"
class icPacketBatch {
public:
    int numberOfFrames;
    can_frame frames[10];
private:
    can_frame currentFrame;
};


#endif //W203_CANBUS_ICPACKETBATCH_H
