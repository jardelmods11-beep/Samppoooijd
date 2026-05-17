#include "CNetGame.h"
#include "utils.h"
#include "ios_hooks.h"
#include "ios_offsets.h"
#include "GTASA.h"

static CPlayerPed* FindPlayerPed_ios(int idx) {
    return GTASA_FUNC(CPlayerPed*(*)(int), FINDPLAYERPED)(idx);
}

int CRunningScript__Process_hook(void* p) {
    static bool done = false;
    if (!done) {
        done = true;
        CNetGame* g = CNetGame::Instance();
        CPlayerPed* ped = g->getPlayerPool()->GetLocalPlayer()->CreateGTAPlayer();
        if (ped) ped->Teleport(CVector(0.0f, 0.0f, 3.0f), 0);
        CNetGame::Instance()->DbgConnect();
    }
    CNetGame::Instance()->Process();
    return 0;
}

int CPopCycle__Update_hook() { return 0; }

void LoadingScreen_hook(char const* a1, char const* a2, char const* a3) {
    samp_log("LoadingScreen_hook(%s, %s, %s)", a1, a2, a3);
    GTASA_FUNC(void(*)(), LOADINGSCREEN_FUNC)();
}

void RenderSAMP() {
    CPlayerPed* ped = FindPlayerPed_ios(-1);
    if (!ped) return;
}

void Render2dStuffAfterFade_hook() {
    GTASA_FUNC(void(*)(), CHUD_DRAWAFTERFADE)();
    RenderSAMP();
    GTASA_FUNC(void(*)(int), CMESSAGES_DISPLAY)(0);
    GTASA_FUNC(void(*)(), CCREDITS_RENDER)();
}

void InitHooks() {
    samp_log("[SAMP] Base: 0x%lx", GetGTASABase());
    HOOK_JMP(OFFSET_CRUNNINGSCRIPT_PROCESS, CRunningScript__Process_hook);
    HOOK_JMP(OFFSET_CPOPCYCLE_UPDATE,        CPopCycle__Update_hook);
    HOOK_JMP(OFFSET_LOADINGSCREEN_FUNC,      LoadingScreen_hook);
    HOOK_JMP(OFFSET_RENDER2DSTUFFAFTERFADE,  Render2dStuffAfterFade_hook);
    samp_log("[SAMP] Hooks aplicados!");
}
