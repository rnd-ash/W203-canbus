#include "ic.h"

#define STATIC_UPDATE_FREQ 1000
#define SCROLL_UPDATE_FREQ 200

class Audio_Page {
    public:
        Audio_Page(IC_DISPLAY *ic);
        
        void update();
        /**
         * Sets the Body text of the display
         * @param body Body text to display
         */
        void setText(const char* body);
        /**
         * Sets the upper and lower symbol of the audio page
         * @param upper Upper symbol to display
         * @param lower Bottom symbol to display
         */
        void setSymbols(IC_DISPLAY::SYMBOL upper, IC_DISPLAY::SYMBOL lower);
        /**
         * Sets the header text
         * @param header Header text to display
         */
        void setHeader(const char* header);

        void enableDiagMode();
        void disableDiagMode();
    private:
        bool isInDiagMode = false;
        /** Rotates string in bodyText by 1 character **/
        void rotateText();
        /** Used to store header string information **/
        char headerText[13];
        /** Used to store full body string **/
        char bodyText[64];
        /** A small portion of bodyText used when we are rotating string **/
        char rotatingText[11];
        /** Represents weather the AUDIO page is currently being displayed or not on the IC **/
        bool isInPage = false;
        /** Signals to scroll and rotate text rather than use static text for long strings **/
        bool isScrolling = false;
        /** Last time the body text was updated **/
        unsigned long lastUpdatetime;
        /** IC_DISPLAY object **/
        IC_DISPLAY *display;
};