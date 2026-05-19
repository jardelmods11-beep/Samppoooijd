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
    // Tenta vários caminhos para garantir que o log seja criado
    const char* paths[] = {
        "/var/mobile/Documents/samp.log",
        "/tmp/samp.log",
        nullptr
    };
    const char* home = getenv("HOME");
    char home_path[512];
    if (home) {
        snprintf(home_path, sizeof(home_path), "%s/Documents/samp.log", home);
        paths[0] = home_path;
    }
    for (int i = 0; paths[i]; i++) {
        g_logFile = fopen(paths[i], "w");
        if (g_logFile) {
            samp_log("[SAMP] Log aberto: %s", paths[i]);
            break;
        }
    }
}

static void* samp_init_thread(void*) {
    samp_log("[SAMP] Thread iniciada, aguardando 5s...");

    // Aguarda mais tempo para o jogo carregar completamente
    sleep(5);

    samp_log("[SAMP] Verificando base...");
    uintptr_t base = GetBase();
    samp_log("[SAMP] Base: 0x%lx", base);

    if (base == 0) {
        samp_log("[SAMP] ERRO: base = 0, abortando");
        return nullptr;
    }

    // Verifica se os offsets críticos estão definidos
    samp_log("[SAMP] OFFSET_CRUNNINGSCRIPT_PROCESS = 0x%lx", (unsigned long)OFFSET_CRUNNINGSCRIPT_PROCESS);
    samp_log("[SAMP] OFFSET_CHUD_DRAWAFTERFADE = 0x%lx", (unsigned long)OFFSET_CHUD_DRAWAFTERFADE);

    if (OFFSET_CRUNNINGSCRIPT_PROCESS == 0) {
        samp_log("[SAMP] ERRO: offsets = 0, abortando");
        return nullptr;
    }

    samp_log("[SAMP] Aplicando hooks...");
    InitHooks();
    samp_log("[SAMP] Inicializado com sucesso!");
    return nullptr;
}

__attribute__((constructor))
static void samp_ios_constructor() {
    // Abre o log PRIMEIRO antes de qualquer outra coisa
    init_log();
    samp_log("[SAMP] ============================================");
    samp_log("[SAMP] libSAMP.dylib carregada!");
    samp_log("[SAMP] Compilado: %s %s", __DATE__, __TIME__);
    samp_log("[SAMP] ============================================");

    pthread_t thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&thread, &attr, samp_init_thread, nullptr);
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
#endif // IOS_PORT
