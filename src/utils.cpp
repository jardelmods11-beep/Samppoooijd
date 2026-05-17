#include "utils.h"
#ifdef IOS_PORT
#include <sys/time.h>

unsigned int GetTickCount() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return (unsigned int)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

void AsciiToGxtChar(const char* src, unsigned short* dst) {
    while (*src) { *dst++ = (unsigned short)(unsigned char)*src++; }
    *dst = 0;
}
#endif
