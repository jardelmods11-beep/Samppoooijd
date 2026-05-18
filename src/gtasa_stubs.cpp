// gtasa_stubs.cpp — funções do GTA SA via offsets reais ARM64
#ifdef IOS_PORT
#include <cstdlib>
#include "ios_base.h"
#include "../sa_include/GTASA.h"

// Offsets definidos aqui diretamente para não depender do ios_offsets.h gerado
#define _OFF_FINDPLAYERPED       0x004A5DE0UL
#define _OFF_ASCIITOGXTCHAR      0x00441430UL
#define _OFF_CSTREAMING_INIT     0x002EB31CUL
#define _OFF_CSTREAMING_LOADALL  0x002EB31CUL
#define _OFF_CCLOTHES_REBUILD    0x002DED20UL
#define _OFF_CPED_SETPEDSTATE    0x003DB7D8UL
#define _OFF_CPED_SETMOVESTATE   0x003DB864UL
#define _OFF_CWORLD_ADD          0x003921D8UL
#define _OFF_CWORLD_REMOVE       0x00392504UL

CPlayerPed* FindPlayerPed(int id) {
    return CALL(CPlayerPed*(*)(int), _OFF_FINDPLAYERPED)(id);
}

void AsciiToGxtChar(const char* src, unsigned short* dst) {
    CALL(void(*)(const char*, unsigned short*), _OFF_ASCIITOGXTCHAR)(src, dst);
}

void CPlayerPed::SetupPlayerPed(int idx) {
    CALL(void(*)(int), _OFF_FINDPLAYERPED)(idx);
}

int CStreaming::RequestSpecialModel(int model, const char* name, int flags) {
    return CALL(int(*)(int, const char*, int), _OFF_CSTREAMING_INIT)(model, name, flags);
}
int CStreaming::LoadAllRequestedModels(bool b) {
    return CALL(int(*)(bool), _OFF_CSTREAMING_LOADALL)(b);
}

void CClothes::RebuildPlayer(CPlayerPed* ped, bool b) {
    CALL(void(*)(CPlayerPed*, bool), _OFF_CCLOTHES_REBUILD)(ped, b);
}

void* CPed::operator new(size_t size) { return ::malloc(size); }

void CPed::SetPedState(ePedState state) {
    CALL(void(*)(CPed*, ePedState), _OFF_CPED_SETPEDSTATE)(this, state);
}
void CPed::SetMoveState(eMoveState state) {
    CALL(void(*)(CPed*, eMoveState), _OFF_CPED_SETMOVESTATE)(this, state);
}

int CWorld::PlayerInFocus = 0;
void CWorld::Add(CEntity* entity) {
    CALL(void(*)(CEntity*), _OFF_CWORLD_ADD)(entity);
}
void CWorld::Remove(CEntity* entity) {
    CALL(void(*)(CEntity*), _OFF_CWORLD_REMOVE)(entity);
}

extern "C" void __clear_cache(void*, void*) {}

#endif
