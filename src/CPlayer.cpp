#include "CPlayer.h"
#include <string.h>
#include <cstdlib>
#include "utils.h"
#include "ios_base.h"

// Offset do construtor real CPed::CPed(unsigned int) no gta3sa
#define OFF_CPED_NEW_CTOR 0x002DED20UL

// Cria um CPed chamando o construtor real do GTA SA
static CPed* CreateCPed(unsigned int modelIndex) {
    // Aloca memória do tamanho de CPed
    void* mem = ::malloc(2048); // CPed é grande, 2KB é seguro
    if (!mem) return nullptr;
    // Chama o construtor real do GTA SA via offset (inicializa vtable etc)
    typedef CPed* (*CPedCtor_t)(void*, unsigned int);
    return CALL(CPedCtor_t, OFF_CPED_NEW_CTOR)(mem, modelIndex);
}

CPlayer::CPlayer()
{
    CStreaming::RequestSpecialModel(0, "player", 26);
    CStreaming::LoadAllRequestedModels(true);

    samp_log("Creating CPed ...");
    m_pPed = CreateCPed(2);
    samp_log("CPed created -> 0x%08X", m_pPed);

    if (m_pPed) {
        m_pPed->SetModelIndex(0);
        m_pPed->SetPedState(PED_IDLE);
        m_pPed->SetMoveState(PEDMOVE_STILL);
        CWorld::Add(m_pPed);
        m_pPed->Teleport(CVector(1.0f, 0.0f, 3.0f), 0);
    }
}

CPlayer::~CPlayer()
{
    if (m_pPed) {
        CWorld::Remove(m_pPed);
        ::free(m_pPed);
        m_pPed = nullptr;
    }
}

void CPlayer::setNickName(char* a_pszNickName)
{
    unsigned int l_uiNickNameLen;
    if (m_pszNickName) delete [] m_pszNickName;
    l_uiNickNameLen = strlen(a_pszNickName);
    m_pszNickName = new char[l_uiNickNameLen + 1];
    strcpy(m_pszNickName, a_pszNickName);
}

void CPlayer::setNPC(bool a_bSet) { m_bIsNPC = a_bSet; }

void CPlayer::ProcessPlayerSync(Packet* a_pPacket)
{
    RakNet::BitStream l_BitStream(a_pPacket->data, a_pPacket->length, false);
    l_BitStream.IgnoreBits(8);
    l_BitStream.IgnoreBits(sizeof(_PlayerID) * 8);
    samp_log("Proccess Sync...");
    if (!m_pPed || !m_pPed->m_pMatrix) return;
    CWorld::Remove(m_pPed);
    if (l_BitStream.ReadBit()) { uint16_t k; l_BitStream.Read(k); }
    if (l_BitStream.ReadBit()) { uint16_t k; l_BitStream.Read(k); }
    uint16_t keysOnfoot; l_BitStream.Read(keysOnfoot);
    float px, py, pz; l_BitStream.Read(px); l_BitStream.Read(py); l_BitStream.Read(pz);
    m_pPed->m_pMatrix->m_RwMatrix.pos.x = px;
    m_pPed->m_pMatrix->m_RwMatrix.pos.y = py;
    m_pPed->m_pMatrix->m_RwMatrix.pos.z = pz;
    float rx,ry,rz,ux,uy,uz,ax,ay,az;
    l_BitStream.ReadOrthMatrix(rx,ry,rz,ux,uy,uz,ax,ay,az);
    m_pPed->m_pMatrix->m_RwMatrix.right.x = rx;
    m_pPed->m_pMatrix->m_RwMatrix.right.y = ry;
    m_pPed->m_pMatrix->m_RwMatrix.right.z = rz;
    m_pPed->m_pMatrix->m_RwMatrix.up.x = ux;
    m_pPed->m_pMatrix->m_RwMatrix.up.y = uy;
    m_pPed->m_pMatrix->m_RwMatrix.up.z = uz;
    m_pPed->m_pMatrix->m_RwMatrix.at.x = ax;
    m_pPed->m_pMatrix->m_RwMatrix.at.y = ay;
    m_pPed->m_pMatrix->m_RwMatrix.at.z = az;
    uint8_t compressedHealthAndArmour; l_BitStream.Read(compressedHealthAndArmour);
    uint8_t weapon; l_BitStream.Read(weapon);
    uint8_t specialAction; l_BitStream.Read(specialAction);
    float vx,vy,vz; l_BitStream.ReadVector(vx,vy,vz);
    if (l_BitStream.ReadBit()) { _VehicleID sid; float sx,sy,sz; l_BitStream.Read(sid); l_BitStream.Read(sx); l_BitStream.Read(sy); l_BitStream.Read(sz); }
    if (l_BitStream.ReadBit()) { uint32_t ai; l_BitStream.Read(ai); }
    CWorld::Add(m_pPed);
}

void CPlayer::StreamIn(RPCParameters* rpcParams)
{
    int i;
    RakNet::BitStream l_BitStream(rpcParams->input, rpcParams->numberOfBitsOfData / 8 + 1, false);
    l_BitStream.IgnoreBits(sizeof(_PlayerID) * 8);
    l_BitStream.Read(m_iTeamID); l_BitStream.Read(m_iSkinID);
    float px,py,pz; l_BitStream.Read(px); l_BitStream.Read(py); l_BitStream.Read(pz);
    l_BitStream.Read(m_fFacingAngle);
    l_BitStream.Read(m_uiNickColor);
    l_BitStream.Read(m_iFightingStyle);
    for (i = 0; i < MAX_SKILL_LEVEL; i++) l_BitStream.Read(m_iSkillLevel[i]);
    m_bIsStreamed = true;
    WorldAdd();
}

void CPlayer::StreamOut() { m_bIsStreamed = false; WorldRemove(); }
void CPlayer::WorldAdd() {}
void CPlayer::WorldRemove() {}
