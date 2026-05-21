#ifdef IOS_PORT

#include "ios_base.h"
#include "CNetGame.h"
#include "utils.h"

#ifdef __APPLE__
#include <mach/mach.h>
#include <mach/vm_map.h>
#include <sys/mman.h>
#include <unistd.h>
#include <libkern/OSCacheControl.h>
#endif

#define OFF_CHUD_DRAWAFTERFADE      0x002A67D4UL
#define OFF_RENDER2DSTUFFAFTERFADE  0x002A9294UL
#define OFF_CCREDITS_RENDER         0x002A92A4UL
#define OFF_CMESSAGES_DISPLAY       0x002428E4UL
#define OFF_LOADINGSCREEN_FUNC      0x0024017CUL
#define OFF_CRUNNINGSCRIPT_PROCESS  0x001CFF1CUL
#define OFF_CPOPCYCLE_UPDATE        0x002F7894UL

static bool write_memory(uintptr_t addr, void* data, size_t size) {
#ifdef __APPLE__
    // Método 1: vm_protect via Mach (mais permissivo que mprotect no iOS)
    mach_port_t task = mach_task_self();
    vm_address_t page = addr & ~(vm_address_t)(getpagesize() - 1);
    vm_size_t page_size = getpagesize() * 2;

    kern_return_t kr = vm_protect(task, page, page_size, false,
                                  VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE);
    if (kr != KERN_SUCCESS) {
        samp_log("[SAMP] vm_protect falhou (kr=%d) para 0x%lx — tentando vm_write...", kr, addr);
        // Método 2: vm_write — escreve diretamente na memória do processo
        kr = vm_write(task, addr, (vm_offset_t)data, size);
        if (kr != KERN_SUCCESS) {
            samp_log("[SAMP] vm_write também falhou (kr=%d) para 0x%lx", kr, addr);
            return false;
        }
        sys_icache_invalidate((void*)addr, size);
        return true;
    }
    memcpy((void*)addr, data, size);
    sys_icache_invalidate((void*)addr, size);
    // Restaura proteção
    vm_protect(task, page, page_size, false, VM_PROT_READ | VM_PROT_EXECUTE);
    return true;
#else
    return false;
#endif
}

static void hook_jmp(uintptr_t offset, void* dest) {
    uintptr_t from = GetBase() + offset;
    if (!from || !dest) return;

    int64_t diff = ((int64_t)(uintptr_t)dest - (int64_t)from) / 4;
    if (diff < -(1<<25) || diff > ((1<<25)-1)) {
        samp_log("[SAMP] Hook fora do range B: 0x%lx diff=%lld — usando trampolim", from, (long long)diff);
        // Trampolim ARM64: LDR X16, #8 / BR X16 / .quad addr
        uint8_t tramp[16];
        uint32_t ldr = 0x58000050; // LDR X16, #8
        uint32_t br  = 0xD61F0200; // BR X16
        uint64_t addr = (uint64_t)(uintptr_t)dest;
        memcpy(tramp,    &ldr,  4);
        memcpy(tramp+4,  &br,   4);
        memcpy(tramp+8,  &addr, 8);
        if (write_memory(from, tramp, 16)) {
            samp_log("[SAMP] Hook trampolim OK: 0x%lx -> %p", from, dest);
        }
        return;
    }

    uint32_t instr = 0x14000000 | ((uint32_t)(diff) & 0x3FFFFFF);
    if (write_memory(from, &instr, 4)) {
        samp_log("[SAMP] Hook B OK: 0x%lx -> %p", from, dest);
    }
}

// ── Hooks ─────────────────────────────────────────────────────────────────────
int CRunningScript__Process_hook(void*) {
    static bool done = false;
    if (!done) {
        done = true;
        samp_log("[SAMP] CRunningScript::Process — primeiro tick!");
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
    samp_log("[SAMP] Aplicando hooks (vm_protect/vm_write)...");
    samp_log("[SAMP] Base = 0x%lx", (unsigned long)GetBase());

    hook_jmp(OFF_CRUNNINGSCRIPT_PROCESS, (void*)CRunningScript__Process_hook);
    hook_jmp(OFF_CPOPCYCLE_UPDATE,       (void*)CPopCycle__Update_hook);
    hook_jmp(OFF_LOADINGSCREEN_FUNC,     (void*)LoadingScreen_hook);
    hook_jmp(OFF_RENDER2DSTUFFAFTERFADE, (void*)Render2dStuffAfterFade_hook);

    samp_log("[SAMP] Hooks finalizados!");
}

#endif // IOS_PORT
