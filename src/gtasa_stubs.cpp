// gtasa_stubs.cpp
// Implementa as funções do GTA SA redirecionando para offsets reais.

#ifdef IOS_PORT
#include "ios_base.h"
#include "../sa_include/GTASA.h"

#define OFF_FINDPLAYERPED                     0x004A5DE0UL
#define OFF_CPLAYERPED_SETUPPLAYERPED         0x004A5DE0UL
#define OFF_CSTREAMING_REQUESTSPECIALMODEL    0x002EB31CUL
#define OFF_CSTREAMING_LOADALLREQUESTEDMODELS 0x002EB31CUL
#define OFF_CCLOTHES_REBUILDPLAYER            0x002DED20UL
#define OFF_CPED_CTOR                         0x002DED20UL
#define OFF_CPED_SETPEDSTATE                  0x002A67D4UL
#define OFF_CPED_SETMOVESTATE                 0x002A67D4UL
#define OFF_CWORLD_ADD                        0x002F7894UL
#define OFF_CWORLD_REMOVE                     0x002F7894UL

// FindPlayerPed
CPlayerPed* FindPlayerPed(int id) {
    return CALL(CPlayerPed*(*)(int), OFF_FINDPLAYERPED)(id);
}

// CPlayerPed
void CPlayerPed::SetupPlayerPed(int idx) {
    CALL(void(*)(int), OFF_CPLAYERPED_SETUPPLAYERPED)(idx);
}

// CStreaming — retorna int conforme declaração
int CStreaming::RequestSpecialModel(int model, const char* name, int flags) {
    return CALL(int(*)(int, const char*, int), OFF_CSTREAMING_REQUESTSPECIALMODEL)(model, name, flags);
}

int CStreaming::LoadAllRequestedModels(bool b) {
    return CALL(int(*)(bool), OFF_CSTREAMING_LOADALLREQUESTEDMODELS)(b);
}

// CClothes
void CClothes::RebuildPlayer(CPlayerPed* ped, bool b) {
    CALL(void(*)(CPlayerPed*, bool), OFF_CCLOTHES_REBUILDPLAYER)(ped, b);
}

// CPed
void* CPed::operator new(size_t size) {
    return ::operator new(size);
}

CPed::CPed(unsigned int modelIndex) {
    CALL(void(*)(CPed*, unsigned int), OFF_CPED_CTOR)(this, modelIndex);
}

void CPed::SetPedState(ePedState state) {
    CALL(void(*)(CPed*, ePedState), OFF_CPED_SETPEDSTATE)(this, state);
}

void CPed::SetMoveState(eMoveState state) {
    CALL(void(*)(CPed*, eMoveState), OFF_CPED_SETMOVESTATE)(this, state);
}

// CWorld
int CWorld::PlayerInFocus = 0;

void CWorld::Add(CEntity* entity) {
    CALL(void(*)(CEntity*), OFF_CWORLD_ADD)(entity);
}

void CWorld::Remove(CEntity* entity) {
    CALL(void(*)(CEntity*), OFF_CWORLD_REMOVE)(entity);
}

// __clear_cache — builtin ARM64, stub para o linker
extern "C" void __clear_cache(void*, void*) {}

#endif // IOS_PORT
