#pragma once
#include "ios_base.h"

// ── Offsets reais do gta3sa ARM64 (14MB, fat binary ARMv7+ARM64) ─────────────
// Encontrados via ADRP+ADD scan, ADR scan, pattern analysis

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
#define OFFSET_ASCIITOGXTCHAR                    0x00441430UL
#define OFFSET_CWORLD_ADD                        0x003921D8UL
#define OFFSET_CWORLD_REMOVE                     0x00392504UL
#define OFFSET_CPED_SETPEDSTATE                  0x003DB7D8UL
#define OFFSET_CPED_SETMOVESTATE                 0x003DB864UL
