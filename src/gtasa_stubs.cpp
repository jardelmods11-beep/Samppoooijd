// gtasa_stubs.cpp — implementa funções do GTA SA via offsets reais
#ifdef IOS_PORT
#include <cstdlib>
#include "ios_base.h"
#include "ios_offsets.h"
#include "../sa_include/GTASA.h"

// ── FindPlayerPed ─────────────────────────────────────────────────────────────
CPlayerPed* FindPlayerPed(int id) {
    return CALL(CPlayerPed*(*)(int), OFFSET_FINDPLAYERPED)(id);
}

// ── AsciiToGxtChar ────────────────────────────────────────────────────────────
void AsciiToGxtChar(const char* src, unsigned short* dst) {
    CALL(void(*)(const char*, unsigned short*), OFFSET_ASCIITOGXTCHAR)(src, dst);
}

// ── CPlayerPed::SetupPlayerPed ────────────────────────────────────────────────
void CPlayerPed::SetupPlayerPed(int idx) {
    CALL(void(*)(int), OFFSET_FINDPLAYERPED)(idx); // placeholder
}

// ── CStreaming ────────────────────────────────────────────────────────────────
int CStreaming::RequestSpecialModel(int model, const char* name, int flags) {
    return CALL(int(*)(int, const char*, int), OFFSET_CSTREAMING_INIT)(model, name, flags);
}
int CStreaming::LoadAllRequestedModels(bool b) {
    return CALL(int(*)(bool), OFFSET_CSTREAMING_LOADALLREQUESTEDMODELS)(b);
}

// ── CClothes ─────────────────────────────────────────────────────────────────
void CClothes::RebuildPlayer(CPlayerPed* ped, bool b) {
    CALL(void(*)(CPlayerPed*, bool), OFFSET_CCLOTHES_REBUILDPLAYER)(ped, b);
}

// ── CPed ─────────────────────────────────────────────────────────────────────
void* CPed::operator new(size_t size) {
    return ::malloc(size);
}

void CPed::SetPedState(ePedState state) {
    CALL(void(*)(CPed*, ePedState), OFFSET_CPED_SETPEDSTATE)(this, state);
}
void CPed::SetMoveState(eMoveState state) {
    CALL(void(*)(CPed*, eMoveState), OFFSET_CPED_SETMOVESTATE)(this, state);
}

// ── CWorld ────────────────────────────────────────────────────────────────────
int CWorld::PlayerInFocus = 0;
void CWorld::Add(CEntity* entity) {
    CALL(void(*)(CEntity*), OFFSET_CWORLD_ADD)(entity);
}
void CWorld::Remove(CEntity* entity) {
    CALL(void(*)(CEntity*), OFFSET_CWORLD_REMOVE)(entity);
}

// ── __clear_cache ─────────────────────────────────────────────────────────────
extern "C" void __clear_cache(void*, void*) {}

#endif // IOS_PORT
