/**
 * Debugging macro
 * 
 * Uncomment first line to disable debugging printing to Serial
 */

#define DEBUG
#ifdef DEBUG
  #define DPRINT(...) Serial.print(__VA_ARGS__)
  #define DPRINTLN(...) Serial.println(__VA_ARGS__)
#else
  #define DPRINT(...)
  #define DPRINTLN(...)
#endif