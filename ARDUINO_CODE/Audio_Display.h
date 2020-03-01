#ifndef AUDIO_DISPLAY_H
#define AUDIO_DISPLAY_H

#include "ic_display.h"
#include "Engine.h"

const char * const PROGMEM STARTUP_HEADER = "No Connection";
const char * const PROGMEM STARTUP_BODY = "Open App!";
const char * const PROGMEM STARTUP_CONNECTED_HEADER = "App open";
const char * const PROGMEM STARTUP_CONNECTED_BODY = "Play Music";

const char * const PROGMEM PAUSED_HEADER = "Paused";
const char * const PROGMEM PAUSED_BODY = "Press play";
const char * const PROGMEM DIAG_MODE_HEADER = "Scroll";
const char * const PROGMEM DIAG_MODE_BODY = "DIAG MODE";


const char * const PROGMEM DIAG_HEADER_ATF = "ATF Temp";
const char * const PROGMEM DIAG_HEADER_TORQUE_CONVERTER = "TC lockup";
const char * const PROGMEM DIAG_HEADER_GEARING = "Tar/Act gear";
const char * const PROGMEM DIAG_HEADER_COOLANT_TEMP = "Eng Temp";
const char * const PROGMEM DIAG_HEADER_INTAKE_TEMP = "Intake Temp";
const char * const PROGMEM DIAG_HEADER_CONSUMPTION = "Fuel usage";
const char * const PROGMEM DIAG_HEADER_OIL_TEMP = "Oil Temp";

const char * const PROGMEM DIAG_ERROR_HEAD = "INVALID";
const char * const PROGMEM DIAG_ERROR_BODY = "ERROR";

/**
 * Audio screen handler
 * 
 * One class can send data to this at once. The sending classes can define
 * what contents and formats to show on the header text and body text of the page,
 * as well as set symbols.
 * 
 * The update function in this class then deals with sending data to the IC using
 * the IC display class.
 * 
 * The Audio page has the following that can be updated:
 * 
 * |----------|
 * |  HEADER  | <-- Header text (Max 11 chars)
 * |          |
 * |    A     | <-- Top symbol
 * |   BODY   | <-- Body text (Max 56px in text). Any more will trigger text scrolling
 * |    B     | <-- Bottom symbol
 * |          |
 * |          |
 * |----------|
 */
class AUDIO_DISPLAY {
    public:
        AUDIO_DISPLAY(IC_DISPLAY *d);
        /**
         * Updates the UI on the car 
         */
        void update();

        /**
         * Sets the header text for the Audio page
         * @param text Text to display on the header
         * @param format Format byte. See IC Display class for details
         * @param refreshInterval How often should the IC update the header?
         */
        void setHeader(const char* text, uint8_t format, int refreshInterval);

        /**
         * Sets the body text for the Audio page
         * @param text Text to display on the body line
         * @param format Format byte. See IC Display class for details
         * @param refreshInterval How often should the IC update the body?
         */
        void setBody(const char* text, uint8_t format, int refreshInterval);

        /**
         * Sets the symbols on the audio page
         * @param topSymbol Top symbol to display above the body text
         * @param bottomSymbol Bottom to display under the body text
         */
        void setSymbols(uint8_t topSymbol, uint8_t bottomSymbol);
    private:
        IC_DISPLAY *display;
        char header[24] = {0x00};
        char body[255] = { 0x00 };
        uint8_t body_crc = 0x00;
        uint8_t header_crc = 0x00;
        bool scrollingRequired = false;
        uint8_t getCRC(const char* str);
        int headerRefreshMsec = 1000;
        int bodyRefreshMsec = 2000;
        uint8_t topSymbol = 0x00;
        uint8_t bottomSymbol = 0x00;
        uint8_t bodyFmt = 0x00;
        uint8_t headFmt = 0x00;
        unsigned long lastUpdateBody = millis();
        unsigned long lastUpdateHeader = millis();
        bool isInPage = false;
};
#endif