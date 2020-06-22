#ifndef BLUETOOTH_H_
#define BLUETOOTH_H_

#include "HardwareSerial.h"

// Bluetooth command ID's from app
#define BT_CMD_PING 0xFF // Ping to arduino
#define PING_TIMEOUT 10000; // Milliseconds
// -- Music control stuff --
#define BT_CMD_TRACK_NAME 0x00 // Track name provided
#define BT_CMD_MUSIC_CTRL 0x01 // Control status provided
#define BT_CMD_TRACK_LEN  0x02 // New track length provided
#define BT_CMD_TRACK_SEEK 0x03 // New track seek position provided

// -- Music statuses. These are sent / received as args for BT_CMD_MUSIC_CTRL  --
// Music control args - From phone app
#define BT_PAUSE 0x00
#define BT_PLAY 0x01
// Sent to phone on steering wheel command
#define BT_NEXT 0x02
#define BT_PREV 0x03

// -- Lights show stuff -- 
#define BT_CMD_LIGHT_SHOW 0x04;
#define BT_LIGHT_SAM_A5 0x01; // Control SAM A5 (for dipped and fogs)
#define BT_LIGHT_SAM_A3 0x02; // Control SAM A3 (for indicator lights)
namespace Bluetooth {
    struct Payload {
        uint8_t argSize;
        uint8_t cmd;
        uint8_t args[254];
    };
    
    void begin(int baud);
    bool getPayload(Payload* p);
    void sendPayload(Payload *s);
    bool isConnected();
};

#endif