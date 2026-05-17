#ifndef __UTILS_H__
#define __UTILS_H__

#include <cstdio>
#include <cstdarg>
#include <ctime>
#include <cstdlib>

#ifdef IOS_PORT

inline void samp_log(const char* fmt, ...) {
    va_list a; va_start(a, fmt);
    vfprintf(stderr, fmt, a);
    fprintf(stderr, "\n");
    va_end(a);
}
#define log samp_log

inline void TemporaryFPSVisualization() {}
inline void emu_GammaSet(int) {}

unsigned int GetTickCount();
void AsciiToGxtChar(const char* src, unsigned short* dst);

#else
#include <android/log.h>
#define log(...) __android_log_print(ANDROID_LOG_DEBUG, "SAMP", __VA_ARGS__)
void TemporaryFPSVisualization();
void emu_GammaSet(int v);
unsigned int GetTickCount();
void AsciiToGxtChar(const char* src, unsigned short* dst);
#endif

#endif
