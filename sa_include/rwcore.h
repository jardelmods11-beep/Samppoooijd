#pragma once
#include <cstddef>
struct RwV3d { float x, y, z; };
struct RwMatrix {
    RwV3d right; unsigned int flags;
    RwV3d up;    unsigned int pad1;
    RwV3d at;    unsigned int pad2;
    RwV3d pos;   unsigned int pad3;
};
struct RwFrame { RwMatrix modelling; RwMatrix ltm; };
struct RwObject { unsigned char type, subType, flags, privateFlags; void* parent; };
typedef int RwInt32;
typedef unsigned int RwUInt32;
typedef float RwReal;
typedef bool RwBool;
