#pragma once
#include <cstdint>
#include <cstring>

#ifdef __APPLE__
#include <mach-o/dyld.h>
static inline uintptr_t GetGTASABase() {
    for (uint32_t i = 0; i < _dyld_image_count(); i++) {
        const char* n = _dyld_get_image_name(i);
        if (n && (strstr(n,"GTASA")||strstr(n,"gta3sa")||strstr(n,"gta_sa")))
            return (uintptr_t)_dyld_get_image_header(i);
    }
    return (uintptr_t)_dyld_get_image_header(0);
}
#else
static inline uintptr_t GetGTASABase() { return 0; }
#endif

// Calcula endereço real a partir do offset
#define GTASA_ADDR(offset) (GetGTASABase() + (uintptr_t)(offset))

// Chama função pelo endereço absoluto
// Uso: GTASA_FUNC(void(*)(), 0x002A67D4UL)()
// ou:  GTASA_FUNC(void(*)(), OFFSET_CHUD_DRAWAFTERFADE)()
#define GTASA_FUNC(type, offset) (reinterpret_cast<type>(GTASA_ADDR(offset)))

// ── OFFSETS REAIS (gta3sa ARM64) ─────────────────────────────────────────────
#define OFFSET_FINDPLAYERPED                     0x004A5DE0UL
#define OFFSET_CFONT_INITIALISE                  0x0037C114UL
#define OFFSET_CFONT_PRINTSTRING                 0x0037EBC4UL
#define OFFSET_CHUD_DRAWAFTERFADE                0x002A67D4UL
#define OFFSET_RENDER2DSTUFFAFTERFADE            0x002A9294UL
#define OFFSET_CCREDITS_RENDER                   0x002A92A4UL
#define OFFSET_CMESSAGES_DISPLAY                 0x002428E4UL
#define OFFSET_LOADINGSCREEN_FUNC                0x0024017CUL
#define OFFSET_CSTREAMING_INIT                   0x002EB31CUL
#define OFFSET_CSTREAMING_LOADALLREQUESTEDMODELS 0x002EB31CUL
#define OFFSET_CSTREAMING_SETUP                  0x002F9880UL
#define OFFSET_CDSTREAM_INIT                     0x00160798UL
#define OFFSET_CRUNNINGSCRIPT_PROCESS            0x001CFF1CUL
#define OFFSET_CPOPCYCLE_UPDATE                  0x002F7894UL
#define OFFSET_PEDEVENT_INIT                     0x0035BBE0UL
#define OFFSET_CCLOTHES_REBUILDPLAYER            0x002DED20UL
#define OFFSET_SAVELOAD                          0x002D4B84UL
#define OFFSET_TOUCHINPUT                        0x0014505CUL
#define OFFSET_GGAMESTATE_FUNC                   0x004B7E70UL
