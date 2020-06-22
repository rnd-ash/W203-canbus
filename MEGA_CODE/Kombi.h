#ifndef KOMBI_H_
#define KOBI_H_

#include "Canbus.h"

// --- Can ID's for KOMBI ---
#define CID_KOMBI_A1 0x000C
#define CID_KOMBI_A5 0x01CA

// --- Page ID's from Kombi ---
#define KOMBI_PAGE_PHONE 0x05
#define KOMBI_PAGE_AUDIO 0x03
#define KOMBI_PAGE_OTHER 0x02

// --- Key press registers ---
#define BUTTON_PHONE_END  0b10000000
#define BUTTON_PHONE_SEND 0b01000000
#define BUTTON_VOL_DOWN   0b00100000
#define BUTTON_VOL_UP     0b00000100
// - RESERVED -
// 0b00010000
// 0b00100000
#define BUTTON_PAGE_PREV  0b00000010
#define BUTTON_PAGE_NEXT  0b00000001

// -- Custom key presses (Long press) --
#define BUTTON_PHONE_END_LONG  0b10000100
#define BUTTON_PHONE_SEND_LONG 0b01000100
#define BUTTON_PAGE_PREV_LONG  0b00000110
#define BUTTON_PAGE_NEXT_LONG  0b00000101

#define TIME_TO_LONG_PRESS 2000 // Milliseconds

/**
 * Stores data about KOMBI_A*** CAN IDs
 * which come from the Instrument cluster's (IC) ECU
 *
 * These include:
 *  1. Steering wheel control inputs
 *  2. Speed of car (Km/h)
 *  3. Trip computer units (Used for Live MPG calculations)
 *  4. Current page in the IC display
 */
class Kombi {
public:
    /**
     * Processes an incomming can frame 
     */
    void processKombiFrame(can_frame *f);

    /**
     * Returns a uint representing the current key press 
     * being pressed on the steering wheel
     */
    uint8_t getKeyPress();

    /**
     * Returns a uint representing the current page the IC
     * is in. Can either be audio, phone, or other
     */
    uint8_t getICPage();

    /**
     * Returns true if the IC is using Imperial units,
     * false if IC is using Metric
     */
    bool useImperial();
    uint8_t vSpeedKmh = 0; // Vehicle speed in KMH
private:
    /**
     * Processes can frame 0x000C (KOMBI_A1),
     * which contains details about the settings
     * on the IC
     */
    void processFrameA1(can_frame *f);

    /**
     * Processes can frame 0x01CA (KOMBI_A5),
     * which contains details about IC page
     * and key inputs
     */
    void processFrameA5(can_frame *f);

    // -- Stuff for registering custom long presses --
    uint8_t currKeyPress;
    unsigned long lastPollTime = millis();
    int keyPressDuration = 0;
    bool keyPressed = false;

    // -- Settings and other values --
    bool isUsingImperial = false; // Metric by default
    uint8_t currPage = 0;
};

extern Kombi* kombi;

#endif