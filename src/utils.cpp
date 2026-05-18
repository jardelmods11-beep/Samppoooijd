#include "utils.h"
#ifdef IOS_PORT
#include <sys/time.h>

unsigned int GetTickCount() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return (unsigned int)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

    *dst = 0;
}
#endif
