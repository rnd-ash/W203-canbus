//#define DEBUG
#ifdef DEBUG
  #define DPRINT(...) Serial.print(__VA_ARGS__)
  #define DPRINTLN(...) Serial.println(__VA_ARGS__)
#else
  #define DPRINT(...)
  #define DPRINTLN(...)
#endif

// comment out if you own a Arduino uno! or other smaller board
#define ARDUINO_MEGA

// Comment out if you want MPG in US form, not UK
#define REGION_UK

#define SYSTEM_SRAM_SIZE 2048

// Audio page settings
#define UPDATE_FREQ_HEADER 1000
#define UPDATE_FREQ_SCROLL_BODY 200
#define DIAG_MODE_UPDATE_FREQ 250

// Steering wheel button press detections
#define WHEEL_BUTTON_LONG_PRESS_MS 1000 // If button is held down for this long then its registered as a long tap