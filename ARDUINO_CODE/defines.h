#define DEBUG
#ifdef DEBUG
  #define DPRINT(...) Serial.print(__VA_ARGS__)
  #define DPRINTLN(...) Serial.println(__VA_ARGS__)
#else
  #define DPRINT(...)
  #define DPRINTLN(...)
#endif

#define SYSTEM_SRAM_SIZE 2048

// LED Pins
#define CLOCK_A_PIN 19
#define CLOCK_B_PIN 18
#define BT_RX_LED 14
#define BT_TX_LED 15
#define RX_LED 16
#define TX_LED 17

// Audio page settings
#define UPDATE_FREQ_HEADER 1000
#define UPDATE_FREQ_SCROLL_BODY 175
#define DIAG_MODE_UPDATE_FREQ 500

// Steering wheel button press detections
#define WHEEL_BUTTON_LONG_PRESS_MS 1000 // If button is held down for this long then its registered as a long tap