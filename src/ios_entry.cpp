#ifdef IOS_PORT
#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <pthread.h>
#include <unistd.h>

#include "hooks.h"
#include "ios_base.h"
#include "ios_offsets.h"

static FILE* g_logFile = nullptr;

void samp_log(const char* fmt, ...) {
    va_list a;
    va_start(a, fmt);
    vfprintf(stderr, fmt, a);
    fprintf(stderr, "\n");
    if (g_logFile) {
        vfprintf(g_logFile, fmt, a);
        fprintf(g_logFile, "\n");
        fflush(g_logFile);
    }
    va_end(a);
}

static void init_log() {
    const char* home = getenv("HOME");
    char path[512];
    if (home) {
        snprintf(path, sizeof(path), "%s/Documents/samp.log", home);
        g_logFile = fopen(path, "w");
    }
    if (!g_logFile) {
        g_logFile = fopen("/tmp/samp.log", "w");
    }
}

static void* samp_net_thread(void*) {
    samp_log("[SAMP] Thread de rede iniciada");
    // Aqui vai a lógica de rede do SAMP no futuro
    return nullptr;
}

__attribute__((constructor))
static void samp_ios_constructor() {
    // Abre o log primeiro
    init_log();
    samp_log("[SAMP] ============================================");
    samp_log("[SAMP] libSAMP.dylib carregada!");
    samp_log("[SAMP] Compilado: %s %s", __DATE__, __TIME__);
    samp_log("[SAMP] ============================================");

    // Aplica hooks IMEDIATAMENTE — antes do iOS ativar W^X
    samp_log("[SAMP] Base: 0x%lx", (unsigned long)GetBase());
    samp_log("[SAMP] Aplicando hooks agora (sem delay)...");
    InitHooks();

    // Inicia thread de rede separada
    pthread_t thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&thread, &attr, samp_net_thread, nullptr);
    pthread_attr_destroy(&attr);
}

__attribute__((destructor))
static void samp_ios_destructor() {
    samp_log("[SAMP] libSAMP.dylib descarregada");
    if (g_logFile) {
        fclose(g_logFile);
        g_logFile = nullptr;
    }
}
#endif
