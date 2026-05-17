#pragma once
#include "ios_base.h"

#ifdef __APPLE__
#include <sys/mman.h>
#include <unistd.h>
static inline bool _hook_write_branch(uintptr_t target, uintptr_t dest) {
    if (!target) return false;
    int64_t diff = (int64_t)(dest - target) / 4;
    if (diff < -(1<<25) || diff > ((1<<25)-1)) return false;
    uint32_t instr = 0x14000000 | ((uint32_t)diff & 0x3FFFFFF);
    uintptr_t page = target & ~(uintptr_t)(getpagesize()-1);
    mprotect((void*)page, getpagesize()*2, PROT_READ|PROT_WRITE|PROT_EXEC);
    memcpy((void*)target, &instr, 4);
    __builtin___clear_cache((char*)target, (char*)(target+4));
    return true;
}
#else
static inline bool _hook_write_branch(uintptr_t, uintptr_t) { return false; }
#endif

#define HOOK_JMP(offset, fn) _hook_write_branch(GetBase()+(uintptr_t)(offset),(uintptr_t)(fn))
#define HOOK_CALL(offset, fn) _hook_write_branch(GetBase()+(uintptr_t)(offset),(uintptr_t)(fn))
