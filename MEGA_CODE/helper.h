#ifndef HELPER_H_
#define HELPER_H_

#include "can.h"

#define DEBUG

#define LOG_ERR(...) Serial.print(F("[ERROR] ")); printf_P(__VA_ARGS__)
#define LOG_WARN(...) Serial.print(F("[WARN ] ")); printf_P(__VA_ARGS__)
#define LOG_INFO(...) Serial.print(F("[INFO ] ")); printf_P(__VA_ARGS__)
// Flash string helper to const char*
#define F_TC(x) (const char*)F(x)
#ifdef DEBUG
    #define LOG_DEBG(...) Serial.print(F("[DEBUG] ")); printf_P(__VA_ARGS__)
#else
    #define LOG_DEBG(...)
#endif

#endif