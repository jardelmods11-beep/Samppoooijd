#pragma once
// ios_offsets.h
// GERADO AUTOMATICAMENTE por scripts/extract_offsets.py
// Adapte os valores de offset para a versão do GTA SA iOS que você está usando.
// Os offsets abaixo são PLACEHOLDERS — substitua pelos valores reais extraídos do binário.

#include <cstdint>
#include <dlfcn.h>

// Retorna o endereço base da imagem do GTA SA no processo
static inline uintptr_t GetGTASABase()
{
    static uintptr_t base = 0;
    if (base == 0) {
        Dl_info info;
        // Tenta localizar pelo símbolo público mais comum do GTA SA
        void* sym = dlsym(RTLD_DEFAULT, "JNI_OnLoad");
        if (!sym) sym = dlsym(RTLD_DEFAULT, "_ZN9CRunningScript7ProcessEv");
        if (sym && dladdr(sym, &info)) {
            base = (uintptr_t)info.dli_fbase;
        }
    }
    return base;
}

// Helper: calcula endereço real a partir do offset
#define GTASA_ADDR(offset)  (GetGTASABase() + (uintptr_t)(offset))

// ============================================================
//  OFFSETS — substitua pelos valores corretos do seu binário
//  Use scripts/extract_offsets.py para gerar automaticamente.
// ============================================================

// CRunningScript::Process
#define OFFSET_CRUNNINGSCRIPT_PROCESS   0x00000000UL

// CPopCycle::Update
#define OFFSET_CPOPCYCLE_UPDATE         0x00000000UL

// Loading screen render
#define OFFSET_LOADINGSCREEN_FUNC       0x00000000UL

// Render2dStuffAfterFade
#define OFFSET_RENDER2DSTUFFAFTERFADE   0x00000000UL

// FindPlayerPed (usado em hooks.cpp via GTASA_FUNC)
#define OFFSET_FINDPLAYERPED            0x00000000UL
#define FINDPLAYERPED                   OFFSET_FINDPLAYERPED

// CHud::DrawAfterFade
#define CHUD_DRAWAFTERFADE              0x00000000UL

// CMessages::Display
#define CMESSAGES_DISPLAY               0x00000000UL

// CCredits::Render
#define CCREDITS_RENDER                 0x00000000UL

// CLoadingScreen render func
#define LOADINGSCREEN_FUNC              OFFSET_LOADINGSCREEN_FUNC
