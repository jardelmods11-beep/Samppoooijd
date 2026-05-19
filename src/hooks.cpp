#ifdef IOS_PORT

#include "ios_base.h"
#include "CNetGame.h"
#include "utils.h"

#ifdef __APPLE__
#include <sys/mman.h>
#include <unistd.h>
#endif

#define OFF_CHUD_DRAWAFTERFADE      0x002A67D4UL
#define OFF_RENDER2DSTUFFAFTERFADE  0x002A9294UL
#define OFF_CCREDITS_RENDER         0x002A92A4UL
#define OFF_CMESSAGES_DISPLAY       0x002428E4UL
#define OFF_LOADINGSCREEN_FUNC      0x0024017CUL
#define OFF_CRUNNINGSCRIPT_PROCESS  0x001CFF1CUL
#define OFF_CPOPCYCLE_UPDATE        0x002F7894UL

static void hook_jmp(uintptr_t offset, void* dest) {
#ifdef __APPLE__
    uintptr_t from = GetBase() + offset;
    if (!from || !dest) return;
    int64_t diff = ((int64_t)(uintptr_t)dest - (int64_t)from) / 4;
    // Verifica se está dentro do range do B instruction (±128MB)
    if (diff < -(1<<25) || diff > ((1<<25)-1)) {
        samp_log("[SAMP] AVISO: hook fora do range: 0x%lx -> %p diff=%lld",
                 (unsigned long)from, dest, (long long)diff);
        return;
    }
    uint32_t instr = 0x14000000 | ((uint32_t)(diff) & 0x3FFFFFF);
    uintptr_t page = from & ~(uintptr_t)(getpagesize()-1);
    if (mprotect((void*)page, getpagesize()*2, PROT_READ|PROT_WRITE|PROT_EXEC) != 0) {
        samp_log("[SAMP] AVISO: mprotect falhou para 0x%lx", (unsigned long)from);
        return;
    }
    memcpy((void*)from, &instr, 4);
    __builtin___clear_cache((char*)from, (char*)(from+4));
    samp_log("[SAMP] Hook OK: 0x%lx -> %p", (unsigned long)from, dest);
#endif
}

int CRunningScript__Process_hook(void*) {
    static bool done = false;
    if (!done) {
        done = true;
        samp_log("[SAMP] CRunningScript::Process hookado!");
        CNetGame* g = CNetGame::Instance();
        if (g && g->getPlayerPool()) {
            CPlayerPed* ped = g->getPlayerPool()->GetLocalPlayer()->CreateGTAPlayer();
            if (ped) ped->Teleport(CVector(0.0f, 0.0f, 3.0f), 0);
            g->DbgConnect();
        }
    }
    if (CNetGame::Instance()) CNetGame::Instance()->Process();
    return 0;
}

int CPopCycle__Update_hook() { return 0; }

void LoadingScreen_hook(char const* a, char const* b, char const* c) {
    samp_log("[SAMP] LoadingScreen: %s", a ? a : "null");
    CALL(void(*)(), OFF_LOADINGSCREEN_FUNC)();
}

void Render2dStuffAfterFade_hook() {
    CALL(void(*)(),    OFF_CHUD_DRAWAFTERFADE)();
    CALL(void(*)(int), OFF_CMESSAGES_DISPLAY)(0);
    CALL(void(*)(),    OFF_CCREDITS_RENDER)();
}

void InitHooks() {
    samp_log("[SAMP] Iniciando hooks...");
    samp_log("[SAMP] Base = 0x%lx", (unsigned long)GetBase());

    hook_jmp(OFF_CRUNNINGSCRIPT_PROCESS, (void*)CRunningScript__Process_hook);
    hook_jmp(OFF_CPOPCYCLE_UPDATE,       (void*)CPopCycle__Update_hook);
    hook_jmp(OFF_LOADINGSCREEN_FUNC,     (void*)LoadingScreen_hook);
    hook_jmp(OFF_RENDER2DSTUFFAFTERFADE, (void*)Render2dStuffAfterFade_hook);

    samp_log("[SAMP] Todos os hooks aplicados!");
}

#endif // IOS_PORT
