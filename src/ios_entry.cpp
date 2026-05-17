#ifdef IOS_PORT
#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <pthread.h>
#include <unistd.h>

#include "hooks.h"
#include "ios_offsets.h"

static FILE* g_logFile = nullptr;

static void init_log() {
    const char* home = getenv("HOME");
    if (home) {
        char path[512];
        snprintf(path, sizeof(path), "%s/Documents/samp.log", home);
        g_logFile = fopen(path, "w");
    }
}

static void* samp_init_thread(void*) {
    sleep(3);
    uintptr_t base = GetGTASABase();
    if (base == 0) return nullptr;
    if (OFFSET_FINDPLAYERPED == 0) return nullptr;
    InitHooks();
    return nullptr;
}

__attribute__((constructor))
static void samp_ios_constructor() {
    init_log();
    fprintf(stderr, "[SAMP] libSAMP.dylib carregada\n");
    pthread_t thread;
    pthread_create(&thread, nullptr, samp_init_thread, nullptr);
    pthread_detach(thread);
}

__attribute__((destructor))
static void samp_ios_destructor() {
    if (g_logFile) { fclose(g_logFile); g_logFile = nullptr; }
}
#endif
