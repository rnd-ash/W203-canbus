#include "Bluetooth.h"
#include "helper.h"
#include <Arduino.h>


unsigned long lastPingTime = millis();
namespace Bluetooth {
    
    void begin(int baud) {
        Serial1.begin(baud);
        Serial1.clearWriteError();
        LOG_INFO(F_TC("Bluetooth ready!\n"));
    }
    
    bool getPayload(Payload* p) {
        if (Serial1.available()) {
            memset(p, 0x00, sizeof(struct Payload));
            uint8_t cmdSize = Serial1.read();
            Serial1.readBytes(&p->cmd, cmdSize);
            p->argSize = cmdSize-1;
            if (p->cmd == BT_CMD_PING) {
                lastPingTime = millis();
                return false;
            }
            return true;
        }
        return false;
    }

    void sendPayload(Payload *s) {
        Serial1.write(s->argSize+1);
        Serial1.write(s->cmd);
        for (int i = 0; i < s->argSize; i++) {
            Serial1.write(s->args[i]);
        }
    }

    bool isConnected() {
        return millis() - lastPingTime <= PING_TIMEOUT;
    }
}